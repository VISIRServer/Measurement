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

#include "consumer.h"
#include "connection.h"

using namespace Net;

Consumer::Consumer(Connection* connection)
{
	mConnection = connection;
	Reset();
}

void Consumer::Reset()
{
	mCurrent	= 0;
	mLength		= 0;
}

size_t Consumer::Receive(void* buffer, size_t length)
{
	size_t size = length - mCurrent; // read no more than length..
	if (size <=0) return length;

	int rv = mConnection->Receive((unsigned char*)buffer+mCurrent, length-mCurrent);
	if (rv < 0)
	{
		return rv; // error
	}
	else if (rv == 0) // disconnected?
	{
		return 0;
	}
	else
	{
		mCurrent += rv;
		return mCurrent;
	}
}

bool Consumer::Done()
{
	return (mCurrent >= mLength);
}

