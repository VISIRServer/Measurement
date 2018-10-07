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
 * Copyright (c) 2004-2009 Johan Zackrisson
 * All Rights Reserved.
 */

#include "instrument.h"
#include "instrumentblock.h"
#include "nodeinterpreter.h"

#include <stringop.h>
#include <basic_exception.h>

Instrument::Instrument(InstrumentType type, int instrumentID)
{
	mType = type;
//	mName = "<unknown instrument>";

	mID = instrumentID;
}

Instrument::~Instrument()
{
}

bool Instrument::Error(const std::string& errorstring)
{
	throw ValidationException(errorstring);
	return false;
}

void Instrument::CopyFrom(Instrument* pInstrument)
{
	if (pInstrument->GetType() != mType) throw BasicException("Instrument::CopyFrom(): not same type of instrument");
	//mName		= pInstrument->mName;
	//mID		= pInstrument->mID;
}
