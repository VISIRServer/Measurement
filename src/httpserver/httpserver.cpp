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

#include "httpserver.h"

#include <network/multiplexer.h>
#include <network/sockethandler.h>
#include <network/server.h>
#include <network/connection.h>

#include <syslog.h>
#include <config.h>

using namespace std;

fstream		fhttplog;
LogModule	httplog("proto_http", 5);

void InitHTTPLog(Config* pConfig)
{
	static bool sLogInited = false;
	if (sLogInited) return;
	sLogInited = true;

	int logging = pConfig->GetInt("Log", 1);
	int loglevel = pConfig->GetInt("LogLevel", 1);
	string logdir = pConfig->GetString("LogDir", "logs");

	if (logging != 0 && loglevel >= 5) // loglevel 5 enables xml logging
	{
		fhttplog.open((logdir + DirSeparator() + "proto_http.log").c_str(), ios_base::binary | ios_base::out | ios_base::app);
		httplog.AddFileStream(&fhttplog);
		httplog.AddScreenStream(&cout);
		httplog.AddErrorStream(&cerr);

		httplog.Out(1) << "Starting HTTP log session" << endl;
	}
}

class HTTPServerHandler : public Net::SocketHandler
{
public:
	bool	ListenOn(int port);

	virtual void			HandleEvent(int flags);

	virtual Net::Socket*	GetSocket() { return mpServerSocket; }
	virtual bool			IsAlive() { return true; }
	virtual bool			Shutdown() { return true; }

	HTTPServerHandler(HTTPServer* pServer);
	virtual ~HTTPServerHandler();

private:
	Net::Server* mpServerSocket;
	HTTPServer* mpServer;
};


HTTPServerHandler::HTTPServerHandler(HTTPServer* pServer)
{
	mpServerSocket = new Net::Server();
	mpServer = pServer;
}

HTTPServerHandler::~HTTPServerHandler()
{
	mpServerSocket->Disconnect();
	delete mpServerSocket;
}

bool HTTPServerHandler::ListenOn(int port)
{
	if (!mpServerSocket->StartServer(port, 100)) return false;
	return true;
}

void HTTPServerHandler::HandleEvent(int flags)
{
	Net::Connection* pClient = mpServerSocket->CheckIncomming();
	mpServer->AddConnection(pClient);
}

///////////////////

HTTPServer::HTTPServer(Net::Multiplexer* pServer, ServerProtocolService* pSrvProtSrvc, ClientManager* pClientMgr)
{
	mpServerHandler = new HTTPServerHandler(this);
	mpServer	= pServer;
	mpSrvProtSrvc = pSrvProtSrvc;
	mpClientMgr = pClientMgr;
}

HTTPServer::~HTTPServer()
{
	while(!mConnections.empty())
	{
		delete mConnections.back();
		mConnections.pop_back();
	}

	mpServer->RemoveHandler(mpServerHandler);
	delete mpServerHandler;
}

bool HTTPServer::Init(int port, Config* pConfig)
{
	if (!mpServerHandler->ListenOn(port))
	{
		return false;
	}

	mpServer->AddHandler(mpServerHandler);

	InitHTTPLog(pConfig);

	return true;
}

void HTTPServer::AddConnection(Net::Connection* pConnection)
{
	if (pConnection == NULL) return;

	sysout << timestamp << "HTTP connection from: " << pConnection->GetPeerIPAsString() << endl;
	httplog.Log(1) << timestamp << "HTTP connection from: " << pConnection->GetPeerIPAsString() << endl;

	HTTPConnection* pHTTPCon = new HTTPConnection(pConnection, this, mpSrvProtSrvc, mpClientMgr);
	pHTTPCon->Init(); // even if we fail to init, we want the connection to send an error

	mConnections.push_back(pHTTPCon);
	mpServer->AddHandler(pHTTPCon);
}

void HTTPServer::RemoveConnection(HTTPConnection* pHTTPCon)
{
	mConnections.remove(pHTTPCon);
	mpServer->RemoveHandler(pHTTPCon);

	delete pHTTPCon;
}