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

#include "systemtransactions.h"

#include "client.h"
#include "authentication.h"
#include "session.h"

#include <protocol/basic_types.h>
#include <protocol/auth.h>
#include <basic_exception.h>

#include <syslog.h>

SystemTransactionHandler::SystemTransactionHandler(Authentication* pAuth)
{
	mpAuth = pAuth;
}

SystemTransactionHandler::~SystemTransactionHandler()
{
}

bool SystemTransactionHandler::CanHandle(protocol::Transaction* pTransaction)
{
	typedef protocol::Transaction::tRequests tRequests;
	const tRequests& requests = pTransaction->GetRequests();

	for(tRequests::const_iterator it = requests.begin(); it != requests.end(); it++)
	{
		switch((*it)->GetType())
		{
		case protocol::RequestType::Authorize:
		case protocol::RequestType::Heartbeat:
		case protocol::RequestType::DomainPolicy:
			continue;
		default:
			return false;
		}
	}

	return true;
}

bool SystemTransactionHandler::Perform(protocol::Transaction* pTransaction, protocol::TransactionCallback* pCallback)
{
	typedef protocol::Transaction::tRequests tRequests;
	const tRequests& requests = pTransaction->GetRequests();

	for(tRequests::const_iterator it = requests.begin(); it != requests.end(); it++)
	{
		Client* pClient = (Client*) pTransaction->GetOwner();

		switch((*it)->GetType())
		{
		case protocol::RequestType::Authorize:
			
			if (!pClient) throw BasicException("No Client!");
			
			//if (pClient->GetAuthorized())
			if (false) //pClient->GetAuthorized())
			{
				pCallback->TransactionError("Already authorized", protocol::Fatal);
				return false;
			}
			else
			{
				protocol::AuthRequest* pAuth = (protocol::AuthRequest*)(*it);
				if (mpAuth->Authenticate(pClient, pAuth->GetCookie(), pAuth->GetKeepAlive()))
				{
					syserr << "Authenticated: " << pAuth->GetCookie() << " keepalive: " << pAuth->GetKeepAlive() << std::endl;
					Session* pSession = pClient->GetSession();
					if (pSession == NULL)
					{
						throw BasicException("For some reason, the session is null after auth");
					}

					pAuth->SetResponse(new protocol::AuthResponse(pClient->GetSession()->GetKey()));
					pCallback->TransactionDone();
					return true;
				}
				else
				{
					pCallback->TransactionError("Authorization failed", protocol::Fatal);
					return false;
				}
			}
		break;

		case protocol::RequestType::Heartbeat:
			pCallback->TransactionDone();
			return true;
		break;

		case protocol::RequestType::DomainPolicy:
			pCallback->TransactionDone();			
			return true;
		break;
		
		default:
			throw BasicException("Unknown type");
		break;
		}
	}

	pCallback->TransactionError("Unknown system transaction", protocol::Fatal);
	return true;
}

bool SystemTransactionHandler::Cancel(protocol::Transaction* pTransaction, protocol::TransactionCallback* pCallback)
{
	return true;
}
