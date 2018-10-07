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
 * Copyright (c) 2008 Johan Zackrisson
 * All Rights Reserved.
 */

#include "xmlconnection.h"
#include "xmlserver.h"

#include <network/connection.h>
#include <syslog.h>
#include <sstream>

#include <basic_exception.h>

#include <measureserver/clientmanager.h>
#include <measureserver/protocolservice.h>
#include <measureserver/session.h>
#include <measureserver/transactionrequest.h>
#include <measureserver/requestqueue.h>

#include <xmlprotocol/producer.h>
#include <xmlprotocol/requestparser.h>

using namespace std;

#define MAX_REQUEST_SIZE (128*1024)



XMLConnection::XMLConnection(Net::Connection* pConnection, XMLServer* pServer, ServerProtocolService* pSrvProtSrvc, ClientManager* pClientMgr, double shorttimeout, double timeout)
{
	mpConnection = pConnection;

	mpServer	= pServer;
	mpSrvProtSrvc = pSrvProtSrvc;
	mpClientMgr = pClientMgr;

	mpConnection->SetNonBlocking();
	mpConnection->SetSelectMask(NET_READ_FLAG | NET_EXCEPTION_FLAG);

	mState = eNormal;

	mCurrentPos = 0;
	mValidPackets = 0;
	mpClient = NULL;
	mpCurrentRequest = NULL;

	mShortTimeout	= shorttimeout;
	mTimeout		= timeout;
}

XMLConnection::~XMLConnection()
{
	if (mpCurrentRequest)
	{
		syserr << "A transaction was canceled mid flight (dtor)" << endl;
		mpCurrentRequest->ClientGone();
		mpCurrentRequest = NULL;
	}

	if (mpClient)
	{
		mpClient->RemoveListener(this);

		mpSrvProtSrvc->GetRequestQueue()->RemoveRequestsFrom(mpClient);
		mpClient->ConnectionClosed();
		mpClientMgr->RemoveClient(mpClient);
		delete mpClient;
		mpClient = NULL;
	}

	delete mpConnection;
}

bool XMLConnection::Init()
{
	mpClient = mpClientMgr->AddClient(mpConnection);
	if (!mpClient)
	{
		Error("Unable to create client, server overloaded");
		return false;
	}
	mpClient->AddListener(this);

	return true;
}

void XMLConnection::HandleEvent(int flags)
{
	if (flags & NET_EXCEPTION_FLAG)
	{
		Shutdown();
		return;
	}

	if (flags & NET_WRITE_FLAG)
	{
		if (!mSendBuffer.Empty())
		{
			//cout << "sending response" << endl;

			int rv = mSendBuffer.Send(mpConnection);
			if (rv < 0)
			{
				syserr << "failed to send response" << endl;
				Shutdown();
				return;
			}
			else if (rv > 0)
			{
				mSendBuffer.Clear();
				// we don't care about sending anymore
				mpConnection->SetSelectMask(NET_READ_FLAG | NET_EXCEPTION_FLAG);

				if (mState == eClosing)
				{
					Shutdown();
					return;
				}
			}
			// if we are not finished sending, we don't want the connection closed
		}
		else if (mState == eClosing)
		{
			Shutdown();
			return;
		}
	}

	if ( (flags & NET_READ_FLAG) && (mState != eClosing) )
	{
		int rv = mReceiveBuffer.Receive(mpConnection, MAX_REQUEST_SIZE);
		if (rv < 0)
		{
			//syserr << "Couldn't read xml request, client probably disconnected" << endl;
			Shutdown();
			return;
		}
		else if (rv == 1)
		{
			if (mReceiveBuffer.GetSize() >= MAX_REQUEST_SIZE)
			{
				Error("Request to large");
				return;
			}
		}

		ParseRequest();
	}
}

void XMLConnection::SendResponse(const char* buffer, size_t length)
{
	xmllog.Log(5) << "XML reponse: " << endl << string(buffer, length) << endl;

	mSendBuffer.Fill((void*)buffer, length);
	mSendBuffer.Fill((void*)"\0", 1);
	mpConnection->SetSelectMask(NET_WRITE_FLAG | NET_READ_FLAG | NET_EXCEPTION_FLAG);
}

void XMLConnection::SendError(std::string msg)
{
	Error(msg);
}

Net::Socket* XMLConnection::GetSocket()
{
	return mpConnection;
}

bool XMLConnection::IsAlive()
{
	if (mValidPackets == 0 && mLifeTimer.elapsed() > mShortTimeout) return false;
	if (mLifeTimer.elapsed() > mTimeout) return false;
	return true;
}

