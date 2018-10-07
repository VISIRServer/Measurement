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

#include <arpa/inet.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <fcntl.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>

#include <syslog.h>

#include <string>

using namespace Net;

#define UNINITIALIZED_SOCKET 0x00
#define SERVER_SOCKET 0x01
#define CLIENT_SOCKET 0x02

#define SOCKET_ERROR -1

#define BACKLOG 10

Socket::Socket(BlockingMode mode)
{
	mMode			= mode;
	mSelectMask		= NET_ALL_FLAGS;
	mSocket			= -1;
}

Socket::Socket(opaque_socket pSocket)
{
	mMode			= Blocking;			// set blocking as default
	mSelectMask		= NET_ALL_FLAGS;	// select on all
	mSocket			= pSocket;	
}

Socket::~Socket()
{
	CloseSocket();
}

bool Socket::Destroy()
{
	return CloseSocket();
}

bool Socket::Init()
{
	return true;
}

bool Socket::StartServer(int port, int backlog)
{
	struct addrinfo hints, *res;
	
	// first, load up address structs with getaddrinfo():
	
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;  // use IPv4 or IPv6, whichever
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;     // fill in my IP for me
	
	char portstr[10];
	snprintf(portstr,10, "%i", port);
	getaddrinfo(NULL, portstr, &hints, &res);
	
	mSocket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if (bind(mSocket, res->ai_addr, res->ai_addrlen) == -1) {
		freeaddrinfo(res);
		return false;
	}
	
	freeaddrinfo(res);
	if (listen(mSocket, backlog) == -1) return false;
	return true;
	
/*	if (!CreateTCPSocket()) return false;
	//mSocket->flags = SERVER_SOCKET;

	struct sockaddr_in my_addr;
	memset(&my_addr,0,sizeof(struct sockaddr_in));
	my_addr.sin_family		= AF_INET;			// host byte order
	my_addr.sin_port		= htons(port);		// short, network byte order
	my_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	// bind socket
	if (bind(mSocket, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == SOCKET_ERROR)
	{
		return false;
	}

	// listen
	if (listen(mSocket->socket, backlog+3) == SOCKET_ERROR)
	{
		return false;
	}

	return true;
*/
}

bool Socket::HasConnecting()
{
	//if (!CheckSocket()) return false;

	struct timeval time;	
	fd_set readfds, writefds, exceptfds;
	time.tv_sec = 0; time.tv_usec = 0;

	FD_ZERO(&readfds);
	FD_ZERO(&writefds);
	FD_ZERO(&exceptfds);

	FD_SET(mSocket,&readfds);

	if (select(0,&readfds,&writefds,&exceptfds,&time) == SOCKET_ERROR)
	{
		// throw exception
		return false;
	}

	if (FD_ISSET(mSocket, &readfds))
	{
		return true;
	}
	return false;
}

opaque_socket Socket::AcceptConnecting()
{
    // it should be safe to accept.. if HasConnecting is called before this one
	//Socket* newSocket = new Socket(this->mMode);
	//realsocket* pSocket = new realsocket();
	
	int newfd = -1;
	
	struct sockaddr_storage their_addr;
	socklen_t addr_size = sizeof their_addr;

	if ((newfd = accept(mSocket,(struct sockaddr *) &their_addr,&addr_size)) == -1)
	{
		HandleError();
		return -1;
	}
	else
	{
		return newfd;
	}
}

bool Socket::Connect(const char* addr, int port)
{
	struct addrinfo hints, *servinfo, *p;
	int sockfd;
	
	// first, load up address structs with getaddrinfo():
	
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;  // use IPv4 or IPv6, whichever
	hints.ai_socktype = SOCK_STREAM;

	char portstr[10];
	snprintf(portstr, 10, "%i", port);
	getaddrinfo(addr, portstr, &hints, &servinfo);
		
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family,
							 p->ai_socktype,
							 p->ai_protocol)) == -1) {
			continue;
		}
		
		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			continue;
		}
		
		break; // if we get here, we must have connected successfully
	}

	freeaddrinfo(servinfo); // all done with this structure
	
	if (p == NULL) {
		// looped off the end of the list with no connection
		//fprintf(stderr, "failed to connect\n");
		//exit(2);
		return false;
	}
	
	mSocket = sockfd;
	return true;
	
	
	
/*	if (!CreateTCPSocket()) return false;
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

	if ((connect(mSocket, (struct sockaddr *)&dest_addr, sizeof(struct sockaddr))) == SOCKET_ERROR)
	{
		if (mMode == NonBlocking)
		{
			if (WSAGetLastError() == WSAEWOULDBLOCK) return true;
		}

		HandleError();
		return false;
	}
	return true;
 */
}

