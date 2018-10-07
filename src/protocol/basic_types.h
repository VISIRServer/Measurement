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
 * Copyright (c) 2008-2009 Johan Zackrisson
 * All Rights Reserved.
 */

#pragma once
#ifndef __PROTOCOL_BASIC_TYPES_H__
#define __PROTOCOL_BASIC_TYPES_H__

#include <instruments/instrument.h>

#include "protocol.h"

#include <string>

namespace protocol
{

class HeartbeatRequest : public Request
{
public:
	HeartbeatRequest() { mType = RequestType::Heartbeat; }
	virtual ~HeartbeatRequest() {}
};

class InstrumentCommand
{
public:
	virtual Instrument::InstrumentType InstrumentType() = 0;
	virtual void ApplySettings(InstrumentBlock* pInstrument) = 0;

	virtual ~InstrumentCommand() {}
};

class MeasureRequest : public Request
{
public:
	typedef std::list< InstrumentCommand* > tCmdList;

	MeasureRequest(std::string sessionKey)
	{
		mType = RequestType::Measurement;
		mSessionKey	= sessionKey;
	}
	virtual ~MeasureRequest()
	{
		while(!mCmdList.empty())
		{
			delete mCmdList.back();
			mCmdList.pop_back();
		}
	}

	// takes ownership
	void			AddInstrumentCommand(InstrumentCommand* pCmd) { mCmdList.push_back(pCmd); }
	const tCmdList&	GetCmdList() { return mCmdList; }
	const std::string&	GetSessionKey() const { return mSessionKey; }
private:
	tCmdList mCmdList;
	std::string	mSessionKey;
};

class DomainPolicyRequest : public Request
{
public:
	DomainPolicyRequest() { mType = RequestType::DomainPolicy; }
	virtual ~DomainPolicyRequest() {}
};

} // end of namespace protocol

#endif
