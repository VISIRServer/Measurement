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

#include "scgiconnection.h"
#include "scgiserver.h"
#include "scgirequest.h"

#include <network/connection.h>
#include <syslog.h>
#include <sstream>

#include <measureserver/clientmanager.h>
#include <measureserver/protocolservice.h>
#include <measureserver/session.h>
#include <measureserver/service.h>
#include <measureserver/transactionrequest.h>
#include <measureserver/requestqueue.h>

#include <xmlprotocol/producer.h>
#include <xmlprotocol/requestparser.h>

#include <basic_exception.h>

using namespace std;

#define MAX_REQUEST_SIZE (128*1024)

const char* SCGIindexPage = "<html>"
"<body>"
"<h1>OpenLabs Measurement Server</h1>"
"<p>This is the <a href='scgi://openlabs.bth.se'>OpenLabs</a> measurement server reporting for duty.</p>"
"</body>"
"</html>";

SCGIConnection::SCGIConnection(Net::Connection* pConnection, SCGIServer* pServer, ServerProtocolService* pSrvProtSrvc, ClientManager* pClientMgr)
{
	mpConnection = pConnection;
	mpServer	= pServer;
	mpSrvProtSrvc = pSrvProtSrvc;
	mpClientMgr	= pClientMgr;

	mpRequest = new SCGIRequest();

	mRequestSize = MAX_REQUEST_SIZE;

	mpConnection->SetNonBlocking();
	mpConnection->SetSelectMask(NET_READ_FLAG | NET_EXCEPTION_FLAG);

	mState = eRequest;
	mKeepAlive = false;

	mpClient = NULL;

	mpCurrentRequest = NULL;

	static int id = 1;
	mRequestID = id++;
}

SCGIConnection::~SCGIConnection()
{
	mpConnection->Destroy();
	if (mpCurrentRequest)
	{
		mpCurrentRequest->ClientGone();
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

	delete mpRequest;
	delete mpConnection;
}

bool SCGIConnection::Init()
{
	mpClient = mpClientMgr->AddClient(mpConnection);
	if (!mpClient)
	{
		SCGIError("Unable to create client, server overloaded");
		mState = eClosing;
		return false;
	}
	mpClient->AddListener(this);

	return true;
}

void SCGIConnection::HandleEvent(int flags)
{
	//cout << "HandleEvents: " << mRequestID << " " << (flags & NET_EXCEPTION_FLAG ? "X" : "") << (flags & NET_WRITE_FLAG ? "W" : "") << (flags & NET_READ_FLAG ? "R" : "") << endl;
	if (flags & NET_EXCEPTION_FLAG)
	{
		Shutdown();
		return;
	}

	if (flags & NET_WRITE_FLAG)
	{
		if (!mSendBuffer.Empty())
		{
			scgilog.Log(5) << "sending reponse" << endl;

			int rv = mSendBuffer.Send(mpConnection);
			if (rv < 0)
			{
				syserr << "failed to send scgi response" << endl;
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
					//cout << "Closing after write: " << mRequestID << endl;
					scgilog.Log(5) << "Closing connection after write" << endl;
					//Shutdown();
					mpConnection->Disconnect();
					return;
				}
			}

			return;
		}
		/*else if (mState == eClosing)
		{
			syslog << "Closing connection, no data to send" << endl;
			scgilog.Log(5) << "Closing connection, no data to send" << endl;
			Shutdown();
			return;
		}*/
	}

	if (flags & NET_READ_FLAG)
	{
		if (mState == eClosing)
		{
			// discard any incoming data
			char buffer[20*1024];
			int rv = -1;
			rv = mpConnection->Receive(buffer, sizeof(buffer));
			//cout << "rv: " << rv << endl;
			if (rv <= 0)
			{
				//cout << "closing socket: " << mRequestID << endl;
				//mpConnection->Disconnect();
				Shutdown();
				return;
			}
			else
			{
				//cout << "rv: " << rv << endl;
			}

			return;
		}

		scgilog.Log(5) << "reading: " << mRequestSize << endl;

		int rv = mReceiveBuffer.Receive(mpConnection, mRequestSize);
		if (rv < 0)
		{
			cout << "Coulnd't read scgi request: " << mRequestID << endl;
			//syserr << "Couldn't read scgi request, client probably disconnected" << endl;
			Shutdown();
			return;
		}
		else if (rv == 1)
		{
			/*if (mReceiveBuffer.GetSize() >= MAX_REQUEST_SIZE)
			{
				scgilog.Log(5) << "Receive buffer is to large: " << mReceiveBuffer.GetSize() << endl;
				SCGIError("Request to large");
				mState = eClosing;
				return;
			}*/
		}

		//cout << "Got data: " << mRequestID << " " << mReceiveBuffer.GetSize() << endl;
		scgilog.Log(5) << "Got data: " << mReceiveBuffer.GetSize() << endl;
		if (mReceiveBuffer.GetSize() > 10000)
		{
			scgilog.Log(5) << "something is weird" << endl;
		}

		if (mState != eClosing)
		{
			ParseSCGIRequest(mReceiveBuffer.GetBuffer(), mReceiveBuffer.GetSize());
		}
	}
}