bool Socket::Disconnect()
{
	//if (mSocket->socket == UNINITIALIZED_SOCKET) return true;
	if (mSocket == -1) return true;

	if (close(mSocket) == SOCKET_ERROR)
	{
		HandleError();
		return false;
	}

	//mSocket->socket = UNINITIALIZED_SOCKET;
	mSocket = -1;

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

		FD_SET(mSocket,&readfds);
		
		bool done = false;

		do {
			if (select(mSocket+1,&readfds,&writefds,&exceptfds,&time) == SOCKET_ERROR)
			{
				if (errno == EINTR) {
					// XXX: should decrease timeout
					continue;
				}
				HandleError();
				return -1;
			}
			else done = true;
		} while(!done);
		/*if (select(0,&readfds,&writefds,&exceptfds,&time) == SOCKET_ERROR)
		{
			HandleError();
			return -1;
		}*/

		if (FD_ISSET(mSocket, &readfds))
		{
			int read = recv(mSocket, (char*) buffer, (int)len,0);
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
		int read = recv(mSocket, (char*) buffer, (int)len,0);
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
	return send(mSocket, (const char*) buffer, (int)len,0);
}

bool Socket::SetNonBlocking()
{
	if (!CreateTCPSocket()) return false;

	if (fcntl(mSocket, F_SETFL, O_NONBLOCK) == -1)
	{
		HandleError();
		return false;
	}

	mMode = NonBlocking;
	return true;
}

bool Socket::CheckSocket() const
{
	if (mSocket == -1) return false;
	//if (mSocket->socket == UNINITIALIZED_SOCKET) return false;
	return true;
}

bool Socket::CreateTCPSocket()
{
	/*if (!CheckSocket())
	{
		if ((mSocket->socket = socket(AF_INET,SOCK_STREAM,0)) == INVALID_SOCKET)
		{
			return false;
		}
		return true;
	}

	// reuse previous socket
	 */
	return true;
}

bool Socket::CloseSocket()
{
	return (Disconnect() == 1);
}

void Socket::HandleError()
{
	/*int error = WSAGetLastError();

	char* errorstring = "<unknown>";
	switch(error)
	{
	case WSAEINVAL:			errorstring = "WSAEINVAL";			break;
	case WSAENOTCONN:		errorstring = "WSAENOTCONN";		break;
	case WSAECONNREFUSED:	errorstring = "WSAECONNREFUSED";	break;
	case WSAEWOULDBLOCK:	errorstring = "WSAEWOULDBLOCK";		break;
	case WSAECONNRESET:		errorstring = "WSAECONNRESET";		break;
	case WSAENOTSOCK:		errorstring = "WSAENOTSOCK";		break;
	}

	syserr << "*** SOCKET ERROR *** : " << error << " " << errorstring << std::endl;
	 */
}

int Socket::Select(const tSockets& in, tSockets& out, int timeout_ms)
{
	struct timeval time;	
	fd_set readfds, writefds, exceptfds;
	time.tv_sec = timeout_ms / 1000; time.tv_usec = timeout_ms % 1000;

	FD_ZERO(&readfds);
	FD_ZERO(&writefds);
	FD_ZERO(&exceptfds);
	
	int nfds = 0;

	for(tSockets::const_iterator i=in.begin(); i != in.end(); i++)
	{
		//Socket* s = *i;
		// sanity check
		if (!(*i)->CheckSocket())
		{
			return -1;
		}
				
		if ((*i)->mSocket > nfds) nfds = (*i)->mSocket;

		if ((*i)->mSelectMask & NET_READ_FLAG)		FD_SET((*i)->mSocket,&readfds);
		if ((*i)->mSelectMask & NET_WRITE_FLAG)		FD_SET((*i)->mSocket,&writefds);
		if ((*i)->mSelectMask & NET_EXCEPTION_FLAG)	FD_SET((*i)->mSocket,&exceptfds);
	}

	int rv = 0;

	bool done = false;
	do {
		if ((rv = select(nfds+1,&readfds,&writefds,&exceptfds,&time)) == SOCKET_ERROR)
		{
			if (errno == EINTR) {
				// XXX: should decrease timeout
				continue;
			}
			HandleError();
			return -1;
		} else done = true;
	} while(!done);
	
	/*if ((rv = select(nfds+1,&readfds,&writefds,&exceptfds,&time)) == SOCKET_ERROR)
	{
		HandleError();
		return -1;
	}*/

	if (rv == 0) return 0;


	for(tSockets::const_iterator i=in.begin(); i != in.end(); i++)
	{
		Socket* pSocket = *i;
		int flags = 0;
		if (FD_ISSET(pSocket->mSocket,&readfds))	flags |= NET_READ_FLAG;
		if (FD_ISSET(pSocket->mSocket,&writefds))	flags |= NET_WRITE_FLAG;
		if (FD_ISSET(pSocket->mSocket,&exceptfds))	flags |= NET_EXCEPTION_FLAG;

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
	socklen_t len;
	struct sockaddr_storage addr;
	char ipstr[INET6_ADDRSTRLEN];
	int port;
	
	len = sizeof addr;
	getpeername(mSocket, (struct sockaddr*)&addr, &len);
	
	// deal with both IPv4 and IPv6:
	if (addr.ss_family == AF_INET) {
		struct sockaddr_in *s = (struct sockaddr_in *)&addr;
		port = ntohs(s->sin_port);
		inet_ntop(AF_INET, &s->sin_addr, ipstr, sizeof ipstr);
	} else { // AF_INET6
		struct sockaddr_in6 *s = (struct sockaddr_in6 *)&addr;
		port = ntohs(s->sin6_port);
		inet_ntop(AF_INET6, &s->sin6_addr, ipstr, sizeof ipstr);
	}
	
	return ipstr;
	
	
/*	std::string ret;
	struct sockaddr_storage addr;
	sockaddr_in addr;
	int len = sizeof(addr);
	getpeername(mSocket, (sockaddr*)&addr, &len);

	char * out = inet_ntoa(addr.sin_addr);
	if (out) ret = out;
	else ret = "Invalid adress";

	return ret;
*/
}
