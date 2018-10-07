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

#include "scgiserver.h"

#include <network/multiplexer.h>
#include <network/sockethandler.h>
#include <network/server.h>
#include <network/connection.h>

#include <syslog.h>
#include <config.h>

using namespace std;

fstream		fscgilog;
LogModule	scgilog("proto_scgi", 5);

void InitSCGILog(Config* pConfig)
{
	static bool sLogInited = false;
	if (sLogInited) return;
	sLogInited = true;

	int logging = pConfig->GetInt("Log", 1);
	int loglevel = pConfig->GetInt("LogLevel", 1);
	string logdir = pConfig->GetString("LogDir", "logs");

	if (logging != 0 && loglevel >= 5) // loglevel 5 enables xml logging
	{
		fscgilog.open((logdir + DirSeparator() + "proto_scgi.log").c_str(), ios_base::binary | ios_base::out | ios_base::app);
		scgilog.AddFileStream(&fscgilog);
		scgilog.AddScreenStream(&cout);
		scgilog.AddErrorStream(&cerr);

		scgilog.Out(1) << "Starting SCGI log session" << endl;
	}
}

class SCGIServerHandler : public Net::SocketHandler
{
public:
	bool	ListenOn(int port);

	virtual void			HandleEvent(int flags);

	virtual Net::Socket*	GetSocket() { return mpServerSocket; }
	virtual bool			IsAlive() { return true; }
	virtual bool			Shutdown() { return true; }

	SCGIServerHandler(SCGIServer* pServer);
	virtual ~SCGIServerHandler();

private:
	Net::Server* mpServerSocket;
	SCGIServer* mpServer;
};


SCGIServerHandler::SCGIServerHandler(SCGIServer* pServer)
{
	mpServerSocket = new Net::Server();
	mpServer = pServer;
}

SCGIServerHandler::~SCGIServerHandler()
{
	mpServerSocket->Disconnect();
	delete mpServerSocket;
}

bool SCGIServerHandler::ListenOn(int port)
{
	if (!mpServerSocket->StartServer(port, 100)) return false;
	return true;
}

void SCGIServerHandler::HandleEvent(int flags)
{
	Net::Connection* pClient = mpServerSocket->CheckIncomming();
	mpServer->AddConnection(pClient);
}

///////////////////

SCGIServer::SCGIServer(Net::Multiplexer* pServer, ServerProtocolService* pSrvProtSrvc, ClientManager* pClientMgr)
{
	mpServerHandler = new SCGIServerHandler(this);
	mpServer	= pServer;
	mpSrvProtSrvc = pSrvProtSrvc;
	mpClientMgr = pClientMgr;
}

SCGIServer::~SCGIServer()
{
	while(!mConnections.empty())
	{
		delete mConnections.back();
		mConnections.pop_back();
	}

	mpServer->RemoveHandler(mpServerHandler);
	delete mpServerHandler;
}

bool SCGIServer::Init(int port, Config* pConfig)
{
	if (!mpServerHandler->ListenOn(port))
	{
		return false;
	}

	mpServer->AddHandler(mpServerHandler);

	InitSCGILog(pConfig);

	return true;
}

void SCGIServer::AddConnection(Net::Connection* pConnection)
{
	if (pConnection == NULL) return;

	sysout << timestamp << "SCGI connection from: " << pConnection->GetPeerIPAsString() << endl;
	scgilog.Log(1) << timestamp << "SCGI connection from: " << pConnection->GetPeerIPAsString() << endl;

	SCGIConnection* pSCGICon = new SCGIConnection(pConnection, this, mpSrvProtSrvc, mpClientMgr);
	pSCGICon->Init(); // even if we fail to init, we want the connection to send an error

	mConnections.push_back(pSCGICon);
	mpServer->AddHandler(pSCGICon);
}

void SCGIServer::RemoveConnection(SCGIConnection* pSCGICon)
{
	mConnections.remove(pSCGICon);
	mpServer->RemoveHandler(pSCGICon);

	delete pSCGICon;
}