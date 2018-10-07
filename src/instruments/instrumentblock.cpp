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

#include "instrumentblock.h"

#include "oscilloscope.h"
#include "functiongenerator.h"
#include "digitalmultimeter.h"
#include "nodeinterpreter.h"
#include "tripledc.h"
#include "signalanalyzer.h"

InstrumentBlock::InstrumentBlock()
{
	mNodeInterpreter = new NodeInterpreter(Instrument::TYPE_NodeInterpreter);
	mInstruments.push_back(mNodeInterpreter);
}

InstrumentBlock::~InstrumentBlock()
{
	for(tInstruments::const_iterator i=mInstruments.begin(); i != mInstruments.end(); i++)
	{
		Instrument* pInstrument = *i;
		delete pInstrument;
	}
	mInstruments.clear();
}

Instrument* InstrumentBlock::AcquireInstrument(Instrument::InstrumentType type, int instrumentId)
{
	Instrument* pInstrument = GetInstrument(type, instrumentId);
	if (pInstrument) return pInstrument;
	else return CreateInstrument(type, instrumentId);
}

InstrumentBlock::tInstruments InstrumentBlock::GetSources()
{
	tInstruments temp;
	for(tInstruments::const_iterator it=mInstruments.begin(); it != mInstruments.end(); it++)
	{
		if ( (*it)->IsSourceEq() ) temp.push_back(*it);
	}
	return temp;
}

InstrumentBlock::tInstruments InstrumentBlock::GetMeasureEq()
{
	tInstruments temp;
	for(tInstruments::const_iterator it=mInstruments.begin(); it != mInstruments.end(); it++)
	{
		if ( (*it)->IsMeasurementEq() ) temp.push_back(*it);
	}
	return temp;
}

Instrument* InstrumentBlock::CreateInstrument(Instrument::InstrumentType type, int id)
{
	Instrument* pInstrument = GetInstrument(type, id);
	if (pInstrument) return pInstrument;

	switch(type)
	{
	case Instrument::TYPE_Oscilloscope:
		pInstrument = new Oscilloscope(id);
		break;
	case Instrument::TYPE_FunctionGenerator:
		pInstrument = new FunctionGenerator(id);
		break;
	case Instrument::TYPE_DigitalMultimeter:
		pInstrument = new DigitalMultimeter(id);
		break;
	case Instrument::TYPE_TripleDC:
		pInstrument = new TripleDC(id);
		break;
	case Instrument::TYPE_SignalAnalyzer:
		pInstrument = new SignalAnalyzer(id);
		break;
	default:
		throw BasicException("Unknown instrument to create");
	}

	if (pInstrument) mInstruments.push_back(pInstrument);
	return pInstrument;
}

Instrument* InstrumentBlock::GetInstrument(Instrument::InstrumentType type, int instrumentId)
{
	for(tInstruments::const_iterator i=mInstruments.begin(); i != mInstruments.end(); i++)
	{
		if ((*i)->GetType() == type && (*i)->GetID() == instrumentId) return *i;
	}

	return 0;
}

InstrumentBlock::tInstruments InstrumentBlock::GetInstrumentsOfType(Instrument::InstrumentType type)
{
	tInstruments temp;
	for(tInstruments::const_iterator i=mInstruments.begin(); i != mInstruments.end(); i++)
	{
		if ( (*i)->GetType() == type ) temp.push_back(*i);
	}

	return temp;
}

void InstrumentBlock::CopyFrom(InstrumentBlock& block)
{
	// create new instruments in the new block
	for(tInstruments::const_iterator i=block.mInstruments.begin(); i != block.mInstruments.end(); i++)
	{
		Instrument* pInstrument = AcquireInstrument((*i)->GetType(), (*i)->GetID());
		pInstrument->CopyFrom(*i);
	}
}
