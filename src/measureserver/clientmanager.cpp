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
 * Copyright (c) 2007-2009 Johan Zackrisson
 * All Rights Reserved.
 */

#include "clientmanager.h"
#include "client.h"

#include <network/connection.h>

#include <syslog.h>
#include <algorithm>

ClientManager::ClientManager(size_t maxClients)
{
	mMaxClients = maxClients;
	mLastSessionId = 0;
}

ClientManager::~ClientManager()
{
	// if there is clients left in the clientlist.. report them as leaking
	if (!mClients.empty()) syserr << "Client leakage of " << (unsigned int)mClients.size() << " clients.." << std::endl;
	mClients.clear();
}

Client* ClientManager::AddClient(Net::Connection* pConnection)
{
	mLastSessionId++;
	//sysout << timestamp << "(" << mLastSessionId << ") Client connection from: " << pConnection->GetPeerIPAsString() << endl;

	if (mClients.size() >= mMaxClients)
	{
		sysout << timestamp << "Max clients reached, rejecting.. (max " << (unsigned int)mMaxClients << ")" << std::endl;
		return NULL;
	}

	Client* pClient = new Client();
	mClients.push_back(pClient);
	return pClient;
}

void ClientManager::RemoveClient(Client* client)
{
	mClients.remove(client);
}

bool ClientManager::CheckClient(Client* client) const
{
	// lists have no find.. so we have to use the global algorithm instead
	tClients:: const_iterator finder = std::find(mClients.begin(), mClients.end(), client);
	if (finder != mClients.end()) return true;
	else return false;
}

size_t ClientManager::NumCurClients() const
{
	return mClients.size();
}

size_t ClientManager::TotalNumClients() const
{
	return mLastSessionId;
}

const ClientManager::tClients& ClientManager::GetClients() const
{
	return mClients;
}
