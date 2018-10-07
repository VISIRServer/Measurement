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

#ifndef __HTTP_SERVER_H__
#define __HTTP_SERVER_H__

#include "httpconnection.h"
#include <util/logmodule.h>

#include <list>

namespace Net
{
	class Connection;
	class Multiplexer;
}

class HTTPServerHandler;
class Config;

extern LogModule	httplog;

class HTTPServer
{
public:
	bool Init(int port, Config* pConfig);

	void AddConnection(Net::Connection* pConnection);
	void RemoveConnection(HTTPConnection* pHTTPCon);

	HTTPServer(Net::Multiplexer* pServer, ServerProtocolService* pSrvProtSrvc, ClientManager* pClientMgr);
	virtual ~HTTPServer();
private:
	HTTPServerHandler* mpServerHandler;

	typedef std::list<HTTPConnection*> tConnections;
	tConnections			mConnections;
	Net::Multiplexer*		mpServer;
	ServerProtocolService*	mpSrvProtSrvc;
	ClientManager*			mpClientMgr;
};

#endif