void SCGIConnection::SendResponse(const char* buffer, size_t length)
{
	scgilog.Log(5) << "SCGI XML response: " << endl << string(buffer, length) << endl;

	//string response(buffer, length);

	std::stringstream out;
	out << "Status: 200 OK\r\n";
	out << "Server: Measurementserver\r\n";
	out << "Content-Length: " << length << "\r\n";
	out << "Content-Type: text/xml\r\n";
	out << "Cache-Control: no-cache\r\n";
	out << "Access-Control-Allow-Origin: *\r\n";
	if (mKeepAlive) out << "Connection: keep-alive\r\n";
	else out << "Connection: close\r\n";
	out << "\r\n";
	mSendBuffer.Fill((void*)out.str().c_str(), out.str().size());

	mSendBuffer.Fill((void *)buffer, length);
	mpConnection->SetSelectMask(NET_WRITE_FLAG | NET_READ_FLAG | NET_EXCEPTION_FLAG);
}

void SCGIConnection::SendError(std::string msg)
{
	std::stringstream out;
	xmlprotocol::XmlProducer::ProduceError(out, msg);
	SendResponse(out.str().c_str(), out.str().size());
}

Net::Socket* SCGIConnection::GetSocket()
{
	return mpConnection;
}

bool SCGIConnection::IsAlive()
{
	double timeout = 300.0;
	if (mLifeTimer.elapsed() > timeout) return false;
	return true;
}

bool SCGIConnection::Shutdown()
{
	if (mState != eClosing) {
		sysout << "Closing a unfinished socket: " << mRequestID << endl;
	}

	//cout << "Shutdown socket: " << mRequestID << endl;


	if (mpClient) mpClient->ConnectionClosed();
	mpConnection->Disconnect();
	mpConnection->Destroy();
	mpServer->RemoveConnection(this); // this will delete us
	return true;
}

bool SCGIConnection::ParseSCGIRequest(void* buffer, size_t datalength)
{
	scgilog.Log(5) << "ParseSCGIRequest: " << endl << string((char*)buffer, datalength) << endl;

	int error = 0;
	if (mpRequest->ParseRequest((char*)buffer, datalength, error))
	{
		mLifeTimer.restart();

		// request is done
		if (error)
		{
			SCGIError("Error");
			return false;
		}
		else
		{
			sysout << "SCGI request: " << mpRequest->Verb() << " " << mpRequest->URL() << " from " << mpRequest->RemoteAddr() << endl;

			if (mpRequest->Verb() == "POST" && mpRequest->URL() == "/measureserver")
			{
				string payload = mpRequest->GetPayload();
				//cout << "payload" << endl << payload << endl;
			
				//mpClient->HandlePacket((char*)payload.c_str(), payload.size());
				HandlePacket(payload.c_str(), payload.size());
			}
			else if (mpRequest->Verb() == "GET" && mpRequest->URL() == "/crossdomain.xml")
			{
				sysout << "SCGI Policy file request" << endl;

				std::stringstream out;
				out << mpSrvProtSrvc->GetCrossDomainPolicy();
				SendResponse(out.str().c_str(), out.str().size());
			}
			else if (mpRequest->URL() == "/test")
			{
				std::stringstream out;
				//sysout << "Test request: " << mRequestID << endl;
				out << "Test response: " << mRequestID << endl;
				SendResponse(out.str().c_str(), out.str().size());
			}
			else if (mpRequest->URL() == "/")
			{
				std::stringstream out;
				//out << indexPage;
				//SendResponse(out.str().c_str(), out.str().size());
				out << "Status: 200 OK\r\n";
				out << "Server: Measurementserver\r\n";
				out << "Content-Length: " << strlen(SCGIindexPage) << "\r\n";
				out << "Content-Type: text/html\r\n";
				out << "Connection: close\r\n";
				out << "\r\n";
				out << SCGIindexPage;
				mSendBuffer.Fill((void*)out.str().c_str(), out.str().size());
				mpConnection->SetSelectMask(NET_WRITE_FLAG | NET_READ_FLAG | NET_EXCEPTION_FLAG);
			}
			else
			{
				SCGIError("OK", 200);
			}

			
			if (mpRequest->ConnectionType() == SCGIRequest::eConnectionClose)
			{
				mState = eClosing;
				mKeepAlive = false;
			}
			else
			{
				mKeepAlive = true;
			}

			mReceiveBuffer.EraseFront(mpRequest->RequestSize());
			mpRequest->Reset();			
		}
	}
	return true;
}

