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

#include "measurement.h"
#include "response.h"
#include "control.h"

#include <basic_exception.h>
#include <syslog.h>

using namespace EqSrv;
using namespace std;

class MeasurementResponseAdaptor : public ResponseAdaptor
{
public:
	virtual void NotHandled() { throw BasicException("response not handled"); }
	virtual InstrumentBlock* GetBlock() { return mpBlock; }

	MeasurementResponseAdaptor(InstrumentBlock* pBlock) : mpBlock(pBlock) {}
	virtual ~MeasurementResponseAdaptor() {}
private:
	InstrumentBlock* mpBlock;
};

///////////////////

EqMeasurement::EqMeasurement()
{
	mpBlock = NULL;
	mpCallback = NULL;
}

EqMeasurement::~EqMeasurement()
{
}

void EqMeasurement::Setup(InstrumentBlock* pBlock, RequestCallback* pCallback)
{
	mpBlock = pBlock;
	mpCallback = pCallback;
}

void EqMeasurement::OnResponse(Serializer& in)
{
	try
	{
		if (!mpBlock)
		{
			syserr << "Response on canceled request" << endl;
			return;
		}

		MeasurementResponseAdaptor adaptor(mpBlock);
		EquipmentServerResponse::ParseResponse(in, adaptor, NULL); // don't care about circuit information
		if (mpCallback)
		{
			mpCallback->RequestDone();
		}
		else
		{
			syserr << "Response on canceled request" << endl;
		}
	}
	catch(BasicException e)
	{
		OnError(e.what());
	}
}

void EqMeasurement::OnError(std::string msg)
{
	if (mpCallback) mpCallback->Error(msg, protocol::Fatal);
}