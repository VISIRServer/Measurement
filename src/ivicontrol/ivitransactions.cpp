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
 * Copyright (c) 2008 André van Schoubroeck
 * Copyright (c) 2008-2009 Johan Zackrisson
 * All Rights Reserved.
 */

#include "ivitransactions.h"
#include "ivimodule.h"
#include "experiment.h"
#include "ividrivers.h"

#include <measureserver/service.h>

#include <protocol/basic_types.h>
#include <instruments/instrument.h>

#include <usbmatrix/matrix.h>

#include <basic_exception.h>

#include <list>
#include <vector>
#include <iostream>

using namespace IVIControl;
using namespace std;

IVITransactionHandler::IVITransactionHandler()
{
	mpDrivers			= NULL;
	mpListParser		= NULL;
	mpMatrix			= NULL;
	mpServices			= NULL;
}

IVITransactionHandler::~IVITransactionHandler()
{
	if(mpMatrix) delete mpMatrix;
}

void IVITransactionHandler::Init(Drivers* pDrivers, ListParser* pListParser, ModuleServices* pServices)
{
	mpDrivers			= pDrivers;
	mpListParser		= pListParser;
	mpMatrix			= new USBMatrix::Matrix(pDrivers->GetUSB(0));
	mpServices			= pServices;
}

bool IVITransactionHandler::CanHandle(protocol::Transaction* pTransaction)
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



bool IVITransactionHandler::CanHandleMeasurement(protocol::MeasureRequest* pMeasureRq)
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

bool IVITransactionHandler::Perform(protocol::Transaction* pTransaction, protocol::TransactionCallback* pCallback)
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

bool IVITransactionHandler::PerformMeasurement(protocol::Transaction* pTransaction, protocol::MeasureRequest* pMeasureRq, protocol::TransactionCallback* pCallback)
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
		mpServices->TranslateCircuitAndValidate(pBlock);

		Experiment::DoExperiment(pBlock, mpDrivers, mpListParser, mpMatrix);
		pCallback->TransactionDone();

		return true;
	}
	catch(ValidationException e)
	{
		cerr << "IVITransactionHandler::PerformMeasurement: validation exception: " << e.what() << endl;
		pCallback->TransactionError(e.what(), protocol::Notification);
		return false;
	}
	catch(BasicException e)
	{
		cerr << "IVITransactionHandler::PerformMeasurement(): Failed to complete the measurement" << endl;
	    pCallback->TransactionError(e.what(), protocol::Notification);
	    return false;
//		throw e;
	}

	// never reached
	return false;
}

bool IVITransactionHandler::Cancel(protocol::Transaction* pTransaction, protocol::TransactionCallback* pCallback)
{
	// xxx: this is not good at all..
	//mpControl->CancelRequest(mpAdaptor);
	return true;
}

