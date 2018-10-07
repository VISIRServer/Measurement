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
 * Copyright (c) 2008-2009 Johan Zackrisson
 * All Rights Reserved.
 */

#ifndef __EQ_CONNECTION_H__
#define __EQ_CONNECTION_H__

#include <network/iobuffer.h>
#include <network/sockethandler.h>

#include <util/timer.h>

#include <string>

class Serializer;

namespace Net {
	class Connection;
	class Multiplexer;
}

namespace EqSrv
{

class EqConnectionCallback
{
public:
	virtual void OnResponse(Serializer& in) = 0;
	virtual void OnError(std::string msg) = 0;
	virtual ~EqConnectionCallback() {}
};

class EqConnection : public Net::SocketHandler
{
public:
	virtual void HandleEvent(int flags);

	virtual Net::Socket*	GetSocket();

	virtual bool	IsAlive();
	virtual bool	Shutdown();

	bool SendCommand(Serializer& out, EqConnectionCallback* callback);
	void CancelCommand();

	EqConnection(Net::Multiplexer* pServer, std::string host, int port);
	virtual ~EqConnection();
private:
	void Cleanup();
	bool MakeConnection();

	void HandleResponse(Serializer& in);

	enum eReadState
	{
		eReadHeader,
		eReadPacket,
	} mReadState;

	size_t mPacketSize;

	Net::Connection*		mConnection;

	Net::SendBuffer		mSendBuffer;
	Net::ReceiveBuffer	mReceiveBuffer;

	bool mHandlerAdded;
	EqConnectionCallback* mpCurrentCallback;
	timer				mStopClock;

	Net::Multiplexer*	mpServer;
	std::string			mHost;
	int					mPort;
};

} // end of namespace

#endif
