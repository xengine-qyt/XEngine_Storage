﻿#include "../StorageApp_Hdr.h"

BOOL XEngine_APPHelp_ProxyAuth(LPCTSTR lpszClientAddr, LPCTSTR lpszMethod, LPCTSTR lpszPostUrl, TCHAR** pptszListHdr, int nHdrCount, int nSDType)
{
	int nSDLen = 1024;
	int nAuthType = 0;
	int nAuthLen = MAX_PATH;
	LPCTSTR lpszClientType;
	TCHAR tszAuthStr[MAX_PATH];
	TCHAR tszSDBuffer[1024];
	RFCCOMPONENTS_HTTP_HDRPARAM st_HDRParam;

	memset(tszAuthStr, '\0', MAX_PATH);
	memset(tszSDBuffer, '\0', sizeof(tszSDBuffer));
	memset(&st_HDRParam, '\0', sizeof(RFCCOMPONENTS_HTTP_HDRPARAM));

	if (!st_ServiceCfg.st_XProxy.st_XProxyAuth.bAuth)
	{
		return TRUE;
	}

	if (STORAGE_NETTYPE_HTTPDOWNLOAD == nSDType)
	{
		lpszClientType = _T("下载客户端");
	}
	else if (STORAGE_NETTYPE_HTTPUPLOADER == nSDType)
	{
		lpszClientType = _T("上传客户端");
	}
	else
	{
		lpszClientType = _T("业务客户端");
	}

	if (!RfcComponents_HttpHelp_GetAuthInfo(&pptszListHdr, nHdrCount, tszAuthStr, &nAuthLen, &nAuthType))
	{
		st_HDRParam.bIsClose = TRUE;
		st_HDRParam.bAuth = TRUE;
		st_HDRParam.nHttpCode = 402;

		RfcComponents_HttpServer_SendMsgEx(xhCenterHttp, tszSDBuffer, &nSDLen, &st_HDRParam);
		XEngine_Net_SendMsg(lpszClientAddr, tszSDBuffer, nSDLen, nSDType);
		XLOG_PRINT(xhLog, XENGINE_HELPCOMPONENTS_XLOG_IN_LOGLEVEL_ERROR, _T("%s:%s,没有验证信息,无法继续"), lpszClientType, lpszClientAddr);
		return FALSE;
	}

	if (1 == nAuthType)
	{
		TCHAR tszUserName[64];
		TCHAR tszUserPass[64];

		memset(tszUserName, '\0', sizeof(tszUserName));
		memset(tszUserPass, '\0', sizeof(tszUserPass));

		OPenSsl_Help_BasicDecoder(tszAuthStr, tszUserName, tszUserPass);
		if (_tcslen(st_ServiceCfg.st_XProxy.st_XProxyAuth.tszAuthProxy) > 0)
		{
			int nBLen = 0;
			int nResponseCode = 0;
			TCHAR* ptszBody = NULL;
			
			Protocol_StoragePacket_BasicAuth(lpszMethod, lpszPostUrl, lpszClientAddr, tszUserName, tszUserPass, tszSDBuffer, &nSDLen);
			APIHelp_HttpRequest_Post(st_ServiceCfg.st_XProxy.st_XProxyAuth.tszAuthProxy, tszSDBuffer, &nResponseCode, &ptszBody, &nBLen);
			if (200 == nResponseCode)
			{
				XLOG_PRINT(xhLog, XENGINE_HELPCOMPONENTS_XLOG_IN_LOGLEVEL_INFO, _T("%s:%s,代理服务:%s 验证通过,用户名:%s,密码:%s"), lpszClientType, lpszClientAddr, st_ServiceCfg.st_XProxy.st_XProxyAuth.tszAuthProxy, tszUserName, tszUserPass);
			}
			else
			{
				st_HDRParam.bIsClose = TRUE;
				st_HDRParam.bAuth = TRUE;
				st_HDRParam.nHttpCode = nResponseCode;

				RfcComponents_HttpServer_SendMsgEx(xhCenterHttp, tszSDBuffer, &nSDLen, &st_HDRParam);
				XEngine_Net_SendMsg(lpszClientAddr, tszSDBuffer, nSDLen, nSDType);
				XLOG_PRINT(xhLog, XENGINE_HELPCOMPONENTS_XLOG_IN_LOGLEVEL_ERROR, _T("%s:%s,用户验证失败,用户名:%s,密码:%s,错误码:%d,错误内容:%s"), lpszClientType, lpszClientAddr, tszUserPass, tszUserPass, nResponseCode, ptszBody);
			}
			BaseLib_OperatorMemory_FreeCStyle((VOID**)&ptszBody);
		}
		else
		{
			if (!Session_User_Exist(tszUserName, tszUserPass))
			{
				st_HDRParam.bIsClose = TRUE;
				st_HDRParam.bAuth = TRUE;
				st_HDRParam.nHttpCode = 402;

				RfcComponents_HttpServer_SendMsgEx(xhCenterHttp, tszSDBuffer, &nSDLen, &st_HDRParam);
				XEngine_Net_SendMsg(lpszClientAddr, tszSDBuffer, nSDLen, nSDType);
				XLOG_PRINT(xhLog, XENGINE_HELPCOMPONENTS_XLOG_IN_LOGLEVEL_ERROR, _T("%s:%s,验证用户失败,无法继续"), lpszClientType, lpszClientAddr);
				return FALSE;
			}
			XLOG_PRINT(xhLog, XENGINE_HELPCOMPONENTS_XLOG_IN_LOGLEVEL_INFO, _T("%s:%s,本地验证用户验证通过,用户名:%s,密码:%s"), lpszClientType, lpszClientAddr, tszUserName, tszUserPass);
		}
	}
	else
	{
		st_HDRParam.bIsClose = TRUE;
		st_HDRParam.bAuth = TRUE;
		st_HDRParam.nHttpCode = 402;

		RfcComponents_HttpServer_SendMsgEx(xhCenterHttp, tszSDBuffer, &nSDLen, &st_HDRParam);
		XEngine_Net_SendMsg(lpszClientAddr, tszSDBuffer, nSDLen, nSDType);
		XLOG_PRINT(xhLog, XENGINE_HELPCOMPONENTS_XLOG_IN_LOGLEVEL_ERROR, _T("%s:%s,没有验证信息,无法继续"), lpszClientType, lpszClientAddr);
		return FALSE;
	}
	return TRUE;
}
BOOL XEngine_APPHelp_RangeFile(LPCTSTR lpszClientAddr, int* pInt_SPos, int* pInt_EPos, __int64x* pInt_Count, TCHAR** pptszListHdr, int nHdrCount, int nSDType)
{
	LPCTSTR lpszRangeStr = _T("Range");
	LPCTSTR lpszLengthStr = _T("Content-Length");
	LPCTSTR lpszClientType;
	TCHAR tszKeyStr[128];
	TCHAR tszValueStr[128];
	TCHAR tszFieldStr[128];

	memset(tszKeyStr, '\0', sizeof(tszKeyStr));
	memset(tszValueStr, '\0', sizeof(tszValueStr));
	memset(tszFieldStr, '\0', sizeof(tszFieldStr));

	if (STORAGE_NETTYPE_HTTPDOWNLOAD == nSDType)
	{
		lpszClientType = _T("下载客户端");
	}
	else if (STORAGE_NETTYPE_HTTPUPLOADER == nSDType)
	{
		lpszClientType = _T("上传客户端");
	}
	else
	{
		lpszClientType = _T("业务客户端");
	}
	//是否有范围
	if (RfcComponents_HttpHelp_GetField(&pptszListHdr, nHdrCount, lpszRangeStr, tszFieldStr))
	{
		//是否没有找到
		int nBPos = 0;  //某些时候有个BYTE   
		if (NULL != _tcsstr(tszFieldStr, _T("bytes=")))
		{
			nBPos = 6;
		}
		if (!BaseLib_OperatorString_GetKeyValue(tszFieldStr + nBPos, "-", tszKeyStr, tszValueStr))
		{
			XLOG_PRINT(xhLog, XENGINE_HELPCOMPONENTS_XLOG_IN_LOGLEVEL_ERROR, _T("%s:%s,请求内容有范围信息,但是解析失败,内容:%s"), lpszClientType, lpszClientAddr, tszFieldStr);
			return FALSE;
		}
		//得到 1/2 如果有的话
		TCHAR tszRangeEnd[128];
		TCHAR tszRangeCount[128];

		memset(tszRangeEnd, '\0', sizeof(tszRangeEnd));
		memset(tszRangeCount, '\0', sizeof(tszRangeCount));
		if (BaseLib_OperatorString_GetKeyValue(tszValueStr, "/", tszRangeEnd, tszRangeCount))
		{
			*pInt_SPos = _ttoi(tszKeyStr);
			*pInt_EPos = _ttoi(tszRangeEnd);
			*pInt_Count = _ttoi64(tszRangeCount);
		}
		else
		{
			*pInt_SPos = _ttoi(tszKeyStr);
			*pInt_EPos = _ttoi(tszValueStr);
		}
		XLOG_PRINT(xhLog, XENGINE_HELPCOMPONENTS_XLOG_IN_LOGLEVEL_INFO, _T("%s:%s,客户端的请求设置了数据范围:%d - %d/%lld"), lpszClientType, lpszClientAddr, *pInt_SPos, *pInt_EPos, *pInt_Count);
	}
	else
	{
		RfcComponents_HttpHelp_GetField(&pptszListHdr, nHdrCount, lpszLengthStr, tszFieldStr);
		*pInt_Count = _ttoi64(tszFieldStr);
		XLOG_PRINT(xhLog, XENGINE_HELPCOMPONENTS_XLOG_IN_LOGLEVEL_INFO, _T("%s:%s,请求内容是新的数据,大小:%lld"), lpszClientType, lpszClientAddr, *pInt_Count);
	}
	return TRUE;
}
BOOL XEngine_APPHelp_VerHash(LPCTSTR lpszClientAddr, LPCTSTR lpszFileName, LPCTSTR lpszFileHash, TCHAR** pptszListHdr, int nHdrCount)
{
	if (st_ServiceCfg.st_XStorage.bUPHash)
	{
		LPCTSTR lpszKeyStr = _T("FileHash");
		TCHAR tszValueStr[MAX_PATH];
		memset(tszValueStr, '\0', MAX_PATH);

		if (RfcComponents_HttpHelp_GetField(&pptszListHdr, nHdrCount, lpszKeyStr, tszValueStr))
		{
			if (0 == _tcsnicmp(lpszFileHash, tszValueStr, _tcslen(lpszFileHash)))
			{
				_tremove(lpszFileName);
				XLOG_PRINT(xhLog, XENGINE_HELPCOMPONENTS_XLOG_IN_LOGLEVEL_ERROR, _T("上传客户端:%s,上传的文件信息HASH效验成功,文件:%s 原始HASH:%s,计算HASH:%s"), lpszClientAddr, lpszFileName, tszValueStr, lpszFileHash);
			}
			else
			{
				_tremove(lpszFileName);
				XLOG_PRINT(xhLog, XENGINE_HELPCOMPONENTS_XLOG_IN_LOGLEVEL_ERROR, _T("上传客户端:%s,上传的文件信息HASH校验失败,无法继续,文件:%s 已被删除,原始HASH:%s,计算HASH:%s"), lpszClientAddr, lpszFileName, tszValueStr, lpszFileHash);
				return FALSE;
			}
		}
		else
		{
			XLOG_PRINT(xhLog, XENGINE_HELPCOMPONENTS_XLOG_IN_LOGLEVEL_ERROR, _T("上传客户端:%s,上传的信息没有附带HASH值,无法验证文件的正确性"), lpszClientAddr);
			return FALSE;
		}
	}
	return TRUE;
}