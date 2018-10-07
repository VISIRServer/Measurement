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

#include "xmlserver.h"

#include <network/multiplexer.h>
#include <network/sockethandler.h>
#include <network/server.h>
#include <network/connection.h>

#include <syslog.h>
#include <config.h>

using namespace std;

fstream		fxmllog;
LogModule	xmllog("proto_xml", 5);

void InitXMLLog(Config* pConfig)
{
	static bool sLogInited = false;
	if (sLogInited) return;
	sLogInited = true;

	int logging = pConfig->GetInt("Log", 1);
	int loglevel = pConfig->GetInt("LogLevel", 1);
	string logdir = pConfig->GetString("LogDir", "logs");

	if (logging != 0 && loglevel >= 5) // loglevel 5 enables xml logging
	{
		fxmllog.open((logdir + DirSeparator() + "proto_xml.log").c_str(), ios_base::binary | ios_base::out | ios_base::app);
		xmllog.AddFileStream(&fxmllog);
		xmllog.AddScreenStream(&cout);
		xmllog.AddErrorStream(&cerr);

		xmllog.Out(1) << "Starting XML log session" << endl;
	}
}

class XMLServerHandler : public Net::SocketHandler
{
public:
	bool	ListenOn(int port);

	virtual void			HandleEvent(int flags);

	virtual Net::Socket*	GetSocket() { return mpServerSocket; }
	virtual bool			IsAlive() { return true; }
	virtual bool			Shutdown() { return true; }

	XMLServerHandler(XMLServer* pServer);
	virtual ~XMLServerHandler();

private:
	Net::Server* mpServerSocket;
	XMLServer* mpServer;
};

XMLServerHandler::XMLServerHandler(XMLServer* pServer)
{
	mpServerSocket = new Net::Server();
	mpServer = pServer;
}

XMLServerHandler::~XMLServerHandler()
{
	mpServerSocket->Disconnect();
	delete mpServerSocket;
}

bool XMLServerHandler::ListenOn(int port)
{
	if (!mpServerSocket->StartServer(port, 100)) return false;
	return true;
}

void XMLServerHandler::HandleEvent(int flags)
{
	Net::Connection* pClient = mpServerSocket->CheckIncomming();
	mpServer->AddConnection(pClient);
}

///////////////////

XMLServer::XMLServer(Net::Multiplexer* pServer, ServerProtocolService* pSrvProtSrvc, ClientManager* pClientMgr)
{
	mpServerHandler = new XMLServerHandler(this);
	mpServer	= pServer;
	mpSrvProtSrvc = pSrvProtSrvc;
	mpClientMgr = pClientMgr;
}

XMLServer::~XMLServer()
{
	while(!mConnections.empty())
	{
		delete mConnections.back();
		mConnections.pop_back();
	}

	mpServer->RemoveHandler(mpServerHandler);
	delete mpServerHandler;
}

bool XMLServer::Init(int port, Config* pConfig)
{
	if (!mpServerHandler->ListenOn(port))
	{
		return false;
	}

	mpServer->AddHandler(mpServerHandler);

	InitXMLLog(pConfig);

	return true;
}

void XMLServer::AddConnection(Net::Connection* pConnection)
{
	if (pConnection == NULL) return;

	sysout << timestamp << "XMLServer connection from: " << pConnection->GetPeerIPAsString() << endl;

	XMLConnection* pXMLCon = new XMLConnection(pConnection, this, mpSrvProtSrvc, mpClientMgr, 30.0, 600.0);
	if (pXMLCon->Init())
	{
		mConnections.push_back(pXMLCon);
		mpServer->AddHandler(pXMLCon);
	}
	else
	{
		delete pXMLCon;
	}
}

void XMLServer::RemoveConnection(XMLConnection* pHTTPCon)
{
	mConnections.remove(pHTTPCon);
	mpServer->RemoveHandler(pHTTPCon);

	delete pHTTPCon;
}