bool XMLConnection::Shutdown()
{
	if (mpCurrentRequest)
	{
		syserr << "A transaction was canceled mid flight (Shutdown)" << endl;
		mpCurrentRequest->ClientGone();
		mpCurrentRequest = NULL;
	}

	if (mpClient) mpClient->ConnectionClosed();
	mpConnection->Disconnect();
	mpServer->RemoveConnection(this); // this will delete us
	return true;
}

void XMLConnection::Error(std::string error)
{
	std::stringstream out;
	xmlprotocol::XmlProducer::ProduceError(out, error);

	xmllog.Log(5) << timestamp << "XML Error reponse: " << endl << out.str() << endl;

	mSendBuffer.Fill((void*)out.str().c_str(), out.str().size());
	mSendBuffer.Fill((void*)"\0", 1);
	mState = eClosing;
	mpConnection->SetSelectMask(NET_WRITE_FLAG | NET_READ_FLAG | NET_EXCEPTION_FLAG);
}

void XMLConnection::ParseRequest()
{
	char* buffer = (char*) mReceiveBuffer.GetBuffer();
	size_t length = mReceiveBuffer.GetSize();
	while(mCurrentPos < length)
	{
		if (buffer[mCurrentPos] == '\0')
		{
			if (!HandlePacket(buffer, mCurrentPos))
			{
				mState = eClosing;
			}
			else
			{
				mValidPackets++;
				mLifeTimer.restart();
			}

			mReceiveBuffer.EraseFront(mCurrentPos+1);
			mCurrentPos = 0;
			return;
		}

		mCurrentPos++;
	}
}

void XMLConnection::Close(bool forceful)
{
	if (forceful)
	{
		Shutdown();
	}
	else
	{
		mState = eClosing;
	}
}

bool XMLConnection::HandlePacket(const char* pData, size_t length)
{
	xmllog.Log(5) << "XML request: " << endl << string(pData, length) << endl;

	xmlprotocol::RequestParser parser;
	xmlprotocol::RequestParser::tTransactions transactions;
	
	try
	{
		if (!parser.ParsePacket(pData, length, transactions))
		{
			Error("Can't understand request");
			return false;
		}
	}
	catch(BasicException e)
	{
		Error(e.what());
		return false;
	}

	if ( (transactions.size() == 0) || (transactions.size() > 1) )
	{
		Error("Zero or more than one transaction in request");

		// clear the transaction list!
		while(transactions.size() > 0)
		{
			delete transactions.front();
			transactions.pop_front();
		}

		return false;
	}

	protocol::Transaction* pTransaction = transactions.front();
	protocol::TransactionErrorType errtype = pTransaction->GetErrorState();
	if (errtype != protocol::NoError)
	{
		Error(pTransaction->GetError());
		delete pTransaction;
		return false;
	}

	pTransaction->SetIssuer(this);
	pTransaction->SetOwner(mpClient);

	// hand over ownership to the handler
	mpCurrentRequest = mpSrvProtSrvc->ProcessTransaction(pTransaction, mpClient);
	if (mpCurrentRequest == NULL) return false;
	
	return true;	
}

void XMLConnection::TransactionComplete(protocol::Transaction* pTransaction)
{
	mpCurrentRequest = NULL;

	Session* pSession = NULL;
	InstrumentBlock* pBlock = NULL;

	if (mpClient) pSession = mpClient->GetSession();
	if (pSession) pBlock = pSession->GetBlock();	
	
	std::stringstream out;
	xmlprotocol::XmlProducer::TransactionResponse(pTransaction, pBlock, out, mpSrvProtSrvc);

	protocol::TransactionErrorType errtype = pTransaction->GetErrorState();
	if (errtype != protocol::NoError)
	{
		Error(pTransaction->GetError());
		return;
	}

	SendResponse(out.str().c_str(), out.str().size());
}

void XMLConnection::TransactionError(protocol::Transaction* pTransaction, const char* msg, protocol::TransactionErrorType type)
{
	mpCurrentRequest = NULL;
	Error(msg);
}


void XMLConnection::SessionDestroyed()
{
	syserr << "Session is destroyed while client is connected!" << endl;

	if (mpCurrentRequest)
	{
		// Cancel the request by pretending that the client has disconnected
		mpCurrentRequest->ClientGone();
		mpCurrentRequest = NULL;
	}

	Error("Your session has timed out because of inactivity");
}