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
 * Copyright (c) 2008 Johan Zackrisson
 * All Rights Reserved.
 */

#ifndef __SCGI_CONNECTION_H__
#define __SCGI_CONNECTION_H__

#include <network/sockethandler.h>
#include <network/iobuffer.h>
#include <util/timer.h>

#include <measureserver/client.h>

#include <protocol/protocol.h>

#include <string>

class SCGIServer;
class SCGIRequest;
class RequestQueue;
class ClientManager;

class ServerProtocolService;
class TransactionRequest;

class SCGIConnection : public Net::SocketHandler, public protocol::TransactionIssuer, public IClientEventListener
{
public:
	virtual void	HandleEvent(int flags);
	virtual Net::Socket*	GetSocket();
	virtual bool	IsAlive();
	virtual bool	Shutdown();

	virtual void TransactionComplete(protocol::Transaction* pTransaction);
	virtual void TransactionError(protocol::Transaction* pTransaction, const char* msg, protocol::TransactionErrorType type);

	virtual void SessionDestroyed();

	bool Init();

	SCGIConnection(Net::Connection* pConnection, SCGIServer* pServer, ServerProtocolService* pSrvProtSrvc, ClientManager* pClientMgr);
	virtual ~SCGIConnection();
private:
	void SendResponse(const char* data, size_t length);
	void SendError(std::string msg);

	bool ParseSCGIRequest(void* buffer, size_t datalength);
	void SCGIError(std::string error, int errornr = 400);

	bool ParseSCGILine(char* buffer, size_t length);

	Net::Connection*	mpConnection;
	SCGIServer*			mpServer;
	Client*				mpClient;
	ServerProtocolService* mpSrvProtSrvc;
	ClientManager*		mpClientMgr;
	TransactionRequest*		mpCurrentRequest;

	timer				mLifeTimer;

	Net::SendBuffer		mSendBuffer;
	Net::ReceiveBuffer	mReceiveBuffer;

	SCGIRequest*	mpRequest;
	size_t			mRequestSize;

	bool			mKeepAlive;
	int				mRequestID;

	enum eState
	{
		eRequest,
		eClosing
	} mState;

	//
	bool HandlePacket(const char* pData, size_t length);
};

#endif
