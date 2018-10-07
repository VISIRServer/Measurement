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

#include "iobuffer.h"
#include "connection.h"

#include <iostream>
#include <syslog.h>

#include <vector>

using namespace Net;

struct Net::IOBuffer_internal
{
	typedef std::vector<char> tByteBuffer;
	tByteBuffer mBuffer;
};

SendBuffer::SendBuffer()
{
	mWrap = new IOBuffer_internal();
	mOffset = 0;
}

SendBuffer::~SendBuffer()
{
	delete mWrap;
}

bool SendBuffer::Fill(void* pData, size_t size)
{
	mWrap->mBuffer.insert(mWrap->mBuffer.end(), (char*)pData, (char*)pData + size);
	return true;
}

bool SendBuffer::Clear()
{
	mOffset = 0;
	mWrap->mBuffer.clear();
	return true;
}

bool SendBuffer::Empty()
{
	return mWrap->mBuffer.empty();
}

int SendBuffer::Send(Connection* pConnection)
{
	size_t len = mWrap->mBuffer.size() - mOffset;
	int rv = pConnection->Send(&mWrap->mBuffer[0], len);
	if (rv <= 0) return -1;

	mOffset += rv;
	if (mWrap->mBuffer.size() == mOffset) return 1;
	return 0;
}


ReceiveBuffer::ReceiveBuffer()
{
	mWrap = new IOBuffer_internal();
}

ReceiveBuffer::~ReceiveBuffer()
{
	delete mWrap;
}

int ReceiveBuffer::Receive(Connection* pConnection, size_t length)
{
	size_t remaining = length - mWrap->mBuffer.size();
	if (remaining == 0) return 1;

	std::vector<char> tmpbuffer(remaining);
	
	int rv = pConnection->Receive(&tmpbuffer[0], remaining);
	if (rv <= 0) return -1;
	else
	{
		mWrap->mBuffer.insert(mWrap->mBuffer.end(), &tmpbuffer[0], &tmpbuffer[0] + rv);
	}

	return (mWrap->mBuffer.size() == length) ? 1 : 0;
}

void* ReceiveBuffer::GetBuffer() const
{
	return &mWrap->mBuffer[0];
}

size_t ReceiveBuffer::GetSize() const
{
	return mWrap->mBuffer.size();
}

void ReceiveBuffer::Clear()
{
	mWrap->mBuffer.clear();
}

void ReceiveBuffer::EraseFront(size_t length)
{
	//std::vector<char>& test = mWrap->mBuffer;
	//size_t before = test.size();
	mWrap->mBuffer.erase(mWrap->mBuffer.begin(), mWrap->mBuffer.begin() + length);

	//size_t after = test.size();
}