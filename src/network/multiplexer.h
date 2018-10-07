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

#ifndef __NETWORK_MULTIPLEXER_H__
#define __NETWORK_MULTIPLEXER_H__

#include "sockethandler.h"

#include <map>
#include <list>

namespace Net {

/// Network multiplexer / server utility.
/// Manages sockethandlers for processing (mapping to select).
/// Notice: you must keep the list of selectable sockets intact in all cases..
/// No closed or broken sockets should be in the list..

class Multiplexer
{
public:
	bool	WaitForEvent(int timeout_ms);

	bool	HouseKeeping();

	void	AddHandler(Net::SocketHandler* handler);
	void	RemoveHandler(Net::SocketHandler* handler);

			Multiplexer();
	virtual ~Multiplexer();
private:
	typedef std::map< Net::Socket*, Net::SocketHandler* > tHandlers;
	typedef std::list< Net::Socket* > tSockets;

	tHandlers		mHandlers;
	tSockets		mSockets;
};

}

#endif
