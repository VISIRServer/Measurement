/**** BEGIN LICENSE BLOCK ****
 * This file is a part of the VISIR(TM) (Virtual Systems in Reality)
 * Software package.
 * 
 * VISIR(TM) is used to open laboratories for remote operation and control
 * as a supplement and a complement to local use.
 * 
 * VISIR(TM) is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2, as published by the Free Software Foundation.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. No liability
 * can be imposed for any impact on any equipment by the software. See
 * the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 **** END LICENSE BLOCK ****/

/*
 * Copyright (c) 2005-2009 Johan Zackrisson
 * All Rights Reserved.
 */

#include "socket.h"
#include <winsock2.h>
#include <syslog.h>

#include <string>

using namespace Net;

#define UNINITIALIZED_SOCKET 0x00
#define SERVER_SOCKET 0x01
#define CLIENT_SOCKET 0x02

/// We don't want to expose the winsock api from the network library.. so we do this silly wrapping instead.
struct Net::realsocket
{
	SOCKET	socket;
	int		flags;
};

Socket::Socket(BlockingMode mode)
{
	mMode			= mode;
	mSelectMask		= NET_ALL_FLAGS;

	mSocket			= new realsocket();
	mSocket->socket	= UNINITIALIZED_SOCKET;
	mSocket->flags	= UNINITIALIZED_SOCKET;
}

Socket::Socket(realsocket* pSocket)
{
	
	mMode			= Blocking;			// set blocking as default
	mSelectMask		= NET_ALL_FLAGS;	// select on all
	mSocket			= pSocket;	
}

Socket::~Socket()
{
	Destroy();
	delete mSocket;
}

bool Socket::Init()
{
	// check for winsock version 2.0<
	WSADATA wsaData;

	int verh = 2;	int verl = 0;
	int err = WSAStartup( MAKEWORD(verh,verl) , &wsaData );
	if ( err != 0 ) return false;
	if ( LOBYTE( wsaData.wVersion ) != verh ||	HIBYTE( wsaData.wVersion ) != verl )
	{
		WSACleanup();
		return false;
	}
	return true;
}

