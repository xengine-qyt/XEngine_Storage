#pragma once
/********************************************************************
//	Created:	2013/1/18  12:24
//	File Name: 	G:\U_DISK_Path\NetSocketEngine\NetEngine_P2xp\NetEngine_P2XPProtocol\P2XPProtocol_Define.h
//	File Path:	G:\U_DISK_Path\NetSocketEngine\NetEngine_P2xp\NetEngine_P2XPProtocol
//	File Base:	P2XPProtocol_Define
//	File Ext:	h
//  Project:    NetSocketEngine(网络通信引擎)
//	Author:		qyt
//	Purpose:	P2XP协议构造分析器导出模块
//	History:
*********************************************************************/
//////////////////////////////////////////////////////////////////////////
//                        导出函数
//////////////////////////////////////////////////////////////////////////
extern "C" DWORD P2XPProtocol_GetLastError(int *pInt_ErrorCode = NULL);
/************************************************************************/
/*                        解析协议导出                                  */
/************************************************************************/
/********************************************************************
函数名称：P2XPProtocol_Parse_Login
函数功能：登录解析函数
 参数.一：lpszMsgBuffer
  In/Out：In
  类型：常量字符指针
  可空：N
  意思：输入要解析的缓冲区
 参数.二：nMsgLen
  In/Out：In
  类型：整数型
  可空：N
  意思：输入缓冲区大小
 参数.三：pSt_P2XPPeer
  In/Out：In
  类型：数据结构指针
  可空：N
  意思：输出解析后的节点信息
返回值
  类型：逻辑型
  意思：是否成功
备注：
*********************************************************************/
extern "C" BOOL P2XPProtocol_Parse_Login(LPCTSTR lpszMsgBuffer, int nMsgLen, XENGINE_P2XPPEER_PROTOCOL* pSt_P2XPPeer);
/********************************************************************
函数名称：P2XPProtocol_Parse_List
函数功能：解析列表请求
 参数.一：lpszMsgBuffer
  In/Out：In
  类型：常量字符指针
  可空：N
  意思：输入要解析的缓冲区
 参数.二：nMsgLen
  In/Out：In
  类型：整数型
  可空：N
  意思：输入缓冲区大小
 参数.三：ptszPubAddr
  In/Out：Out
  类型：字符指针
  可空：N
  意思：输出公有地址
 参数.四：ptszPriAddr
  In/Out：Out
  类型：字符指针
  可空：N
  意思：输出私有地址
返回值
  类型：逻辑型
  意思：是否成功
备注：
*********************************************************************/
extern "C" BOOL P2XPProtocol_Parse_List(LPCTSTR lpszMsgBuffer, int nMsgLen, TCHAR* ptszPubAddr, TCHAR* ptszPriAddr);
/********************************************************************
函数名称：P2XPProtocol_Parse_List
函数功能：解析列表请求
 参数.一：lpszMsgBuffer
  In/Out：In
  类型：常量字符指针
  可空：N
  意思：输入要解析的缓冲区
 参数.二：nMsgLen
  In/Out：In
  类型：整数型
  可空：N
  意思：输入缓冲区大小
 参数.三：ptszUserName
  In/Out：Out
  类型：字符指针
  可空：N
  意思：输出查询的用户名
返回值
  类型：逻辑型
  意思：是否成功
备注：
*********************************************************************/
extern "C" BOOL P2XPProtocol_Parse_User(LPCTSTR lpszMsgBuffer, int nMsgLen, TCHAR* ptszUserName);
/********************************************************************
函数名称：P2XPProtocol_Parse_Connect
函数功能：请求连接到指定地址
 参数.一：lpszMsgBuffer
  In/Out：In
  类型：常量字符指针
  可空：N
  意思：输入要解析的缓冲区
 参数.二：nMsgLen
  In/Out：In
  类型：整数型
  可空：N
  意思：输入缓冲区大小
 参数.三：pSt_P2XPPeer
  In/Out：In
  类型：数据结构指针
  可空：N
  意思：输出解析后的节点信息
返回值
  类型：逻辑型
  意思：是否成功
备注：
*********************************************************************/
extern "C" BOOL P2XPProtocol_Parse_Connect(LPCTSTR lpszMsgBuffer, int nMsgLen, XENGINE_P2XPIO_PROTOCOL* pSt_IOProtocol);
/************************************************************************/
/*                        打包协议导出                                  */
/************************************************************************/
/********************************************************************
函数名称：P2XPProtocol_Parse_Login
函数功能：登录解析函数
 参数.一：lpszMsgBuffer
  In/Out：In
  类型：常量字符指针
  可空：N
  意思：输入要解析的缓冲区
 参数.二：nMsgLen
  In/Out：In
  类型：整数型
  可空：N
  意思：输入缓冲区大小
 参数.三：pSt_P2XPPeer
  In/Out：In
  类型：数据结构指针
  可空：N
  意思：输出解析后的节点信息
返回值
  类型：逻辑型
  意思：是否成功
备注：
*********************************************************************/
extern "C" BOOL P2XPProtocol_Packet_Common(TCHAR* ptszMsgBuffer, int* pInt_MsgLen, int nCode = 0, LPCTSTR lpszMsgBuffer = NULL);
/********************************************************************
函数名称：P2XPProtocol_Packet_Lan
函数功能：响应同步局域网地址列表
 参数.一：pppSt_ListClients
  In/Out：In
  类型：三级指针
  可空：N
  意思：输入局域网地址信息列表
 参数.二：nListCount
  In/Out：In
  类型：整数型
  可空：N
  意思：局域网地址列表个数
 参数.三：ptszMsgBuffer
  In/Out：Out
  类型：字符指针
  可空：N
  意思：导出封装好的缓冲区
 参数.四：pInt_Len
  In/Out：In/Out
  类型：整数型指针
  可空：N
  意思：输入你的缓冲区大小,输出缓冲区真实大小
返回值
  类型：逻辑型
  意思：是否成功
备注：
*********************************************************************/
extern "C" BOOL P2XPProtocol_Packet_Lan(XENGINE_P2XPPEER_PROTOCOL*** pppSt_ListClients, int nListCount, TCHAR* ptszMsgBuffer, int* pInt_Len);
/********************************************************************
函数名称：P2XPProtocol_Packet_User
函数功能：响应用户查询用户信息的请求协议封包函数
 参数.一：pSt_PeerInfo
  In/Out：In
  类型：数据结构指针
  可空：N
  意思：输入获取到的用户信息
 参数.二：ptszMsgBuffer
  In/Out：Out
  类型：字符指针
  可空：N
  意思：导出封装好的缓冲区
 参数.三：pInt_Len
  In/Out：In/Out
  类型：整数型指针
  可空：N
  意思：输入你的缓冲区大小,输出缓冲区真实大小
返回值
  类型：逻辑型
  意思：是否成功
备注：
*********************************************************************/
extern "C" BOOL P2XPProtocol_Packet_User(XENGINE_P2XPPEER_PROTOCOL* pSt_PeerInfo, TCHAR* ptszMsgBuffer, int* pInt_Len);