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
#ifndef __INSTRUMENT_BLOCK_H__
#define __INSTRUMENT_BLOCK_H__

#include "instrument.h"

#include <vector>

class NodeInterpreter;

/// currently, the InstrumentBlock only suppors one instance of each instrumenttype
class InstrumentBlock
{
public:
	typedef std::vector<Instrument*> tInstruments;

	/// Creates and returns the instrument if it does not already exist.
	//Instrument*			AcquireInstrument(Instrument::InstrumentType type, int instrumentId = 1);

	template<typename T>
	T*					Acquire(int instrumentId = 1)
	{
		return reinterpret_cast<T*>(AcquireInstrument(T::GetStaticType(), instrumentId));
	}

	/// Returns the requested instrument
	/// returns NULL if the instrument is not found
	template<typename T>
	T*					Get(int instrumentId = 1)
	{
		return reinterpret_cast<T*>(GetInstrument(T::GetStaticType(), instrumentId));
	}

	/// returns a list of instruments of a specific type
	/// Notice: there is a assumtion right now about there beeing just one of each instrument..
	tInstruments		GetInstrumentsOfType(Instrument::InstrumentType type);

	template<typename T>
	std::vector<T*>		GetAll()
	{
		std::vector<T*> temp;
		int type = T::GetStaticType();
		for(tInstruments::const_iterator i=mInstruments.begin(); i != mInstruments.end(); i++)
		{
			if ( (*i)->GetType() == type ) temp.push_back(reinterpret_cast<T*>(*i));
		}
		return temp;
	}

	/// returns all instruments
	tInstruments		GetInstruments()	{ return mInstruments; }

	/// Returns all function generators and powersupplies
	tInstruments		GetSources();
	/// Returns all digitalmultimeters and oscilloscopes
	tInstruments		GetMeasureEq();

	/// Returns the nodeinterpreter
	NodeInterpreter*	GetNodeInterpreter() { return mNodeInterpreter; }

	/// Make a copy of the block and all its components
	void				CopyFrom(InstrumentBlock& block);

	// ctor
	InstrumentBlock();
	// dtor, destroys all instruments created in the block
	virtual ~InstrumentBlock();
private:
	/// Creates and returns the instrument if it does not already exist.
	Instrument*			AcquireInstrument(Instrument::InstrumentType type, int instrumentId = 1);

	/// Returns the requested instrument
	/// returns NULL if the instrument is not found
	Instrument*			GetInstrument(Instrument::InstrumentType type, int instrumentId);


	Instrument*			CreateInstrument(Instrument::InstrumentType type, int id);
	tInstruments		mInstruments;
	NodeInterpreter*	mNodeInterpreter;
};

#endif