bool Socket::StartServer(int port, int backlog)
{
	if (!CreateTCPSocket()) return false;
	mSocket->flags = SERVER_SOCKET;

	struct sockaddr_in my_addr;
	memset(&my_addr,0,sizeof(struct sockaddr_in));
	my_addr.sin_family		= AF_INET;			// host byte order
	my_addr.sin_port		= htons(port);		// short, network byte order
	my_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	// bind socket
	if (bind(mSocket->socket, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == SOCKET_ERROR)
	{
		return false;
	}

	int on = 1;
	if (setsockopt(mSocket->socket, SOL_SOCKET, SO_EXCLUSIVEADDRUSE, (const char*)&on, sizeof(on)) < 0)
	{
		return false;
	}

	// listen
	if (listen(mSocket->socket, backlog+3) == SOCKET_ERROR)
	{
		return false;
	}

	return true;
}

bool Socket::HasConnecting()
{
	if (!CheckSocket()) return false;

	struct timeval time;	
	fd_set readfds, writefds, exceptfds;
	time.tv_sec = 0; time.tv_usec = 0;

	FD_ZERO(&readfds);
	FD_ZERO(&writefds);
	FD_ZERO(&exceptfds);

	FD_SET(mSocket->socket,&readfds);

	if (select(0,&readfds,&writefds,&exceptfds,&time) == SOCKET_ERROR)
	{
		// throw exception
		return false;
	}

	if (FD_ISSET(mSocket->socket, &readfds))
	{
		return true;
	}
	return false;
}

realsocket* Socket::AcceptConnecting()
{
    // it should be safe to accept.. if HasConnecting is called before this one
	//Socket* newSocket = new Socket(this->mMode);
	realsocket* pSocket = new realsocket();

	int addr_size = sizeof(struct sockaddr_in);
	struct sockaddr their_addr;

	if ((pSocket->socket = accept(mSocket->socket,&their_addr,&addr_size)) == INVALID_SOCKET)
	{
		HandleError();
		delete pSocket;
		return NULL;
	}
	else
	{
		pSocket->flags = CLIENT_SOCKET;
		return pSocket;
	}
}

bool Socket::Connect(const char* addr, int port)
{
	if (!CreateTCPSocket()) return false;
//	if (!SetNonBlocking()) return false;

	struct hostent *h;
	struct sockaddr_in dest_addr;

	// TODO: make this non-blocking as well.. dns lookups take time
	if ((h=gethostbyname(addr)) == NULL)
	{
		return false;
	}

	memset(&dest_addr,0,sizeof(struct sockaddr_in));
	dest_addr.sin_family = AF_INET;
	dest_addr.sin_port = htons(port);
    dest_addr.sin_addr.s_addr = inet_addr(inet_ntoa(*((struct in_addr *)h->h_addr)));

	if ((connect(mSocket->socket, (struct sockaddr *)&dest_addr, sizeof(struct sockaddr))) == SOCKET_ERROR)
	{
		if (mMode == NonBlocking)
		{
			if (WSAGetLastError() == WSAEWOULDBLOCK) return true;
		}

		HandleError();
		return false;
	}
	return true;
}

bool Socket::Destroy()
{
	if (mSocket->socket == UNINITIALIZED_SOCKET) return true;

	if (closesocket(mSocket->socket) == SOCKET_ERROR)
	{
		HandleError();
		return false;
	}

	mSocket->socket = UNINITIALIZED_SOCKET;

	return true;
}

bool Socket::Disconnect()
{
	shutdown(mSocket->socket, SD_SEND);
	return true;
}

bool Socket::IsConnected() const
{
	return CheckSocket();
}

signed int Socket::Receive(void* buffer, size_t len)
{
	if (mMode == NonBlocking)
	{
		fd_set readfds, writefds, exceptfds;
		struct timeval time;
		time.tv_sec = 0; time.tv_usec = 0;

		FD_ZERO(&readfds);
		FD_ZERO(&writefds);
		FD_ZERO(&exceptfds);

		FD_SET(mSocket->socket,&readfds);

		if (select(0,&readfds,&writefds,&exceptfds,&time) == SOCKET_ERROR)
		{
			HandleError();
			return -1;
		}

		if (FD_ISSET(mSocket->socket, &readfds))
		{
			int read = recv(mSocket->socket, (char*) buffer, (int)len,0);
			if (read == SOCKET_ERROR)
			{
				HandleError();
				return -1;
			}
			if (read == 0)
			{
				CloseSocket();
			}
			return read;
		}
		return 0;
	}
	else
	{
		int read = recv(mSocket->socket, (char*) buffer, (int)len,0);
		if (read == SOCKET_ERROR)
		{
			HandleError();
			return -1;
		}
		if (read == 0)
		{
			CloseSocket();
		}
		return read;
	}
}

signed int Socket::Send(const void* buffer,	size_t len)
{
	// Should select to se if its OK to send..
	return send(mSocket->socket, (const char*) buffer, (int)len,0);
}

bool Socket::SetNonBlocking()
{
	if (!CreateTCPSocket()) return false;

	unsigned long ioctl_opt = 1;
	if (ioctlsocket(mSocket->socket,FIONBIO,&ioctl_opt) == SOCKET_ERROR)
	{
		HandleError();
		return false;
	}

	mMode = NonBlocking;
	return true;
}

bool Socket::CheckSocket() const
{
	if (mSocket->socket == UNINITIALIZED_SOCKET) return false;
	return true;
}

bool Socket::CreateTCPSocket()
{
	if (!CheckSocket())
	{
		if ((mSocket->socket = socket(AF_INET,SOCK_STREAM,0)) == INVALID_SOCKET)
		{
			return false;
		}
		return true;
	}

	// reuse previous socket
	return true;
}

bool Socket::CloseSocket()
{
	return (Disconnect() == 1);
}

void Socket::HandleError()
{
	int error = WSAGetLastError();

	char* errorstring = "<unknown>";
	switch(error)
	{
	case WSAEINVAL:			errorstring = "WSAEINVAL";			break;
	case WSAENOTCONN:		errorstring = "WSAENOTCONN";		break;
	case WSAECONNREFUSED:	errorstring = "WSAECONNREFUSED";	break;
	case WSAEWOULDBLOCK:	errorstring = "WSAEWOULDBLOCK";		break;
	case WSAECONNRESET:		errorstring = "WSAECONNRESET";		break;
	case WSAENOTSOCK:		errorstring = "WSAENOTSOCK";		break;
	case WSAECONNABORTED:	errorstring = "WSAECONNABORTED";	break;
	default:
		errorstring = "<unknown>";
	}

	syserr << "*** SOCKET ERROR *** : " << error << " " << errorstring << std::endl;
}

int Socket::Select(const tSockets& in, tSockets& out, int timeout_ms)
{
	struct timeval time;	
	fd_set readfds, writefds, exceptfds;
	time.tv_sec = timeout_ms / 1000; time.tv_usec = timeout_ms % 1000;

	FD_ZERO(&readfds);
	FD_ZERO(&writefds);
	FD_ZERO(&exceptfds);

	for(tSockets::const_iterator i=in.begin(); i != in.end(); i++)
	{
		//Socket* s = *i;
		// sanity check
		if (!(*i)->CheckSocket())
		{
			return -1;
		}

		if ((*i)->mSelectMask & NET_READ_FLAG)		FD_SET((*i)->mSocket->socket,&readfds);
		if ((*i)->mSelectMask & NET_WRITE_FLAG)		FD_SET((*i)->mSocket->socket,&writefds);
		if ((*i)->mSelectMask & NET_EXCEPTION_FLAG)	FD_SET((*i)->mSocket->socket,&exceptfds);
	}

	int rv = 0;

	if ((rv = select(0,&readfds,&writefds,&exceptfds,&time)) == SOCKET_ERROR)
	{
		HandleError();
		return -1;
	}

	if (rv == 0) return 0;


	for(tSockets::const_iterator i=in.begin(); i != in.end(); i++)
	{
		Socket* pSocket = *i;
		int flags = 0;
		if (FD_ISSET(pSocket->mSocket->socket,&readfds))	flags |= NET_READ_FLAG;
		if (FD_ISSET(pSocket->mSocket->socket,&writefds))	flags |= NET_WRITE_FLAG;
		if (FD_ISSET(pSocket->mSocket->socket,&exceptfds))	flags |= NET_EXCEPTION_FLAG;

		if (flags) out.push_back(pSocket);

		pSocket->SetSelectFlags(flags);
	}

	return rv;
}

void Socket::SetSelectFlags(int flags)
{
	mSelectFlags = flags;
}

int Socket::GetSelectFlags() const
{
	return mSelectFlags;
}

void Socket::SetSelectMask(int flags)
{
	mSelectMask = flags;
}

std::string Socket::GetPeerIPAsString() const
{
	std::string ret;
	sockaddr_in addr;
	int len = sizeof(addr);
	getpeername(mSocket->socket, (sockaddr*)&addr, &len);

	char * out = inet_ntoa(addr.sin_addr);
	if (out) ret = out;
	else ret = "Invalid adress";

	return ret;
}
