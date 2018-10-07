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

#ifndef __NETWORK_SOCKET_H__
#define __NETWORK_SOCKET_H__

#include <list>
#include <string>

namespace Net
{

#define NET_READ_FLAG		0x01
#define NET_WRITE_FLAG		0x02
#define NET_EXCEPTION_FLAG	0x04

#define NET_ALL_FLAGS		0x07

#if _WIN32
// wrapper for windows sockets.. no need to expose junk..
typedef struct realsocket* opaque_socket;
#else
typedef int opaque_socket;
#endif

/// Handles all lowlevel socket functions
class Socket
{
public:
	typedef std::list<Socket*> tSockets;

	enum BlockingMode
	{
		Blocking		= 1,
		NonBlocking		= 2,
		Asyncronous		= 3,
	};

	static bool	Init();

	// server methods, use only in servermode
	bool		StartServer(int port, int backlog);
	bool		HasConnecting();
	opaque_socket	AcceptConnecting();

	// client methods, use only in clientmode
	bool		Connect(const char* addr, int port);
	bool		Destroy();
	bool		Disconnect();
	bool		IsConnected() const;

	//		shared methods
	//		notice: plattform does not define ssize_t..
	signed int	Receive(void* buffer,		size_t len);
	signed int	Send(const void* buffer,	size_t len);

	int			GetSelectFlags() const;
	void		SetSelectMask(int flags);

	bool		SetNonBlocking();

	///			Select on many sockets
	///			\param in list of sockets to check of network activity
	///			\param out reference to a list which will receive sockets where something has happened
	///			\param timeout_ms timeout in milliseconds
	static int	Select(const tSockets& in, tSockets& out, int timeout_ms);

	//
	std::string		GetPeerIPAsString() const;

	Socket(BlockingMode mode);
	Socket(opaque_socket pSocket);
	virtual ~Socket();
private:
	void		SetSelectFlags(int flags);

	bool		CheckSocket() const;
	bool		CreateTCPSocket();
	bool		CloseSocket();

	static void	HandleError();

	opaque_socket	mSocket;		// hide winsock api from rest of world
	BlockingMode	mMode;
	int				mSelectFlags;
	int				mSelectMask;
};

} // end of namespace

#endif
