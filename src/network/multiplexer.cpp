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
 * Copyright (c) 2009 Johan Zackrisson
 * All Rights Reserved.
 */

#include "multiplexer.h"
#include "server.h"
#include <syslog.h>

using namespace Net;

Multiplexer::Multiplexer()
{
}

Multiplexer::~Multiplexer()
{
}


bool Multiplexer::WaitForEvent(int timeout_ms)
{
	tSockets out;
	
	if (mSockets.empty()) return true; // this will lead to a spin...

	if (Net::Socket::Select(mSockets,out,timeout_ms) < 0)
	{
		syserr << timestamp << "Select failed (" << (unsigned int)mSockets.size() << ")" << std::endl;
		return false;
	}

	// this doesn't guard against handlers going away in the event handling
	for(tSockets::const_iterator i=out.begin(); i!= out.end(); i++)
	{
		// check if socket is still alive before calling handle event
		tHandlers::const_iterator finder = mHandlers.find(*i);
		if (finder != mHandlers.end())
		{
			mHandlers[*i]->HandleEvent((*i)->GetSelectFlags());
		}
		else
		{
			syserr << timestamp << "Trying to handle event on dead eventhandler" << std::endl;
		}
	}
	return true;
}

bool Multiplexer::HouseKeeping()
{
	// handlers can disapear, so we make a copy for conveniance sake
	tHandlers aCopy = mHandlers;
	for(tHandlers::iterator it = aCopy.begin(); it != aCopy.end(); it++)
	{
		if(!it->second->IsAlive())
		{
			syserr << timestamp << "Dead handler: kicking" << std::endl;
			it->second->Shutdown();
		}
	}

	return true;
}

void Multiplexer::AddHandler(Net::SocketHandler* handler)
{
	mSockets.push_back(handler->GetSocket());
	if (mHandlers.find(handler->GetSocket()) != mHandlers.end())
	{
		std::cerr << "Handler is already in map!" << std::endl;
	}
	mHandlers[handler->GetSocket()] = handler;
}

void Multiplexer::RemoveHandler(Net::SocketHandler* handler)
{
	mSockets.remove(handler->GetSocket());
	mHandlers.erase(handler->GetSocket());
}
