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

#include "gpib_transactions.h"
#include "gpib_signalanalyzer.h"

#include <protocol/basic_types.h>
#include <instruments/instrument.h>
#include <instruments/instrumentblock.h>
#include <instruments/signalanalyzer.h>

#include <syslog.h>
#include <basic_exception.h>

using namespace gpib;

GPIBTransactionHandler::GPIBTransactionHandler()
{
	mpAnalyzer = NULL;
}

GPIBTransactionHandler::~GPIBTransactionHandler()
{
}

void GPIBTransactionHandler::Init(GPIBSignalAnalyzer* pAnalyzer)
{
	mpAnalyzer = pAnalyzer;
}

bool GPIBTransactionHandler::CanHandle(protocol::Transaction* pTransaction)
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

bool GPIBTransactionHandler::CanHandleMeasurement(protocol::MeasureRequest* pMeasureRq)
{
	typedef protocol::MeasureRequest::tCmdList tCmdList;
	const tCmdList& cmds = pMeasureRq->GetCmdList();
	for(tCmdList::const_iterator it = cmds.begin(); it != cmds.end(); it++)
	{
		switch((*it)->InstrumentType())
		{
		case Instrument::TYPE_SignalAnalyzer:
			// accepted types
			continue;
		default:
			// everything else should not be accepted
			return false;
		}
	}

	return true;
}

bool GPIBTransactionHandler::Perform(protocol::Transaction* pTransaction, protocol::TransactionCallback* pCallback)
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

bool GPIBTransactionHandler::PerformMeasurement(protocol::Transaction* pTransaction, protocol::MeasureRequest* pMeasureRq, protocol::TransactionCallback* pCallback)
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
		Validate(pBlock);
	
		SignalAnalyzer* pAnalyzer = pBlock->Get<SignalAnalyzer>(1);
		if (!pAnalyzer)
		{
			pCallback->TransactionError("no signal analyser data received", protocol::Fatal);
			return true;
		}

		if (!mpAnalyzer->Measure(pAnalyzer))
		{
			// the measurement failed..
			std::string error = "GPIB Error: ";
			error += mpAnalyzer->GetError();
			mpAnalyzer->ResetError();
			
			pCallback->TransactionError(error.c_str(), protocol::Notification);
		}
		else
		{
			pCallback->TransactionDone();
		}

		return true;
	}
	catch(BasicException e)
	{
		syserr << "GPIBTransactionHandler::PerformMeasurement(): Failed to complete the measurement" << std::endl;
		throw;
	}

	// never reached
	return false;
	

}

bool GPIBTransactionHandler::Cancel(protocol::Transaction* pTransaction, protocol::TransactionCallback* pCallback)
{
	// xxx: this is not good at all..
	//mpControl->CancelRequest(mpAdaptor);
	return true;
}

bool GPIBTransactionHandler::Validate(InstrumentBlock* pBlock)
{
	InstrumentBlock::tInstruments instruments = pBlock->GetInstruments();
	if (instruments.empty()) return false; // if we're here.. we should have at least one

	for(InstrumentBlock::tInstruments::iterator it = instruments.begin(); it != instruments.end(); it++)
	{
		if (!(*it)->Validate())
		{
			syslog << "GPIBRequest::Send: Instrument block state is not valid" << std::endl;
			throw BasicException("Instrument state not valid"); // ok?
			return false; // never reached..
		}
	}

	return true;
}
