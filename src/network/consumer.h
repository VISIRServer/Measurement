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

#pragma once
#ifndef __NETWORK_CONSUMER_H__
#define __NETWORK_CONSUMER_H__

#ifndef _WIN32
#include <unistd.h>
#endif

namespace Net
{

class Connection;

/// Network buffer utility
/// Handles prebuffering until whole packets arive (the length you request).
class Consumer
{
public:
	Consumer(Connection* connection);

	/// Begin new read session
	void	Reset();

	/// Receives length data from a connection
	/// Receives at most _length_ bytes, and may be called many times to read fragmented or splitted packets.
	/// \param buffer buffer to place read data
	/// \param length length of data to receive
	size_t	Receive(void* buffer, size_t length);

	/// Returns true when _length_ data is read. See Receive.
	bool	Done();
private:
	Connection*		mConnection;
	size_t			mCurrent;
	size_t			mLength;
};

} // end namespace

#endif
