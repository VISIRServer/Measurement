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

#include "protocolservice.h"
#include "requestqueue.h"
#include "transactionrequest.h"
#include "client.h"
#include "session.h"
#include "service.h"
#include "transactioncontrol.h"

#include <basic_exception.h>
#include <syslog.h>

ServerProtocolService::ServerProtocolService(
	RequestQueue* pRequestQueue
	, TransactionControl* pTransactionControl
	, Service* pService
	, SessionRegistry* pSessionRegistry
	, double timeout
	)
	: mpRequestQueue(pRequestQueue)
	, mpTransactionControl(pTransactionControl)
	, mpService(pService)
	, mpSessionRegistry(pSessionRegistry)
	, mTimeout(timeout)
{
}

ServerProtocolService::~ServerProtocolService()
{
}


TransactionRequest* ServerProtocolService::ProcessTransaction(protocol::Transaction* pTransaction, Client* pClient)
{
	pTransaction->SetOwner(pClient);

	protocol::TransactionHandler* pHandler = mpTransactionControl->GetReceiverFor(pTransaction);
	if (!pHandler)
	{
		pTransaction->GetIssuer()->TransactionError(pTransaction, "Unable to find handler for transaction", protocol::Fatal);
		return NULL;
	}

	TransactionRequest* pRequest = new TransactionRequest(mpRequestQueue, pClient, pTransaction, pHandler, mTimeout);

	Session* pSession = NULL;

	// we can get a login request on a already authenticated session, hand back the same key

	if (pRequest->MustBindToSession())
	{
		pSession = mpSessionRegistry->BindToSession(pClient, pRequest->GetSessionKey());
		if (!pSession)
		{
			syserr << "Failed session key: " << pRequest->GetSessionKey() << std::endl;
			pTransaction->GetIssuer()->TransactionError(pTransaction, "Your session key is not valid", protocol::Fatal);
			delete pRequest;
			return NULL;
		}

		// if the session already has a active transaction, make it go away
		if (pSession->HasActiveTransaction())
		{
			pTransaction->GetIssuer()->TransactionError(pTransaction, "One transaction at the time", protocol::Fatal);
			delete pRequest;
			return NULL;
		}
	}
	
	try
	{
		if (pSession) LogLevel(timerlog,4) << timestamp << "ServerProtocolService::ProcessTransaction: client_id=" << pClient->ClientID() << " session_id=" << pSession->GetNumber() << std::endl;
		else LogLevel(timerlog,4) << timestamp << "ServerProtocolService::ProcessTransaction: clientid=" << pClient->ClientID() << std::endl;
		if (!pRequest->BuildRequest())
		{
			delete pRequest;
			return NULL;
		}
	}
	catch(BasicException e)
	{
		pTransaction->GetIssuer()->TransactionError(pTransaction, e.what(), protocol::Fatal);
		delete pRequest;
		return NULL;
	}

	if (pSession) pSession->SetActiveTransaction(pTransaction);

	mpRequestQueue->AddRequest(pRequest);
	return pRequest;
}

const char* ServerProtocolService::GetCrossDomainPolicy()
{
	return mpService->GetCrossDomainPolicy().c_str();
}
