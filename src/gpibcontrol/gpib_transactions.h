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

#ifndef __GPIB_TRANSACTIONS_H__
#define __GPIB_TRANSACTIONS_H__

#include <protocol/protocol.h>

class InstrumentBlock;

namespace protocol { class MeasureRequest; }

namespace gpib
{

class GPIBSignalAnalyzer;

class GPIBTransactionHandler : public protocol::TransactionHandler
{
public:
	virtual bool	CanHandle(protocol::Transaction* pTransaction);
	virtual bool	Perform(protocol::Transaction* pTransaction, protocol::TransactionCallback* pCallback);
	virtual bool	Cancel(protocol::Transaction* pTransaction, protocol::TransactionCallback* pCallback);

	void	Init(GPIBSignalAnalyzer* pAnalyzer);
	GPIBTransactionHandler();
	virtual ~GPIBTransactionHandler();
private:
	bool CanHandleMeasurement(protocol::MeasureRequest* pMeasureRq);
	bool PerformMeasurement(protocol::Transaction* pTransaction, protocol::MeasureRequest* pMeasureRq, protocol::TransactionCallback* pCallback);

	bool Validate(InstrumentBlock* pBlock);

	GPIBSignalAnalyzer* mpAnalyzer;
};

} // end of namespace

#endif
