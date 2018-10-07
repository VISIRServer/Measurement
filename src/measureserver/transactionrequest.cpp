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

#include "transactionrequest.h"
#include "requestqueue.h"

#include "clientmanager.h"
#include "client.h"
#include "session.h"
#include "transactioncontrol.h"

#include <protocol/protocol.h>
#include <protocol/basic_types.h>

#include <basic_exception.h>
#include <syslog.h>

TransactionRequest::TransactionRequest(
	RequestQueue* pQueue
	, Client* pOwner
	, protocol::Transaction* pTransaction
	, protocol::TransactionHandler* pTransactionHandler
	, double timeout
	) : Request(pQueue, pOwner)
	, mpTransaction(pTransaction)
	, mpHandler(pTransactionHandler)
	, mTimeout(timeout)
{
	mpProxyIssuer = NULL;

	mHasBeenSent	= false;
}

TransactionRequest::~TransactionRequest()
{
	if (mpTransaction)
	{
		delete mpTransaction;
		mpTransaction = NULL;
	}
	
	if (mpProxyIssuer)
	{
		delete mpProxyIssuer;
		mpProxyIssuer = NULL;
	}
}

void TransactionRequest::Send()
{
	LogLevel(syslog, 5) << timestamp << "TransactionRequest::Send" << std::endl;
	mTimer.restart();

	mHasBeenSent = true;

	if (mpOwner)
	{
		try
		{
			mpHandler->Perform(mpTransaction, this);
		}
		catch(ValidationException e)
		{
			Error(e.what(), protocol::Notification);
			RequestDone();
		}
		catch(BasicException e)
		{
			Error(e.what(), protocol::Fatal);
			RequestDone();
		}
	}
	else
	{
		syserr << timestamp << "TransactionRequest::Send Ownerless request" << std::endl;
		RequestDone();
	}
}

bool TransactionRequest::BuildRequest()
{
	return true;
}

// This should NOT delete the instance..
void TransactionRequest::Cancel()
{
	if (mHasBeenSent)
	{
		syserr << timestamp << "Canceling transaction in progress" << std::endl;
		mpHandler->Cancel(mpTransaction, this);
	}
	mpOwner = NULL;
}

bool TransactionRequest::HasTimedOut()
{
	if (mTimer.elapsed() > mTimeout)
	{
		Error("Timeout", protocol::Notification);
		Cancel();
		mpQueue->RequestDone(this); // this will destory us
		return true;
	}

	return false;	
}

InstrumentBlock* TransactionRequest::GetInstrumentBlock()
{
	if (!mpOwner) return NULL;
	if (!mpOwner->GetSession()) throw BasicException("TransactionRequest::GetInstrumentBlock: Missing session");
	return mpOwner->GetSession()->GetBlock();
}

void TransactionRequest::RequestDone()
{
	if (mpOwner) {
		if (mpOwner->GetSession()) {
			LogLevel(timerlog,4) << timestamp << "TransactionRequest::RequestDone (after " << mTimer.elapsed() << ") client_id=" << mpOwner->ClientID() << " session_id=" << mpOwner->GetSession()->GetNumber() << std::endl;
		} else {
			LogLevel(timerlog,4) << timestamp << "TransactionRequest::RequestDone (after " << mTimer.elapsed() << ") client_id=" << mpOwner->ClientID() << std::endl;
		}
	}

	if (mpTransaction)
	{
		delete mpTransaction;
		mpTransaction = NULL;
	}

	mpQueue->RequestDone(this); // this will delete us
}

void TransactionRequest::Error(std::string msg, protocol::TransactionErrorType type)
{
	syserr << timestamp << "TransactionRequest error: " << msg << std::endl;
	if (mpOwner)
	{
		Session* pSession = mpOwner->GetSession();
		if (pSession) pSession->SetActiveTransaction(NULL); // no active transaction

		mpTransaction->GetIssuer()->TransactionError(mpTransaction, msg.c_str(), type);
	}
}

void TransactionRequest::TransactionDone()
{
	if (mpOwner)
	{
		Session* pSession = mpOwner->GetSession();
		if (pSession) pSession->SetActiveTransaction(NULL); // no active transaction
		mpTransaction->GetIssuer()->TransactionComplete(mpTransaction);
	}
	RequestDone(); // this will delete us
}

void TransactionRequest::TransactionError(const char* msg, protocol::TransactionErrorType type)
{
	Error(msg, type);
	if (mpTransaction)
	{
		delete mpTransaction;
		mpTransaction = NULL;
	}

	mpQueue->RequestDone(this); // this will delete us
}

bool TransactionRequest::MustBindToSession()
{
	// if the transaction contains measurements, they should be 
	typedef protocol::Transaction::tRequests tRequests;
	const tRequests& requests = mpTransaction->GetRequests();
	for(tRequests::const_iterator it = requests.begin(); it != requests.end(); it++)
	{
		if ((*it)->GetType() == protocol::RequestType::Measurement)
		{
			return true;
		}
	}

	return false;
}

std::string TransactionRequest::GetSessionKey()
{
	typedef protocol::Transaction::tRequests tRequests;
	const tRequests& requests = mpTransaction->GetRequests();
	for(tRequests::const_iterator it = requests.begin(); it != requests.end(); it++)
	{
		if ((*it)->GetType() == protocol::RequestType::Measurement)
		{
			protocol::MeasureRequest* pMeasure = (protocol::MeasureRequest*) *it;
			return pMeasure->GetSessionKey();
		}
	}

	return "";
}


void TransactionRequest::ClientGone()
{
	Cancel();
	mpQueue->RemoveRequest(this); // this will delete us
}