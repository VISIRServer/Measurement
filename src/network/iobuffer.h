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

#pragma once
#ifndef __NETWORK_IO_BUFFER_H__
#define __NETWORK_IO_BUFFER_H__

#ifndef _WIN32
#include <unistd.h>
#endif

namespace Net
{

class Connection;

struct IOBuffer_internal;

class SendBuffer
{
public:
	bool Fill(void* pData, size_t size);
	bool Clear();

	bool Empty();

	int Send(Connection* pConnection);

	SendBuffer();
	virtual ~SendBuffer();
private:
	IOBuffer_internal* mWrap;
	size_t	mOffset;
};

/////////////////

class ReceiveBuffer
{
public:
	int Receive(Connection* pConnection, size_t length);

	void* GetBuffer() const;
	size_t GetSize() const;

	void Clear();
	void EraseFront(size_t length);

	ReceiveBuffer();
	virtual ~ReceiveBuffer();
private:
	IOBuffer_internal* mWrap;
};

} // end namespace

#endif
