﻿#include "StorageApp_Hdr.h"

XHTHREAD CALLBACK XEngine_Webdav_HTTPThread(XPVOID lParam)
{
	int nThreadPos = *(int*)lParam;
	nThreadPos++;

	while (bIsRun)
	{
		//等待指定线程事件触发
		if (HttpProtocol_Server_EventWaitEx(xhWebdavHttp, nThreadPos))
		{
			int nListCount = 0;
			XENGINE_MANAGEPOOL_TASKEVENT** ppSt_PKTClient;
			//获取当前队列池中所有触发上传客户端
			HttpProtocol_Server_GetPoolEx(xhWebdavHttp, nThreadPos, &ppSt_PKTClient, &nListCount);
			for (int i = 0; i < nListCount; i++)
			{
				for (int j = 0; j < ppSt_PKTClient[i]->nPktCount; j++)
				{
					int nMsgLen = 0;
					int nHdrCount = 0;
					XCHAR* ptszMsgBuffer = NULL;
					XCHAR** ppszListHdr = NULL;
					RFCCOMPONENTS_HTTP_REQPARAM st_HTTPParam;

					memset(&st_HTTPParam, '\0', sizeof(RFCCOMPONENTS_HTTP_REQPARAM));
					//获得指定上传客户端触发信息
					if (HttpProtocol_Server_GetMemoryEx(xhWebdavHttp, ppSt_PKTClient[i]->tszClientAddr, &ptszMsgBuffer, &nMsgLen, &st_HTTPParam, &ppszListHdr, &nHdrCount))
					{
						XEngine_Task_HttpWebdav(ppSt_PKTClient[i]->tszClientAddr, ptszMsgBuffer, nMsgLen, &st_HTTPParam, ppszListHdr, nHdrCount);
					}
					BaseLib_OperatorMemory_FreeCStyle((XPPMEM)&ptszMsgBuffer);
					BaseLib_OperatorMemory_Free((XPPPMEM)&ppszListHdr, nHdrCount);
				}
			}
			BaseLib_OperatorMemory_Free((XPPPMEM)&ppSt_PKTClient, nListCount);
		}
	}
	return 0;
}

