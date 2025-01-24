// NetSocket.cpp : Defines the entry point for the DLL application.
//
/*--------------------------------------------------------------------
                                                                                      
                                                                                      
                        COPYRIGHT (C) 2008, IRSV Corporation
                        ALL RIGHTS RESERVED
                                                                                      
 This source code has been made available to you by IRSV on an
 AS-IS basis. Anyone receiving this source code is licensed under IRSV
 copyrights to use it in any way he or she deems fit, including copying it,
 modifying it, compiling it, and redistributing it either with or without
 modifications. Any person who transfers this source code or any derivative
 work must include the IRSV copyright notice and this paragraph in
 the transferred software.
 1.0    Initial net software                       CCL   2008.12.08
                                                                                      
-----------------------------------------------------------------------*/
//#include "stdafx.h"

#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include "NetSocket.h"
//#include "WinSock2.h"

//#include "stdafx.h"
//#include "vdefine.h"
// #include <unistd.h>

//#include "ATLCONV.H"
#include "netdefine.h"


#include<stdio.h>  
#include<stdlib.h>  
#include<string.h>  
#include<errno.h>  
//#include<netdb.h>  
//#include<sys/socket.h>  
#include<sys/types.h>  
//#include<netinet/in.h>  
//#include<unistd.h>  
#include<arpa/inet.h>
//#include<sys/ioctl.h>
#include <time.h>

#ifndef WIN32
typedef sockaddr_in SOCKADDR_IN;
#define closesocket(x) close(x)
#endif
/***************************************************************
Description:
		Connect the videoeye server.
		
Parameters:
		ipAddress		: The IP address.
		port    		: The port for network.
		
Returns:
		TRUE: Connect success.
		FALSE: Connect fail.
****************************************************************/
BOOL ConnectSever(SOCKET *Socket, char * ipAddress,UINT32 port)
{
	DisconnectSever(Socket);
	if (*Socket == INVALID_SOCKET)
	{
		if(!OpenSocket(Socket))  //port,SOCK_STREAM ,ipAddress
			return FALSE;
	}

	if (!ConnectSocket(*Socket, ipAddress,port))
	{
		
		return FALSE;
	}

	//Set receive timeout time.
	 SINT32 ret = ReceiveTimeout(*Socket,1000*2);
	if(ret != 0)
	{
			printf("ReceiveTimeout faild %d, %d \n", *Socket, ret);

	}
	 ret = SendTimeout(*Socket, 1000*2);
	if(ret != 0)
	{
			printf("SendTimeout faild %d, %d \n", *Socket, ret);

	}
	return TRUE;
}


/***************************************************************
Description:
		Close with the server.
		
Parameters:
		void.
		
Returns:
		void.
****************************************************************/
void DisconnectSever(SOCKET *hSocket)
{
	if (*hSocket != INVALID_SOCKET)
	{
#if 0
//yan change
		shutdown(m_hSocket, SD_SEND);
		shutdown(m_hSocket, SD_RECEIVE);
		Sleep(50);

		//VERIFY(SOCKET_ERROR != closesocket(m_hSocket));
		closesocket(m_hSocket);
#endif		
		*hSocket = INVALID_SOCKET;
	}
}

/***************************************************************
Description:
		Close with the server.
		
Parameters:
		void.
		
Returns:
		void.
****************************************************************/
void Disconnect(SOCKET *m_hSocket)
{
	if (*m_hSocket != INVALID_SOCKET)
	{

		shutdown(*m_hSocket, SD_SEND);
		shutdown(*m_hSocket, SD_RECEIVE);
	#ifdef WIN32	
		Sleep(50);
	#else	
   	 usleep(50*1000);
	#endif
		///VERIFY(SOCKET_ERROR != closesocket(m_hSocket));
		//close(m_hSocket);
		closesocket(*m_hSocket);
		
		*m_hSocket = INVALID_SOCKET;
	}
}

/***************************************************************
Description:
		Open the client socket.
		
Parameters:
		socketPort:	the socket port.
		socketType: the socket type.
		socketAddress: the socket address
		
Returns:
		TRUE: the bind socket success.
		FALSE: the bind socket failure.
****************************************************************/
BOOL OpenSocket(SOCKET *hSocket )
{
	int nResult;
	SINT32 socketType = SOCK_STREAM;
	if (CreateSocket(hSocket, socketType))
	{
		//if (BindingSocket(socketPort,socketAddress))   //bind ������˵ģ�����ǿͻ��ˣ�����Ҫ
			return TRUE;
//		nResult = GetLastError();
//		DisconnectSever();
//		WSASetLastError(nResult);
	}
	return FALSE;
}

