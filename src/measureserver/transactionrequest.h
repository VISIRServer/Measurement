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
#ifndef __TRANSACTION_REQUEST_H__
#define __TRANSACTION_REQUEST_H__

#include "request.h"

#include <protocol/protocol.h>
#include <timer.h>

class Session;
class TransactionControl;

/// Represents a request to the measurement server
// xxx: merge this with request, no other request types will ever be needed..
class TransactionRequest : public Request , public protocol::TransactionCallback
{
public:
	virtual void	Send();
	virtual bool	BuildRequest();
	virtual bool	HasTimedOut();
	virtual void	Cancel();

	virtual void	RequestDone();

	virtual InstrumentBlock* GetInstrumentBlock();
	virtual void	TransactionDone();
	virtual void	TransactionError(const char* msg, protocol::TransactionErrorType type);

	// return true on measurements that can be bound to a session
	bool		MustBindToSession();

	// bind the transaction to the session
	std::string GetSessionKey();

	void		ClientGone();

	TransactionRequest(
		RequestQueue* pQueue
		, Client* owner
		, protocol::Transaction* pTransaction
		, protocol::TransactionHandler* pTransactionHandler
		, double timeout
		);
	virtual ~TransactionRequest();
private:
	void	Error(std::string msg, protocol::TransactionErrorType type);
	protocol::Transaction* mpTransaction;
	protocol::TransactionHandler* mpHandler;
	timer	mTimer;
	double	mTimeout;
	bool	mHasBeenSent;

	protocol::TransactionIssuer* mpProxyIssuer;
};

#endif
