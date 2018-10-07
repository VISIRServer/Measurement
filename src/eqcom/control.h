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

#ifndef __EQ_CONTROL_H__
#define __EQ_CONTROL_H__

class InstrumentBlock;

#include "connection.h"

#include <instruments/listparser.h>
#include <instruments/netlist2.h>
#include <protocol/protocol.h>

class Config;
class ModuleServices;

namespace Net {
	class Multiplexer;
}

namespace EqSrv
{

class EqMeasurement;
class EqTransactionHandler;
class EqConnection;

class RequestCallback
{
public:
	virtual void RequestDone() = 0;
	virtual void Error(std::string msg, protocol::TransactionErrorType type) = 0;
	virtual ~RequestCallback() {}
};

// todo: rename
class EquipmentServerControl : public EqConnectionCallback
{
public:
	bool	Init();
	bool	IsInitDone();
	bool	HasInitFailed();
	bool	Tick();

	// send a request according to a pre-made setup order
	bool	SendBakedRequest(InstrumentBlock* pBlock, RequestCallback* pCallback);
	bool	CancelRequest(RequestCallback* pCallback);

	//const NetList2&		GetServerNetlist() { return mServerNetlist; }

	EquipmentServerControl(Net::Multiplexer* pServer, Config* pConfig, ModuleServices* pService);
	virtual ~EquipmentServerControl();
private:
	bool RequestServerInfo();
	bool TryRequestServerInfo();
	virtual void OnResponse(Serializer& in);
	virtual void OnError(std::string msg);

	bool			mInitDone;
	bool			mFailed;
	EqConnection*	mpEqConnection;
	NetList2		mServerNetlist; // change to pointer?
	EqMeasurement*	mpMeasurement;
	RequestCallback* mpCookie;
	bool			mMatrixDisabled;
	EqTransactionHandler*	mpTransactionHandler;
	Net::Multiplexer*	mpServer;
	Config*				mpConfig;
	ModuleServices*		mpService;

	int		mRetries;
	int		mTimeout;
};

} // end of namespace

#endif