/***************************************************************
Description:
		Create the client socket.
		
Parameters:
		protocolType:	the protocol type.
		socketType: the socket type.
		addressFormat: the address format.
		
Returns:
		TRUE: Create socket success.
		FALSE: Create socket failure.
****************************************************************/
BOOL CreateSocket(SOCKET *hSocket, SINT32 socketType)
{

//	ASSERT(m_hSocket == INVALID_SOCKET);
	
	SINT32 addressFormat = AF_INET;
	SINT32 protocolType = 0;
	*hSocket = socket(addressFormat,socketType,protocolType);
//	m_hSocket = WSASocket(addressFormat,socketType,protocolType,NULL,0,WSA_FLAG_OVERLAPPED);
	if (*hSocket != INVALID_SOCKET)
	{
		return TRUE;
	}
	TRACE("CreateSocket faild %d \n", *hSocket);
	return FALSE;
}

/***************************************************************
Description:
		Connect the client socket.
		
Parameters:
		hostAddress:	the host address.
		hostPort:		the host port.
		
Returns:
		TRUE: the bind socket success.
		FALSE: the bind socket failure.
****************************************************************/
BOOL ConnectSocket(SOCKET hSocket, char * hostAddress,UINT32 hostPort)
{

	SOCKADDR_IN sockAddr;
//	sockaddr_in  sockAddr;
	memset(&sockAddr,0,sizeof(sockAddr));

	char * lpszAscii = ((char *)hostAddress);
	sockAddr.sin_family = AF_INET;
	sockAddr.sin_addr.s_addr = inet_addr(lpszAscii);   

	if (sockAddr.sin_addr.s_addr == INADDR_NONE)
	{

			return FALSE;
		
	}

	sockAddr.sin_port = htons((u_short)hostPort);

	SINT32 ret = connect(hSocket, (struct sockaddr *)&sockAddr,sizeof(sockAddr));
	if(ret != SOCKET_ERROR)
		return  TRUE;
	else
	{
		//	int err = WSAGetLastError();
		//	TRACE("connect faild %d, 0x%x, port: %d, err %d. \n", m_hSocket, sockAddr.sin_addr.s_addr, sockAddr.sin_port, err);
			return FALSE;
	
	}

	
}

/***************************************************************
Description:
		Receive the packet from the network.
		
Parameters:
		lpBuf:	the data buffer.
		bufLen: the data length.
		flags: the send socket flag.
		
Returns:
		Return the data length.		
****************************************************************/
SINT32 ReceivePacket(SOCKET hSocket, void* lpBuf, UINT32 bufLen)
{
	int nLeft, nResult, nCount=0;
	char* pBuf = (char *)lpBuf;
	nLeft = bufLen;
	UINT32 flags = 0;
	while(nLeft > 0)
	{
		while ((nResult = ReceiveBuffer(hSocket,pBuf, nLeft, flags)) == SOCKET_ERROR)
		{

#ifdef WIN32
			//Receive timeout, deal with it
			int Error = WSAGetLastError();
			switch (Error)
			{
			case WSAECONNRESET:
			case WSAEHOSTDOWN:
			case WSAECONNABORTED:
				//Socket error because the server close
				return -1;
			//No data read
			case WSAEWOULDBLOCK:
				break;
			//Other error
			default:
				if(nCount++>=2)// || theApp.pUserManager.m_bStopSocket)
				{
					//No data from the server, disconnect
					return -1;
				}
			}
 #endif
		}

		nCount = 0;
		nLeft -= nResult;
		pBuf += nResult;
	}

	return (bufLen - nLeft);
}

/***************************************************************
Description:
Receive the Respond from the network.

Parameters:
lpBuf:	the data buffer.
bufLen: the data length.
flags: the send socket flag.

Returns:
Return the data length.
****************************************************************/
SINT32 ReceiveRespond(SOCKET hSocket, void* lpBuf, UINT32 bufLen)
{
	int nResult, nCount = 0;
	unsigned int nLeft;
	char* pBuf = (char *)lpBuf;
	UINT32 flags = 0;

	if (ReceiveBuffer(hSocket, pBuf, 4, flags) == SOCKET_ERROR)
	{
		return -1;
	}
	memcpy(&nLeft, pBuf, 4);
	nLeft = nLeft <= (bufLen ) ? (nLeft-4) : (bufLen - 4);
	bufLen = nLeft + 4;
	pBuf += 4;
	while (nLeft > 0)
	{
		while ((nResult = ReceiveBuffer(hSocket, pBuf, nLeft, flags)) == SOCKET_ERROR)
		{

			//			if(theApp.pUserManager.m_bDisconnect || theApp.GetVideoDlg()->m_bExit)
			//				return -1;
#ifdef WIN32
			//Receive timeout, deal with it
			int Error = WSAGetLastError();
			switch (Error)
			{
			case WSAECONNRESET:
			case WSAEHOSTDOWN:
			case WSAECONNABORTED:
				//Socket error because the server close
				return -1;
				//No data read
			case WSAEWOULDBLOCK:
				break;
				//Other error
			default:
				if (nCount++ >= 2)// || theApp.pUserManager.m_bStopSocket)
				{
					//No data from the server, disconnect
					return -1;
				}
			}
#endif
		}

		nCount = 0;
		nLeft -= nResult;
		pBuf += nResult;
	}

	return (bufLen - nLeft);
}