void SCGIConnection::SCGIError(std::string error, int errornr)
{
	mState = eClosing;
	syserr << "SCGIError (" << errornr << "): " << error << " " << endl;

	std::stringstream out;
	out << "Status: " << errornr << " " << error << "\r\n";
	out << "SERVER: Measurementserver\r\n";
	out << "Cache-Control: no-cache\r\n";
	out << "Access-Control-Allow-Origin: *\r\n";
	out << "Access-Control-Allow-Methods: GET, POST\r\n";
	out << "Access-Control-Allow-Headers: Content-Type\r\n";

	out << "Content-Length: " << error.length() << "\r\n";
	out << "Connection: close\r\n";
	out << "\r\n";
	out << error;

	mSendBuffer.Fill((void*)out.str().c_str(), out.str().size());
	mpConnection->SetSelectMask(NET_WRITE_FLAG | NET_READ_FLAG | NET_EXCEPTION_FLAG);
}

// copied from XMLConnection, maybe we should make utility functions to avoid duplication
//

bool SCGIConnection::HandlePacket(const char* pData, size_t length)
{
	scgilog.Log(5) << "SCGI XML request: " << endl << string(pData, length) << endl;

	xmlprotocol::RequestParser parser;
	xmlprotocol::RequestParser::tTransactions transactions;
	
	try
	{
		if (!parser.ParsePacket(pData, length, transactions))
		{
			SendError("Can't understand request");
			return false;
		}
	}
	catch(BasicException e)
	{
		SendError(e.what());
		return false;
	}

	if ( (transactions.size() == 0) || (transactions.size() > 1) )
	{
		//throw BasicException("Zero or more than one transaction");
		SendError("Zero or more than one transaction in request");

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
		SendError(pTransaction->GetError());
		delete pTransaction;
		return false;
	}

	pTransaction->SetIssuer(this);
	pTransaction->SetOwner(mpClient);

	// hand over ownership to the handler
	if (mpCurrentRequest != NULL)
	{
		SendError("Transaction already in progress");
	}
	else
	{
		mpCurrentRequest = mpSrvProtSrvc->ProcessTransaction(pTransaction, mpClient);
		if (mpCurrentRequest == NULL) return false;
	}
	
	return true;
}

void SCGIConnection::TransactionComplete(protocol::Transaction* pTransaction)
{
	mpCurrentRequest = NULL;

	Session* pSession = NULL;
	if (mpClient) pSession = mpClient->GetSession();
	if (!pSession)
	{
		SendError("Unable to get session when generating xml response");
		return;
	}

	sysout << "request from session: " << pSession->GetKey() << endl;

	std::stringstream out;
	xmlprotocol::XmlProducer::TransactionResponse(pTransaction, pSession->GetBlock(), out, mpSrvProtSrvc);

	protocol::TransactionErrorType errtype = pTransaction->GetErrorState();
	if (errtype != protocol::NoError)
	{
		SendError(pTransaction->GetError());
		return;
	}

	SendResponse(out.str().c_str(), out.str().size());
}

void SCGIConnection::TransactionError(protocol::Transaction* pTransaction, const char* msg, protocol::TransactionErrorType type)
{
	mpCurrentRequest = NULL;
	SendError(msg);
}

void SCGIConnection::SessionDestroyed()
{
	syserr << "Session is destroyed while client is connected!" << endl;

	if (mpCurrentRequest)
	{
		// Cancel the request by pretending that the client has disconnected
		mpCurrentRequest->ClientGone();
		mpCurrentRequest = NULL;
	}

	SendError("Your session has timed out during measurement");
	mState = eClosing;
}