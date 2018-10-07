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
#ifndef __DIGITAL_MULTIMETER_H__
#define __DIGITAL_MULTIMETER_H__

#include "instrument.h"

#include <setget.h>
#include <list>

/// DigitalMultimeter (DMM) instrument class.
/// Handles all DMM configuration.

class DigitalMultimeter : public Instrument
{
public:
	INSTRUMENT_TYPE(Instrument::TYPE_DigitalMultimeter)

	enum Function
	{
		DCVolts		=  0,
		ACVolts		=  1,
        DCCurrent	=  2,
        ACCurrent	=  3,
		Resistance	=  4,
        Resistance4	=  5, ///< 4-wire resistance
		Diode		=  6,
		Frequency	=  7,
		Period		=  8,
		ACvDCcoupl  =  9, ///< AC Voltage DC coupling
		Capacitance	= 10,
		Inductance  = 11,
	};

	enum Resolution
	{
		Digit3_5			= 0,
		Digit4_5			= 1,
		Digit5_5			= 2,
		Digit6_5			= 3,
	};

	enum AutoZero
	{
		Auto				= -1,
		Off					= 0,
		On					= 1,
		Once				= 2,
	};

	SET_GET(Function, Function, mFunction);

	SET_GET_STR(FunctionStr);
	SET_GET_STR(ResolutionStr);

	SET_GET(Resolution, Resolution, mResolution);
	SET_GET(double,		Range,		mRange);
	SET_GET(AutoZero,	AutoZero,	mAutoZero);
	
	SET_GET(double,		MeasureResult, mMeasureResult);

	bool				MeasuresCurrent() const;
	bool				MeasuresResistance() const;
	
	virtual bool		Validate();

	virtual void		CopyFrom(Instrument* pInstrument);

	virtual bool		IsMeasurementEq() const	{ return true; }

	virtual void		ResetData();

	/// part of the visitor pattern
	virtual void		Accept(InstrumentVisitor& visitor);

	explicit			DigitalMultimeter(int instrumentID);
	virtual				~DigitalMultimeter();
private:
	Function		mFunction;
	Resolution		mResolution;
	double			mRange;
	AutoZero		mAutoZero;

	double			mMeasureResult;
};

#endif
