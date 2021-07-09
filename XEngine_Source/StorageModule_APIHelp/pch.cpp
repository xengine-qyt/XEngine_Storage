﻿#include "pch.h"
#include "APIHelp_Distributed/APIHelp_Distributed.h"
/********************************************************************
//    Created:     2021/07/08  16:40:04
//    File Name:   D:\XEngine_Storage\XEngine_Source\StorageModule_APIHelp\pch.cpp
//    File Path:   D:\XEngine_Storage\XEngine_Source\StorageModule_APIHelp
//    File Base:   pch
//    File Ext:    cpp
//    Project:     XEngine(网络通信引擎)
//    Author:      qyt
//    Purpose:     导出函数实现
//    History:
*********************************************************************/
BOOL APIHelp_IsErrorOccur = FALSE;
DWORD APIHelp_dwErrorCode = 0;
//////////////////////////////////////////////////////////////////////////
CAPIHelp_Distributed m_APIDistributed;
//////////////////////////////////////////////////////////////////////////
//                       导出函数
//////////////////////////////////////////////////////////////////////////
extern "C" DWORD StorageHelp_GetLastError(int* pInt_SysError)
{
	if (NULL != pInt_SysError)
	{
		*pInt_SysError = errno;
	}
	return APIHelp_dwErrorCode;
}
/************************************************************************/
/*                                                                      */
/************************************************************************/
extern "C" BOOL APIHelp_Distributed_IsMode(list<int>*pStl_ListMode, int nMode)
{
	return m_APIDistributed.APIHelp_Distributed_IsMode(pStl_ListMode, nMode);
}
extern "C" BOOL APIHelp_Distributed_RandomAddr(list<tstring>*pStl_ListAddr, TCHAR * ptszAddr)
{
	return m_APIDistributed.APIHelp_Distributed_RandomAddr(pStl_ListAddr, ptszAddr);
}