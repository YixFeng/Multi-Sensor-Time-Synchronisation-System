/*--------------------------------------------------------------------
                                                                                      
                                                                                      
                        COPYRIGHT (C) 2008,  Corporation
                        ALL RIGHTS RESERVED
                                                                                      
 This source code has been made available to you by  on an
 AS-IS basis. Anyone receiving this source code is licensed under 
 copyrights to use it in any way he or she deems fit, including copying it,
 modifying it, compiling it, and redistributing it either with or without
 modifications. Any person who transfers this source code or any derivative
 work must include the copyright notice and this paragraph in
 the transferred software.
 1.0    Initial net software                       CCL   2008.12.08
                                                                                      
-----------------------------------------------------------------------*/
#ifdef NETSOCKET_EXPORTS
#define NETSOCKET_API __declspec(dllexport)
#else
#define NETSOCKET_API __declspec(dllimport)
#endif


#ifndef _NET_SOCKET_H_
#define _NET_SOCKET_H_

#pragma comment(lib, "Ws2_32.lib")

#ifdef SDK_LIB
#include <WinSock2.h>
#endif
//#include "vdefine.
//#include <sys/socket.h>  
#include <stddef.h>

#include "netdefine.h"

//class CNVServer;
#define BUFSIZE  2*1024*1024//16�ı���


	//Connect the videoeye server.
	//BOOL ConnectSever(char * ipAddress,UINT32 port);
	BOOL ConnectSever(SOCKET *Socket, char * ipAddress, UINT32 port);
	//Close with server
	void DisconnectSever(SOCKET *hSocket);

	//void Disconnect();// ʵ�ʹر�socket
	void Disconnect(SOCKET *m_hSocket);
	//Open the client socket.
	BOOL OpenSocket(SOCKET *hSocket);
	//Binding the socket.
	//BOOL BindingSocket(UINT32 socketPort, char * socketAddress = NULL);

	//Create the socket
	BOOL CreateSocket(SOCKET *hSocket, SINT32 socketType);

	//Connect the socket
	BOOL ConnectSocket(SOCKET hSocket, char * hostAddress, UINT32 hostPort);
	//Set send timeout time.
	SINT32 SendTimeout(SOCKET hSocekt, UINT32 timeout);

	//Set receive timeout time.
	SINT32 ReceiveTimeout(SOCKET hSocekt, UINT32 timeout);

	//Receive the packet from network.
	SINT32 ReceivePacket(SOCKET hSocket, void* lpBuf, UINT32 bufLen);
	SINT32 ReceiveRespond(SOCKET hSocket, void* lpBuf, UINT32 bufLen);

	//Send the packet to network.
	SINT32 SendPactket(SOCKET *hSocket, const void* lpBuf, UINT32 bufLen);

	//Receive the data
	//SINT32 ReceiveBuffer(void* lpBuf, UINT32 bufLen, UINT32 flags);
	SINT32 ReceiveBuffer(SOCKET m_hSocket, void* lpBuf, UINT32 bufLen, UINT32 flags);

	//Send the data.
	//SINT32 SendBuffer(const void* lpBuf, UINT32  bufLen, UINT32 flags);
	SINT32 SendBuffer(SOCKET m_hSocket, const void* lpBuf, UINT32 bufLen, UINT32 flags);
	//Exit socket
	void Exit();




#endif

