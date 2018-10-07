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

#include "control.h"
#include "setupresponse.h"
#include "experiment.h"
#include "measurement.h"
#include "transactions.h"
#include "commands.h"
#include "header.h"
#include "eqlog.h"

#include <network/multiplexer.h>
#include <measureserver/module.h>

#include <protocol/protocol.h>

#include <basic_exception.h>
#include <serializer.h>
#include <config.h>
#include <timer.h>

#include <sstream>

using namespace EqSrv;
using namespace std;

EquipmentServerControl::EquipmentServerControl(Net::Multiplexer* pServer, Config* pConfig, ModuleServices* pService)
{
	mInitDone = false;
	mpMeasurement = new EqMeasurement();
	mpTransactionHandler = new EqTransactionHandler(pService);
	mpCookie = NULL;
	mMatrixDisabled = false;
	mFailed = false;
	
	mpServer = pServer;
	mpEqConnection = NULL;
	mpConfig = pConfig;
	mpService = pService;
}

EquipmentServerControl::~EquipmentServerControl()
{
	mpService->UnregisterTransactionHandler(mpTransactionHandler);

	if (mpEqConnection) { delete mpEqConnection; mpEqConnection = NULL; }
	delete mpMeasurement;
	delete mpTransactionHandler;
}

bool EquipmentServerControl::TryRequestServerInfo()
{
	try
	{
		if (mFailed) // failed or just started, try again or wait for the next retry
		{
			if (mRetries <= 0)
			{
				eqlog.Error() << "Failed to get valid response from equipment server. Giving up.." << endl;
				mFailed = true;
				mInitDone = true;
				return false;
			}
			else if (mTimeout <= 0)
			{
				int retryTimeout	= mpConfig->GetInt("EQ.RetryTimeout", 10);
				mTimeout = retryTimeout;
				mRetries--;

				sysout << endl;
				if (!RequestServerInfo()) {
					mFailed = true;
					mInitDone = true;
					return false;					
				} else {
					return true;
				}
			}
			else
			{
				sys::Sleep(1000);
				sysout << "z";
			}

			mTimeout--;
		}
		else if (!mpServer->WaitForEvent(1000))
		{
			sysout << endl;
			// we need recovery from failed select!
			eqlog.Error() << "Server error: Unable to wait for event. Shuting down..." << endl;
			mFailed = true;
			mInitDone = true;
			return false;
		}
		else
		{
			if (!mFailed) sysout << ".";
		}

		
	}
	catch(BasicException e)
	{
		eqlog.Error() << "EquipmentServerControl failed to init: " << e.what() << endl;
		mFailed = true;
		mInitDone = true;
		return false;
	}
	
	return true;
}

bool EquipmentServerControl::Init()
{
	//int retryCount		= mpConfig->GetInt("EQ.RetryCount", 4);
	//int retryTimeout	= mpConfig->GetInt("EQ.RetryTimeout", 10);
	//int timeoutCycle	= retryTimeout;

	std::string measureHost	= mpConfig->GetString("EQ.Host", "127.0.0.1");
	int measurePort		= mpConfig->GetInt("EQ.Port", 5001);

	mpEqConnection = new EqConnection(mpServer, measureHost, measurePort);

	mMatrixDisabled = (mpConfig->GetInt("ComponentMatrixDisabled", 0) != 0);
	
	mpTransactionHandler->Init(this, mMatrixDisabled);
	mpService->RegisterTransactionHandler(mpTransactionHandler);

	if (mMatrixDisabled) return true;

	mRetries = mpConfig->GetInt("EQ.RetryCount", 4) + 1; // one extra for the normal try
	mTimeout = 0;
	mFailed = true;

	TryRequestServerInfo();
	return true;
}

bool EquipmentServerControl::IsInitDone()
{
	if (mMatrixDisabled) return true;
	if (mInitDone) return true;

	TryRequestServerInfo();
	return false;
}

bool EquipmentServerControl::HasInitFailed()
{
	//return false;
	return mFailed; // last request error status
}

bool EquipmentServerControl::RequestServerInfo()
{
	eqlog.Out(1) << "Sending component list request to equipment server" << endl;
	mFailed = false;
	
	std::stringstream sstream;
	sstream.imbue(std::locale::classic());
	InstrumentCommands::CircuitFetch(sstream);

	Serializer ser;
	ser << sstream.str();

	return mpEqConnection->SendCommand(ser, this);
}

void EquipmentServerControl::OnResponse(Serializer& in)
{
	mInitDone = true;
	mFailed = false;
	try
	{
		SetupResponseAdaptor setupAdaptor;
		EquipmentServerResponse::ParseResponse(in, setupAdaptor, mpService->GetComponentDefinitions());
		mServerNetlist = *setupAdaptor.GetNetList();

		eqlog.Log(3) << "Eqserver returned netlist:" << endl << mServerNetlist.GetNetListAsString() << endl;
		if (!mpService->ValidateMaxlists(mServerNetlist))
		{
			eqlog.Error() << "The returned componentlist is not a superset of the used maxlists" << endl;
		}
	}
	catch(BasicException e)
	{
		eqlog.Error() << "Failed to init: " << e.what() << endl;
		//mResult = false;
		mFailed = true; // xxx
	}
}

void EquipmentServerControl::OnError(std::string msg)
{
	mFailed = true;
	eqlog.Error() << msg << endl;
	//throw BasicException(msg);
}

bool EquipmentServerControl::SendBakedRequest(InstrumentBlock* pBlock, RequestCallback* pCallback)
{
	mpCookie = pCallback; // keep the callback as a cookie, so we know if the matching cancel call is valid

	stringstream sstream;
	Experiment::BuildExperiment(sstream, pBlock, mServerNetlist, mMatrixDisabled);

	Serializer ser;
	ser << sstream.str();
	mpMeasurement->Setup(pBlock, pCallback);

	mpEqConnection->SendCommand(ser, mpMeasurement);
	return true;
}

bool EquipmentServerControl::CancelRequest(RequestCallback* pCallback)
{
	if (pCallback == mpCookie)
	{
		// todo: disconnect from server? seams harch..

		mpEqConnection->CancelCommand();
		mpMeasurement->Setup(NULL,NULL);		
		mpCookie = NULL;
	}
	else
	{
		eqlog.Error() << "EquipmentServerControl::CancelRequest: someone is trying to cancel a request that is not yet sent" << endl;
	}

	return true;
}

bool EquipmentServerControl::Tick()
{
	return true;
}

