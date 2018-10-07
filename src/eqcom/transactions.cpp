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

#include "transactions.h"
#include "control.h"

#include <measureserver/module.h>

#include <protocol/basic_types.h>
#include <instruments/instrument.h>

#include <basic_exception.h>
#include <syslog.h>

using namespace EqSrv;
using namespace std;

class EqSrv::CallbackAdaptor : public RequestCallback
{
public:
	virtual void RequestDone()
	{
		mpCallback->TransactionDone();
	}

	virtual void Error(std::string msg, protocol::TransactionErrorType type)
	{
		mpCallback->TransactionError(msg.c_str(), type);
	}

	void SetCallback(protocol::TransactionCallback* pCallback) { mpCallback = pCallback; }
	CallbackAdaptor() { mpCallback = NULL; }
private:
	protocol::TransactionCallback* mpCallback;
};

EqTransactionHandler::EqTransactionHandler(ModuleServices* pService)
{
	mpAdaptor = new CallbackAdaptor();
	mMatrixDisabled = false;

	mpService = pService;
}

EqTransactionHandler::~EqTransactionHandler()
{
	delete mpAdaptor;
}

void EqTransactionHandler::Init(EquipmentServerControl* pControl, bool matrixDisabled)
{
	mpControl = pControl;
	mMatrixDisabled = matrixDisabled;
}

bool EqTransactionHandler::CanHandle(protocol::Transaction* pTransaction)
{
	typedef protocol::Transaction::tRequests tRequests;
	const tRequests& requests = pTransaction->GetRequests();

	for(tRequests::const_iterator it = requests.begin(); it != requests.end(); it++)
	{
		if ((*it)->GetType() == protocol::RequestType::Measurement)
		{
			protocol::MeasureRequest* pMeasure = (protocol::MeasureRequest*) *it;
			return CanHandleMeasurement(pMeasure);
		}
		else return false;
	}

	return false;
}

bool EqTransactionHandler::CanHandleMeasurement(protocol::MeasureRequest* pMeasureRq)
{
	typedef protocol::MeasureRequest::tCmdList tCmdList;
	const tCmdList& cmds = pMeasureRq->GetCmdList();
	for(tCmdList::const_iterator it = cmds.begin(); it != cmds.end(); it++)
	{
		switch((*it)->InstrumentType())
		{
		case Instrument::TYPE_Oscilloscope:
		case Instrument::TYPE_FunctionGenerator:
		case Instrument::TYPE_DigitalMultimeter:
		case Instrument::TYPE_NodeInterpreter:
		case Instrument::TYPE_TripleDC:
			// accepted types
			continue;
		default:
			// everything else should not be accepted
			return false;
		}
	}

	return true;
}

bool EqTransactionHandler::Perform(protocol::Transaction* pTransaction, protocol::TransactionCallback* pCallback)
{
	typedef protocol::Transaction::tRequests tRequests;
	const tRequests& requests = pTransaction->GetRequests();

	for(tRequests::const_iterator it = requests.begin(); it != requests.end(); it++)
	{
		if ((*it)->GetType() == protocol::RequestType::Measurement)
		{
			protocol::MeasureRequest* pMeasure = (protocol::MeasureRequest*) *it;
			return PerformMeasurement(pTransaction, pMeasure, pCallback);
		}
		else return false;
	}

	return false;
}

bool EqTransactionHandler::PerformMeasurement(protocol::Transaction* pTransaction, protocol::MeasureRequest* pMeasureRq, protocol::TransactionCallback* pCallback)
{
	InstrumentBlock* pBlock = pCallback->GetInstrumentBlock();

	// apply all the settings to the instrument block..
	const protocol::MeasureRequest::tCmdList& cmds = pMeasureRq->GetCmdList();
	for(protocol::MeasureRequest::tCmdList::const_iterator it = cmds.begin(); it != cmds.end(); it++)
	{
		(*it)->ApplySettings(pBlock);
	}

	try
	{
		if (!mMatrixDisabled) mpService->TranslateCircuitAndValidate(pBlock);
		mpAdaptor->SetCallback(pCallback);
		mpControl->SendBakedRequest(pBlock, mpAdaptor);
		return true;
	}
	catch(ValidationException e)
	{
		syserr << "EqTransactionHandler::PerformMeasurement: validation exception: " << e.what() << endl;
		pCallback->TransactionError(e.what(), protocol::Notification);
		return false;
	}
	catch(BasicException e)
	{
		syserr << "EqTransactionHandler::PerformMeasurement(): Failed to complete the measurement" << endl;
		throw e;
	}

	// never reached
	return false;
}

bool EqTransactionHandler::Cancel(protocol::Transaction* pTransaction, protocol::TransactionCallback* pCallback)
{
	// xxx: this is not good at all..
	mpControl->CancelRequest(mpAdaptor);
	return true;
}
