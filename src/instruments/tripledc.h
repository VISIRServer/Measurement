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
 * Copyright (c) 2008 André van Schoubroeck
 * All Rights Reserved.
 */

#ifndef __TRIPLE_DC_H__
#define __TRIPLE_DC_H__

#include "instrument.h"

#include <setget.h>

// constant channel numbers
#define TRIPLEDC_25PLUS 0
#define TRIPLEDC_25MINUS 1
#define TRIPLEDC_6 2

class TripleDCChannel
{
public:
	SET_GET(double, Voltage, mVoltage)
	SET_GET(double, Current, mCurrent)
	SET_GET(double, ActualVoltage  , mActualVoltage)
	SET_GET(double, ActualCurrent  , mActualCurrent)
	SET_GET(int   , OutputEnabled  , mOutputEnabled)
	SET_GET(int   , OutputLimited  , mOutputLimited)
	void	SetMinMax(double min, double max) { mMin = min; mMax = max; }
	bool	Validate();

	TripleDCChannel();
private:
	double		mVoltage;
	double		mCurrent;
	double		mMin, mMax;
	double		mActualVoltage;
	double		mActualCurrent;
	int         mOutputEnabled;
	int			mOutputLimited;
};

class TripleDC : public Instrument
{
public:
	INSTRUMENT_TYPE(Instrument::TYPE_TripleDC)

	TripleDCChannel* GetChannel(int channelnr);

	virtual bool	IsSourceEq() const	{ return true; }
	virtual bool	IsMeasurementEq() const	{ return true; }

	virtual void	Accept(InstrumentVisitor& visitor);
	virtual bool	Validate();
	virtual bool	CheckMaxValues(const NetList2& maxlist);
	virtual void	CopyFrom(Instrument* pInstrument);

	inline TripleDCChannel&	Ch25plus()	{ return mChannels[TRIPLEDC_25PLUS]; }
	inline TripleDCChannel&	Ch25minus()	{ return mChannels[TRIPLEDC_25MINUS]; }
	inline TripleDCChannel&	Ch6()		{ return mChannels[TRIPLEDC_6]; }

	explicit TripleDC(int instrumentID);
	virtual ~TripleDC();

private:
	TripleDCChannel mChannels[3];
};

#endif