/***************************************************************
Description:
		Receive the data.
		
Parameters:
		lpBuf:	the data buffer.
		bufLen: the data length.
		flags: the send socket flag.
		
Returns:
		Return the data length.		
****************************************************************/
SINT32 ReceiveBuffer(SOCKET m_hSocket, void* lpBuf, UINT32 bufLen, UINT32 flags)
{
	return recv(m_hSocket, (char*)lpBuf, bufLen, flags);
}

/***************************************************************	
Description:
		Send the packet to network.
		
Parameters:
		lpBuf:	the data buffer.
		bufLen: the data length.
		flags: the send socket flag.
		
Returns:
		Return the data length.		
****************************************************************/
SINT32 SendPactket(SOCKET *hSocket, const void* lpBuf, UINT32 bufLen)
{
	int nLeft, nWritten, nCount=0;
	char* pBuf = (char*)lpBuf;
	nLeft = bufLen;
	UINT32 flags = 0;
	while (nLeft > 0)
	{
		while ((nWritten = SendBuffer(*hSocket,pBuf, nLeft, flags)) == SOCKET_ERROR)
		{
			if(nCount++>100 )//|| theApp.pUserManager.m_bStopSocket)
				return -1;
#ifdef WIN32
			//Send timeout, deal with it
			int Error = WSAGetLastError();
			switch (Error) 
			{
			case WSAECONNRESET:
			case WSAEHOSTDOWN:
			case WSAECONNABORTED:
				//Socket error because the server close
				Disconnect(hSocket);
				return -1;
			//No data read
			case WSAEWOULDBLOCK:
				break;
			//Other error
			default: 
				if(nCount++>=2)// || theApp.pUserManager.m_bStopSocket)
				{
					//No data from server, disconnect
					return -1;
				}
			}
      #endif
		}

		nCount=0;
		nLeft -= nWritten;
		pBuf += nWritten;
	}
	return (bufLen - nLeft);
}

/***************************************************************
Description:
		Send the data.
		
Parameters:
		lpBuf:	the data buffer.
		bufLen: the data length.
		flags: the send socket flag.
		
Returns:
		Return the data length.		
****************************************************************/
SINT32 SendBuffer(SOCKET m_hSocket, const void* lpBuf, UINT32 bufLen, UINT32 flags)
{
	return send(m_hSocket, (char *)lpBuf, bufLen, flags);
}

/***************************************************************
Description:
		Set send timeout time.
		
Parameters:
		timeout:	the send timeout time.
		
Returns:
		Return the success and failure.		
****************************************************************/
SINT32 SendTimeout(SOCKET hSocekt, UINT32 timeout)
{	
#ifdef WIN32

	if(timeout < 0 )
		return 1;
	if(setsockopt(hSocekt, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeout, sizeof(timeout) ) == SOCKET_ERROR )
		return -1;
#else
    struct timeval stru_timeout;
	if(timeout < 0 )
		return 1;
	stru_timeout.tv_sec=timeout/1000;
  	stru_timeout.tv_usec=timeout%1000;
	if(setsockopt( hSocekt, SOL_SOCKET, SO_SNDTIMEO, (char*)&stru_timeout, sizeof(stru_timeout) ) == SOCKET_ERROR )
		return -1;
	
#endif
	return 0;
}

/***************************************************************
Description:
		Set receive timeout time.
		
Parameters:
		timeout:	the receive timeout time.
		
Returns:
		Return the success and failure.		
****************************************************************/
SINT32 ReceiveTimeout(SOCKET hSocekt, UINT32 timeout)
{	

#ifdef WIN32
	if(timeout < 0 )
		return 1;
		
	
	int iMode = 0;//block
	ioctlsocket(hSocekt, FIONBIO, (u_long FAR*) &iMode);
	if (setsockopt(hSocekt, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout)) == SOCKET_ERROR)
	{
		printf("set rece timeout fail\n\r");
		return -1;
	}
	
#else
	if(timeout < 0 )
		return 1;
  struct timeval stru_timeout;		
	stru_timeout.tv_sec=timeout/1000;
  stru_timeout.tv_usec=timeout%1000;
	
//	int iMode = 0;//block
//	ioctlsocket(hSocekt, FIONBIO, (u_long FAR*) &iMode);
	if (setsockopt(hSocekt, SOL_SOCKET, SO_RCVTIMEO, (char*)&stru_timeout, sizeof(stru_timeout)) == SOCKET_ERROR)
	{
		printf("set rece timeout fail\n\r");
		return -1;
	}


#endif

	
	return 0;
}

/*
void CVeSocket::Exit()
{
	if(m_hSocket != INVALID_SOCKET)
	//	close(m_hSocket);	
		closesocket(m_hSocket);
}
*/