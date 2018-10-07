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

#ifndef __EQ_SYMBOLS_H__
#define __EQ_SYMBOLS_H__

#include <instruments/instrument.h>

#include <sstream>

namespace EqSrv
{

enum InstrumentID
{
	// sources
	V3_FunctionGenerator		= 11,
	V3_TripleDC					= 12,

	// measurement equipment, returns data
	V3_Oscilloscope				= 21,
	V3_DigitalMultimeter		= 22,
	
	// misc.
	V3_ExtendedPeripherals		= 31,

	// control
	V3_CircuitBuilder			= 41,
	V3_DigitalIO				= 42,
	V3_CircuitSwitch			= 44,
};

static const char* endtoken			= "\n";
static const char* InstrumentSpacer	= "\t";

/// Translate ID to Instrument type
inline Instrument::InstrumentType IDToInstrument(int id)
{
	switch(id)
	{
	case V3_Oscilloscope:			return Instrument::TYPE_Oscilloscope;
	case V3_FunctionGenerator:		return Instrument::TYPE_FunctionGenerator;
	case V3_DigitalMultimeter:		return Instrument::TYPE_DigitalMultimeter;
	case V3_ExtendedPeripherals:	return Instrument::TYPE_ExternalPeripherals;
	case V3_DigitalIO:				return Instrument::TYPE_DigitalIO;
	case V3_TripleDC:				return Instrument::TYPE_TripleDC;
	default:
		return Instrument::TYPE_UNDEFINED;
	}
}

/// Translate instrument type to ID
inline int InstrumentToID(Instrument::InstrumentType type)
{
	switch(type)
	{
	case Instrument::TYPE_Oscilloscope:			return V3_Oscilloscope;
	case Instrument::TYPE_FunctionGenerator:	return V3_FunctionGenerator;
	case Instrument::TYPE_DigitalMultimeter:	return V3_DigitalMultimeter;
	case Instrument::TYPE_ExternalPeripherals:	return V3_ExtendedPeripherals;
	case Instrument::TYPE_DigitalIO:			return V3_DigitalIO;
	case Instrument::TYPE_TripleDC:				return V3_TripleDC;
	default:
		// throw exception?
		return -1;
	}
}

/// Write instrument header
inline std::string InstrumentHead(Instrument& instr)
{
	std::stringstream sstream;
	if(instr.GetType() == Instrument::TYPE_DigitalMultimeter)
		sstream << InstrumentToID(instr.GetType()) << "#" << instr.GetID() << InstrumentSpacer;
	else
		sstream << InstrumentToID(instr.GetType()) << InstrumentSpacer;
	return sstream.str();
}

/// Write instrument header
inline std::string InstrumentHeadType(InstrumentID id)
{
	std::stringstream sstream;
	sstream << (int)id << InstrumentSpacer;
	return sstream.str();
}

} // end of namespace

#endif
