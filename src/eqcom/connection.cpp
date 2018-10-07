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

#include "connection.h"
#include "header.h"

#include "eqlog.h"

#include <network/connection.h>
#include <network/multiplexer.h>

#include <stringop.h>

#include <serializer.h>
#include <basic_exception.h>

using namespace EqSrv;
using namespace std;

EqConnection::EqConnection(Net::Multiplexer* pServer, std::string host, int port)
{
	mConnection = new Net::Connection();
	mpCurrentCallback = NULL;
	mHandlerAdded = false;

	mReadState = eReadHeader;

	mpServer = pServer;

	mHost = host;
	mPort = port;
}

EqConnection::~EqConnection()
{
	if (mHandlerAdded) mpServer->RemoveHandler(this);
	mConnection->Disconnect();
	delete mConnection;
}

bool EqConnection::SendCommand(Serializer& out, EqConnectionCallback* callback)
{
	Header::WriteHeader(Header::Data, out);

	eqlog.Log(5) << "EqConnection::SendCommand packet (with header): " << endl << out.GetCStream() << endl;

	mStopClock.restart();

	mSendBuffer.Fill((void*)out.GetCStream().c_str(), out.GetCStream().size());
	
	// replace with a command queue or something..
	mpCurrentCallback = callback;

	if (!mConnection->IsConnected())
	{
		if (!MakeConnection())
		{
			mpCurrentCallback->OnError("Unable to connect to Equipment server");
			return false;
		}
	}

	mConnection->SetSelectMask(NET_WRITE_FLAG | NET_READ_FLAG | NET_EXCEPTION_FLAG);
	return true;
}

void EqConnection::CancelCommand()
{
}

bool EqConnection::MakeConnection()
{
	eqlog.Log(5) << "EqConnection::MakeConnection" << endl;

	mConnection->SetNonBlocking();
	// todo: check if we can reuse this conenction
	if (!mConnection->Connect(mHost.c_str(), mPort)) return false;

	if (!mHandlerAdded)
	{
		mpServer->AddHandler(this);
		mHandlerAdded = true;
	}

	mReadState = eReadHeader;

	return true;
}

void EqConnection::HandleEvent(int flags)
{
	if (flags & NET_EXCEPTION_FLAG)
	{
		eqlog.Error() << "Request failed. Possibly not able to connect to equipment server" << endl;
		if (mpCurrentCallback) mpCurrentCallback->OnError("Equipment server connection failure. Possibly not able to connect. Contact server administrators!");
		Cleanup();
		return;
	}

	if (flags & NET_WRITE_FLAG)
	{
		if (!mSendBuffer.Empty())
		{
			eqlog.Log(5) << "EqConnection::HandleEvent Sending data ( after " << mStopClock.elapsed() << ")" << endl;
			//cerr << "sending data" << endl;

			int rv = mSendBuffer.Send(mConnection);
			if (rv < 0)
			{
				eqlog.Error() << "failed to send all data.." << endl;
				Cleanup();
				return;
			}
			else if (rv > 0)
			{
				mSendBuffer.Clear();
				// we don't care about sending anymore
				mConnection->SetSelectMask(NET_READ_FLAG | NET_EXCEPTION_FLAG);
			}
		}
	}

	if (flags & NET_READ_FLAG)
	{
		// read header
		if (mReadState == eReadHeader)
		{
			//cerr << "reading header" << endl;
			int rv = mReceiveBuffer.Receive(mConnection, 7); // 6 chars + newline
			if (rv < 0)
			{
				if (mReceiveBuffer.GetSize() == 0) // we have been shut down
				{
					eqlog.Log(5) << "Eq server closed session after request" << endl;
				}
				else
				{
					eqlog.Error() << "failed to read equipment server packet header or socket closed" << endl;
				}
				Cleanup();
				return;
			}
			else if (rv > 0)
			{
				mReadState = eReadPacket;
				std::string lenstr;
				lenstr.insert(lenstr.end(), (char*)mReceiveBuffer.GetBuffer(), (char*) mReceiveBuffer.GetBuffer() + 7); //mReceiveBuffer.GetSize());

				int len = ToInt(lenstr.c_str());

				if (len < 0 || len > 65535)
				{
					eqlog.Error() << "Invalid Equipment server response packet length" << endl;
					Cleanup();
					return;
				}

				mPacketSize = len - 1; // minus newline
				mReadState = eReadPacket;
				mReceiveBuffer.Clear();
			}
		}
		else
		{
			int rv = mReceiveBuffer.Receive(mConnection, mPacketSize);
			if (rv < 0)
			{
				eqlog.Error() << "failed to read equipment server packet" << endl;
				Cleanup();
				return;
			}
			else if (rv > 0)
			{
				std::string out;
				out.insert(out.end(), (char*) mReceiveBuffer.GetBuffer(), (char*) mReceiveBuffer.GetBuffer() + mReceiveBuffer.GetSize());

				eqlog.Log(5) << "Eq response packet: " << endl << "'" << out << "'" << endl;

				Serializer aResSer(out.c_str());
				mReadState = eReadHeader;
				mReceiveBuffer.Clear();

				// this may throw, so make sure state is updated before calling
				HandleResponse(aResSer);
			}
		}
	}

	if (!mConnection->IsConnected())
	{
		//syserr << "Somehow the server shut us down" << endl;
		Cleanup();
	}
}

void EqConnection::Cleanup()
{
	if (mHandlerAdded) mpServer->RemoveHandler(this);

	/*if (mpCurrentCallback)
	{
		mpCurrentCallback->OnError("No complete response could be read from eq server");
	}*/

	mHandlerAdded = false;
	mConnection->Destroy();
	mpCurrentCallback = NULL;
	mReadState = eReadHeader;
	mSendBuffer.Clear();
	mReceiveBuffer.Clear();
}

Net::Socket* EqConnection::GetSocket()
{
	return mConnection;
}

bool EqConnection::IsAlive()
{
	return true;
}

bool EqConnection::Shutdown()
{
	return true;
}

void EqConnection::HandleResponse(Serializer& in)
{
	eqlog.Log(5) << "EqConnection::HandleResponse ( after " << mStopClock.elapsed() << ")" << endl;

	if (mpCurrentCallback) mpCurrentCallback->OnResponse(in);
	mpCurrentCallback = NULL; // now the measurement is handled

	// this isn't the "correct" behaivour..
	//but as long as the server doesn't handle multiple requests on a single connection we have no choise
	Cleanup();
}

