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

#include "digitalmultimeter.h"

#include "instrumentblock.h"

#include <syslog.h>
#include <limits>

DigitalMultimeter::DigitalMultimeter(int instrumentID) : Instrument(Instrument::TYPE_DigitalMultimeter, instrumentID)
{
	mFunction		= DCVolts;
	mResolution		= Digit4_5;
	mRange			= -1.0; // autorange
	mMeasureResult	= 0.0;
	mAutoZero		= Off; // XXX: Once leads to errors for some dmms
}

DigitalMultimeter::~DigitalMultimeter()
{
}

bool DigitalMultimeter::MeasuresCurrent() const
{
	if (mFunction == DCCurrent || mFunction == ACCurrent) return true;
	else return false;
}

bool DigitalMultimeter::MeasuresResistance() const
{
	if (mFunction == Resistance) return true;
	else return false;
}

void DigitalMultimeter::Accept(InstrumentVisitor& visitor)
{
	visitor.Visit(*this);
}

bool DigitalMultimeter::Validate()
{
	return true;
}

void DigitalMultimeter::CopyFrom(Instrument* pInstrument)
{
	Instrument::CopyFrom(pInstrument);

	DigitalMultimeter* pDMM = (DigitalMultimeter*) pInstrument;
	
	mFunction				= pDMM->mFunction;
	mResolution				= pDMM->mResolution;
	mRange					= pDMM->mRange;
	mAutoZero				= pDMM->mAutoZero;
	mMeasureResult			= pDMM->mMeasureResult;
}

void DigitalMultimeter::ResetData()
{
	if (mFunction == Resistance) mMeasureResult = std::numeric_limits<double>::quiet_NaN();
	else mMeasureResult = 0.0;
}

static const char* sFunction[] = { "dc volts", "ac volts", "dc current", "ac current", "resistance", "resistance4", "diode", "frequency", "period", "ac voltage dc coupled", "capacitance", "inductance"  ,NULL };
IMPL_SET_GET_STR(DigitalMultimeter, FunctionStr, sFunction, mFunction, Function)

static const char* sResolution[] = { "3.5", "4.5", "5.5", "6.5", NULL };
IMPL_SET_GET_STR(DigitalMultimeter, ResolutionStr, sResolution, mResolution, Resolution)


