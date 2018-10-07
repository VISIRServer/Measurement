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
#ifndef __OSCILLOSCOPE_H__
#define __OSCILLOSCOPE_H__

#include "instrument.h"
#include "trigger.h"
#include "measurement.h"
#include "channel.h"

// define number of channels and measurments supported
#define OSC_CHANNELS		4
#define OSC_MEASUREMENTS	3


/// Oscilloscope instrument class.
/// Holds information about the oscilloscope, channels, measurements and triggers.
class Oscilloscope : public Instrument
{
public:
	INSTRUMENT_TYPE(Instrument::TYPE_Oscilloscope)

	SET_GET(bool,		AutoScale,		mAutoScale);
	SET_GET(double,		TimeRange,		mTimeRange);

	/// Requested number of samples
	SET_GET(int,		ReqNumSamples,	mReqNumSamples);
	/// Reference position in percents
	SET_GET(double,		RefPos,			mRefPos);

	/// Get minimum sample rate
	double			GetMinSampleRate();

	void			SetSampleRate(double rate);
	virtual void	ResetData();

	/// Validate instrument state
	virtual bool	Validate();
	
	// get methods
	Trigger*		GetTriggerPointer()	const;
	Measurement*	GetMeasurementPointer(int measurement) const;
	Channel*		GetChannelPointer(int channel) const;

	Channel&		GetChannel(int chan);

	virtual bool	IsEnabled() const;

	virtual bool	IsMeasurementEq() const	{ return true; }

	/// part of the visitor pattern
	virtual void	Accept(InstrumentVisitor& visitor);

	virtual void	CopyFrom(Instrument* pInstrument);

	//				ctor/dtor
	explicit		Oscilloscope(int instrumentID);
	virtual			~Oscilloscope();
private:
	// members

	Trigger*		mTrigger;
	Channel*		mChannels[OSC_CHANNELS];
	Measurement*	mMeasurements[OSC_MEASUREMENTS];
	bool			mAutoScale;
	double			mTimeRange;
	double			mRefPos;
	int				mReqNumSamples;
};

#endif
