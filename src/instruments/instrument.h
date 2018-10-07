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

#pragma once
#ifndef __INSTRUMENT_H__
#define __INSTRUMENT_H__

#include "visitor.h"

#include <string>

class NetList2;
class InstrumentBlock;

/// Baseclass for all instruments.
/// Holds type information and methods that every instrument must support.
/// Keeps track of which InstrumentBlock the instrument is created in. So many instruments of the same type can coexist (in different blocks). 
/// All instruments must be supported by the InstrumentVisitor for serialization etc.

class Instrument
{
public:

	// \todo move these to the classes.. no need to change this file when you add a new instrument
	enum InstrumentType
	{
		TYPE_Oscilloscope			= 0,
		TYPE_FunctionGenerator		= 1,
		TYPE_DigitalMultimeter		= 4,
		TYPE_ExternalPeripherals	= 8,
		TYPE_DigitalIO				= 9,
		TYPE_NodeInterpreter		= 10,
		TYPE_TripleDC				= 11,
		TYPE_SignalAnalyzer			= 12,
		TYPE_UNDEFINED				= 63,
	};

	//static InstrumentType		GetStaticType() { return TYPE_UNDEFINED; }
	InstrumentType		GetType() const	{ return mType; }

	virtual bool		IsMeasurementEq()	const { return false; }
	virtual bool		IsSourceEq()		const { return false; }

	/// part of the visitor pattern
	virtual void		Accept(InstrumentVisitor& visitor) = 0;

	/// Validate instrument state
	virtual bool		Validate() = 0;

	virtual bool		CheckMaxValues(const NetList2& maxlist) { return true; }

	/// Copy the instrument state from another instrument
	/// throws exception if the instrument type differs
	virtual void		CopyFrom(Instrument* pInstrument);

	virtual void		ResetData() {}

	int					GetID() const 		{ return mID;			}

	static bool			Error(const std::string& errorstring);

	static inline bool	CheckStringValue(const std::string& value, const char* array[])
	{
		for(int i=0; array[i]; i++)	if (value == array[i]) return true;
		return false;
	}

	// ctor/dtor
			Instrument(InstrumentType type, int instrumentID);
	virtual ~Instrument();
protected:
	InstrumentType		mType;
	int					mID;
};

#define INSTRUMENT_TYPE(type) static Instrument::InstrumentType	GetStaticType() { return (type) ; }

#endif