bool XEngine_Task_HttpWebdav(LPCXSTR lpszClientAddr, LPCXSTR lpszMsgBuffer, int nMsgLen, RFCCOMPONENTS_HTTP_REQPARAM* pSt_HTTPParam, XCHAR** pptszListHdr, int nHdrCount)
{
	int nSDLen = 8192;
	int nRVLen = 8192;
	XCHAR tszSDBuffer[8192] = {};
	XCHAR tszRVBuffer[8192] = {};
	RFCCOMPONENTS_HTTP_HDRPARAM st_HDRParam = {};

	LPCXSTR lpszMethodOption = _X("OPTIONS");
	LPCXSTR lpszMethodPropfind = _X("PROPFIND");
	LPCXSTR lpszMethodPropPatch = _X("PROPPATCH");
	LPCXSTR lpszMethodGet = _X("GET");
	LPCXSTR lpszMethodPut = _X("PUT");
	LPCXSTR lpszMethodDel = _X("DELETE");
	LPCXSTR lpszMethodLock = _X("LOCK");
	LPCXSTR lpszMethodUNLock = _X("UNLOCK");

	st_HDRParam.bIsClose = false;
	st_HDRParam.nHttpCode = 200;

	if (0 == _tcsxnicmp(lpszMethodOption, pSt_HTTPParam->tszHttpMethod, _tcsxlen(lpszMethodOption)))
	{
		//用于心跳
		st_HDRParam.bIsClose = true;
		st_HDRParam.nHttpCode = 200;
		LPCXSTR lpszHdrBuffer = _X("Allow: OPTIONS POST GET PUT PROPFIND PROPPATCH DELETE LOCK UNLOCK\r\n");
		HttpProtocol_Server_SendMsgEx(xhWebdavHttp, tszSDBuffer, &nSDLen, &st_HDRParam, NULL, 0, lpszHdrBuffer);
		XEngine_Net_SendMsg(lpszClientAddr, tszSDBuffer, nSDLen, STORAGE_NETTYPE_HTTPWEBDAV);
		XLOG_PRINT(xhLog, XENGINE_HELPCOMPONENTS_XLOG_IN_LOGLEVEL_INFO, _X("WEBDAV客户端:%s,请求OPTIONS心跳方法成功"), lpszClientAddr);
	}
	else if (0 == _tcsxnicmp(lpszMethodPropfind, pSt_HTTPParam->tszHttpMethod, _tcsxlen(lpszMethodPropfind)))
	{
		if (_tcsxlen(pSt_HTTPParam->tszHttpUri) <= 1)
		{
			st_HDRParam.bIsClose = true;
			st_HDRParam.nHttpCode = 403;
			HttpProtocol_Server_SendMsgEx(xhWebdavHttp, tszSDBuffer, &nSDLen, &st_HDRParam);
			XEngine_Net_SendMsg(lpszClientAddr, tszSDBuffer, nSDLen, STORAGE_NETTYPE_HTTPWEBDAV);
			XLOG_PRINT(xhLog, XENGINE_HELPCOMPONENTS_XLOG_IN_LOGLEVEL_ERROR, _X("WEBDAV客户端:%s,处理WEBDAV协议PROPFIND方法失败,地址不正确,URL:%s"), lpszClientAddr, pSt_HTTPParam->tszHttpUri);
			return false;
		}
		XENGINE_STORAGEBUCKET st_StorageBucket = {};
		if (!APIHelp_Distributed_DLStorage(pSt_HTTPParam->tszHttpUri, st_LoadbalanceCfg.st_LoadBalance.pStl_ListBucket, &st_StorageBucket))
		{
			st_HDRParam.bIsClose = true;
			st_HDRParam.nHttpCode = 400;
			HttpProtocol_Server_SendMsgEx(xhWebdavHttp, tszSDBuffer, &nSDLen, &st_HDRParam);
			XEngine_Net_SendMsg(lpszClientAddr, tszSDBuffer, nSDLen, STORAGE_NETTYPE_HTTPWEBDAV);
			XLOG_PRINT(xhLog, XENGINE_HELPCOMPONENTS_XLOG_IN_LOGLEVEL_ERROR, _X("WEBDAV客户端:%s,处理WEBDAV协议PROPFIND方法失败,URL:%s"), lpszClientAddr, pSt_HTTPParam->tszHttpUri);
			return false;
		}
		XCHAR tszFindStr[MAX_PATH] = {};
		//得到是否是文件
		if (APIHelp_Api_UrlStr(st_StorageBucket.tszBuckKey, pSt_HTTPParam->tszHttpUri))
		{
			int nALen = 0;
			_tcsxcpy(tszFindStr, pSt_HTTPParam->tszHttpUri + 1);
			BaseLib_OperatorString_Replace(tszFindStr, &nALen, st_StorageBucket.tszBuckKey, st_StorageBucket.tszFilePath, true);
		}
		else
		{
			//得到深度
			XCHAR tszVluStr[8] = {};
			HttpProtocol_ServerHelp_GetField(&pptszListHdr, nHdrCount, _X("Depth"), tszVluStr);
			if (1 == _ttxoi(tszVluStr))
			{
				_xstprintf(tszFindStr, _X("%s/*"), st_StorageBucket.tszFilePath);
			}
			else
			{
				_tcsxcpy(tszFindStr, st_StorageBucket.tszFilePath);
			}
		}
		//枚举文件
		int nListCount = 0;
		XCHAR** pptszListFile;
		SystemApi_File_EnumFile(tszFindStr, &pptszListFile, &nListCount);
		if (0 == nListCount)
		{
			st_HDRParam.nHttpCode = 404;
			HttpProtocol_Server_SendMsgEx(xhWebdavHttp, tszSDBuffer, &nSDLen, &st_HDRParam);
			XEngine_Net_SendMsg(lpszClientAddr, tszSDBuffer, nSDLen, STORAGE_NETTYPE_HTTPWEBDAV);
			XLOG_PRINT(xhLog, XENGINE_HELPCOMPONENTS_XLOG_IN_LOGLEVEL_ERROR, _X("WEBDAV客户端:%s,处理WEBDAV协议PROPFIND方法失败,文件没有找到,URL:%s"), lpszClientAddr, pSt_HTTPParam->tszHttpUri);
			return false;
		}
		Protocol_StoragePacket_WDPropfind(tszRVBuffer, &nRVLen, &pptszListFile, nListCount, st_StorageBucket.tszFilePath, st_StorageBucket.tszBuckKey);

		st_HDRParam.nHttpCode = 207;
		_tcsxcpy(st_HDRParam.tszMimeType, _X("xml"));
		HttpProtocol_Server_SendMsgEx(xhWebdavHttp, tszSDBuffer, &nSDLen, &st_HDRParam, tszRVBuffer, nRVLen);
		XEngine_Net_SendMsg(lpszClientAddr, tszSDBuffer, nSDLen, STORAGE_NETTYPE_HTTPWEBDAV);
		XLOG_PRINT(xhLog, XENGINE_HELPCOMPONENTS_XLOG_IN_LOGLEVEL_INFO, _X("WEBDAV客户端:%s,处理WEBDAV协议PROPFIND方法成功,文件名称:%s"), lpszClientAddr, tszFindStr);
		return false;
	}
	else if (0 == _tcsxnicmp(lpszMethodGet, pSt_HTTPParam->tszHttpMethod, _tcsxlen(lpszMethodGet)))
	{
		//使用重定向实现下载
		st_HDRParam.bIsClose = true;
		st_HDRParam.nHttpCode = 302;

		XCHAR tszRequestAddr[MAX_PATH] = {};
		XCHAR tszHostStr[128] = {};
		HttpProtocol_ServerHelp_GetField(&pptszListHdr, nHdrCount, _X("Host"), tszHostStr);

		XCHAR tszPortWebdav[64] = {};
		XCHAR tszPortDownload[64] = {};
		_xstprintf(tszPortWebdav, _X("%d"), st_ServiceCfg.nWebdavPort);
		_xstprintf(tszPortDownload, _X("%d"), st_ServiceCfg.nStorageDLPort);
		//转换端口
		int nReplaceLen = 0;
		BaseLib_OperatorString_Replace(tszHostStr, &nReplaceLen, tszPortWebdav, tszPortDownload, true);
		_xstprintf(tszRequestAddr, _X("Location: http://%s%s\r\n"), tszHostStr, pSt_HTTPParam->tszHttpUri);

		HttpProtocol_Server_SendMsgEx(xhWebdavHttp, tszSDBuffer, &nSDLen, &st_HDRParam, NULL, 0, tszRequestAddr);
		XEngine_Net_SendMsg(lpszClientAddr, tszSDBuffer, nSDLen, STORAGE_NETTYPE_HTTPWEBDAV);
		XLOG_PRINT(xhLog, XENGINE_HELPCOMPONENTS_XLOG_IN_LOGLEVEL_INFO, _X("WEBDAV客户端:%s,请求文件下载被重定向到:%s"), lpszClientAddr, tszRequestAddr);
		return true;
	}
	else if (0 == _tcsxnicmp(lpszMethodPut, pSt_HTTPParam->tszHttpMethod, _tcsxlen(lpszMethodPut)))
	{
		//使用重定向实现上传
		st_HDRParam.bIsClose = false;
		st_HDRParam.nHttpCode = 302;

		XCHAR tszRequestAddr[MAX_PATH] = {};
		XCHAR tszHostStr[128] = {};
		HttpProtocol_ServerHelp_GetField(&pptszListHdr, nHdrCount, _X("Host"), tszHostStr);

		XCHAR tszPortWebdav[64] = {};
		XCHAR tszPortDownload[64] = {};
		_xstprintf(tszPortWebdav, _X("%d"), st_ServiceCfg.nWebdavPort);
		_xstprintf(tszPortDownload, _X("%d"), st_ServiceCfg.nStorageUPPort);
		//转换端口
		int nRLen = 0;
		BaseLib_OperatorString_Replace(tszHostStr, &nRLen, tszPortWebdav, tszPortDownload, true);
		//转换地址
		XCHAR tszStroageKey[MAX_PATH] = {};
		XCHAR tszFileName[MAX_PATH] = {};

		int nRet = _stxscanf(pSt_HTTPParam->tszHttpUri + 1, _T("%99[^/]/%199[^\n]"), tszStroageKey, tszFileName);
		if (2 != nRet)
		{
			st_HDRParam.nHttpCode = 413;
			HttpProtocol_Server_SendMsgEx(xhWebdavHttp, tszSDBuffer, &nSDLen, &st_HDRParam);
			XEngine_Net_SendMsg(lpszClientAddr, tszSDBuffer, nSDLen, STORAGE_NETTYPE_HTTPWEBDAV);
			XLOG_PRINT(xhLog, XENGINE_HELPCOMPONENTS_XLOG_IN_LOGLEVEL_ERROR, _X("WEBDAV客户端:%s,处理WEBDAV协议上传方法失败,文件请求路径不正确,URL:%s"), lpszClientAddr, pSt_HTTPParam->tszHttpUri);
			return false;
		}
		_xstprintf(tszRequestAddr, _X("Location: http://%s/api?filename=%s&storeagekey=%s\r\n"), tszHostStr, tszFileName, tszStroageKey);

		HttpProtocol_Server_SendMsgEx(xhWebdavHttp, tszSDBuffer, &nSDLen, &st_HDRParam, NULL, 0, tszRequestAddr);
		XEngine_Net_SendMsg(lpszClientAddr, tszSDBuffer, nSDLen, STORAGE_NETTYPE_HTTPWEBDAV);
		XLOG_PRINT(xhLog, XENGINE_HELPCOMPONENTS_XLOG_IN_LOGLEVEL_INFO, _X("WEBDAV客户端:%s,请求文件上传被重定向到:%s"), lpszClientAddr, tszRequestAddr);
		return true;
	}
	else if (0 == _tcsxnicmp(lpszMethodLock, pSt_HTTPParam->tszHttpMethod, _tcsxlen(lpszMethodLock)))
	{
		//使用重定向实现上传
		st_HDRParam.bIsClose = false;
		st_HDRParam.nHttpCode = 200;

		XENGINE_WEBDAVLOCK st_WDLock = {};
		if (!Protocol_StorageParse_WDLock(lpszMsgBuffer, nMsgLen, &st_WDLock))
		{
			st_HDRParam.nHttpCode = 413;
			HttpProtocol_Server_SendMsgEx(xhWebdavHttp, tszSDBuffer, &nSDLen, &st_HDRParam);
			XEngine_Net_SendMsg(lpszClientAddr, tszSDBuffer, nSDLen, STORAGE_NETTYPE_HTTPWEBDAV);
			XLOG_PRINT(xhLog, XENGINE_HELPCOMPONENTS_XLOG_IN_LOGLEVEL_ERROR, _X("WEBDAV客户端:%s,处理WEBDAV协议LOCK方法失败,协议不正确,内容:%s"), lpszClientAddr, lpszMsgBuffer);
			return false;
		}
		Session_Webdav_Insert(pSt_HTTPParam->tszHttpUri, &st_WDLock);
		Protocol_StoragePacket_WDLock(tszRVBuffer, &nRVLen, &st_WDLock);

		HttpProtocol_Server_SendMsgEx(xhWebdavHttp, tszSDBuffer, &nSDLen, &st_HDRParam, tszRVBuffer, nRVLen);
		XEngine_Net_SendMsg(lpszClientAddr, tszSDBuffer, nSDLen, STORAGE_NETTYPE_HTTPWEBDAV);
		XLOG_PRINT(xhLog, XENGINE_HELPCOMPONENTS_XLOG_IN_LOGLEVEL_INFO, _X("WEBDAV客户端:%s,处理WEBDAV协议LOCK方法成功,锁用户:%s"), lpszClientAddr, st_WDLock.tszOwner);
		return true;
	}
	else if (0 == _tcsxnicmp(lpszMethodUNLock, pSt_HTTPParam->tszHttpMethod, _tcsxlen(lpszMethodUNLock)))
	{
		//使用重定向实现上传
		st_HDRParam.bIsClose = false;
		st_HDRParam.nHttpCode = 204;

		Session_Webdav_Delete(pSt_HTTPParam->tszHttpUri);
		HttpProtocol_Server_SendMsgEx(xhWebdavHttp, tszSDBuffer, &nSDLen, &st_HDRParam);
		XEngine_Net_SendMsg(lpszClientAddr, tszSDBuffer, nSDLen, STORAGE_NETTYPE_HTTPWEBDAV);
		XLOG_PRINT(xhLog, XENGINE_HELPCOMPONENTS_XLOG_IN_LOGLEVEL_INFO, _X("WEBDAV客户端:%s,处理WEBDAV协议UNLOCK方法成功,锁文件:%s"), lpszClientAddr, pSt_HTTPParam->tszHttpUri);
		return true;
		}
	else if (0 == _tcsxnicmp(lpszMethodPropPatch, pSt_HTTPParam->tszHttpMethod, _tcsxlen(lpszMethodPropPatch)))
	{
		st_HDRParam.bIsClose = false;
		st_HDRParam.nHttpCode = 207;

		std::list<string> stl_ListName;
		Protocol_StorageParse_WDPropPatch(lpszMsgBuffer, nMsgLen, &stl_ListName);

		Protocol_StoragePacket_WDPropPatch(tszRVBuffer, &nRVLen, pSt_HTTPParam->tszHttpUri, &stl_ListName);
		HttpProtocol_Server_SendMsgEx(xhWebdavHttp, tszSDBuffer, &nSDLen, &st_HDRParam, tszRVBuffer, nRVLen);
		XEngine_Net_SendMsg(lpszClientAddr, tszSDBuffer, nSDLen, STORAGE_NETTYPE_HTTPWEBDAV);
		XLOG_PRINT(xhLog, XENGINE_HELPCOMPONENTS_XLOG_IN_LOGLEVEL_INFO, _X("WEBDAV客户端:%s,处理WEBDAV协议PROPPATCH方法成功,锁用户:%s"), lpszClientAddr, pSt_HTTPParam->tszHttpUri);
		return true;
	}
	else if (0 == _tcsxnicmp(lpszMethodDel, pSt_HTTPParam->tszHttpMethod, _tcsxlen(lpszMethodDel)))
	{
		//使用重定向实现上传
		st_HDRParam.bIsClose = false;
		st_HDRParam.nHttpCode = 204;

		XENGINE_STORAGEBUCKET st_StorageBucket = {};
		if (!APIHelp_Distributed_DLStorage(pSt_HTTPParam->tszHttpUri, st_LoadbalanceCfg.st_LoadBalance.pStl_ListBucket, &st_StorageBucket))
		{
			st_HDRParam.bIsClose = true;
			st_HDRParam.nHttpCode = 400;
			HttpProtocol_Server_SendMsgEx(xhWebdavHttp, tszSDBuffer, &nSDLen, &st_HDRParam);
			XEngine_Net_SendMsg(lpszClientAddr, tszSDBuffer, nSDLen, STORAGE_NETTYPE_HTTPWEBDAV);
			XLOG_PRINT(xhLog, XENGINE_HELPCOMPONENTS_XLOG_IN_LOGLEVEL_ERROR, _X("WEBDAV客户端:%s,处理WEBDAV协议DELETE方法失败,获取BUCKET失败,URL:%s"), lpszClientAddr, pSt_HTTPParam->tszHttpUri);
			return false;
		}
		int nFLen = 0;
		XCHAR tszFileName[MAX_PATH] = {};
		_tcsxcpy(tszFileName, pSt_HTTPParam->tszHttpUri + 1);
		BaseLib_OperatorString_Replace(tszFileName, &nFLen, st_StorageBucket.tszBuckKey, st_StorageBucket.tszFilePath);

		_xtremove(tszFileName);
		HttpProtocol_Server_SendMsgEx(xhWebdavHttp, tszSDBuffer, &nSDLen, &st_HDRParam);
		XEngine_Net_SendMsg(lpszClientAddr, tszSDBuffer, nSDLen, STORAGE_NETTYPE_HTTPWEBDAV);
		XLOG_PRINT(xhLog, XENGINE_HELPCOMPONENTS_XLOG_IN_LOGLEVEL_INFO, _X("WEBDAV客户端:%s,处理WEBDAV协议DELETE方法成功,删除的文件:%s"), lpszClientAddr, tszFileName);
		return true;
	}
	else
	{
		st_HDRParam.bIsClose = true;
		st_HDRParam.nHttpCode = 405;

		HttpProtocol_Server_SendMsgEx(xhWebdavHttp, tszSDBuffer, &nSDLen, &st_HDRParam);
		XEngine_Net_SendMsg(lpszClientAddr, tszSDBuffer, nSDLen, STORAGE_NETTYPE_HTTPWEBDAV);
		XLOG_PRINT(xhLog, XENGINE_HELPCOMPONENTS_XLOG_IN_LOGLEVEL_ERROR, _X("WEBDAV客户端:%s,发送的方法不支持"), lpszClientAddr);
		return false;
	}
	
	return true;
}