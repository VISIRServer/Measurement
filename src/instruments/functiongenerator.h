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
#ifndef __FUNCTION_GENERATOR_H__
#define __FUNCTION_GENERATOR_H__

#include "instrument.h"

#include <setget.h>

/// Function generator instrument class
class FunctionGenerator : public Instrument
{
public:
	INSTRUMENT_TYPE(Instrument::TYPE_FunctionGenerator)

	enum WaveForm
	{
		Sine		= 0,
		Square		= 1,
		Triangle	= 2,
		RampUp		= 3,
		RampDown	= 4,
		DC			= 5,
		Noise		= 6,
		UserDefined	= 7,
	};

	enum TriggerMode
	{
		Single		= 0,
		Continous	= 1,
		Steped		= 2,
		Burst		= 3,
	};

	enum TriggerSource
	{
		Immediate	= 0,
		External	= 1,
	};

	/// set/get automation
	SET_GET(WaveForm,		WaveForm,		mWaveForm);			
	SET_GET(double,			Amplitude,		mAmplitude);			///< set/get signal amplitude
	SET_GET(double,			Frequency,		mFrequency);			///< set/get signal frequency
	SET_GET(double,			DCOffset,		mDCOffset);				///< set/get signal amplitude offset
	SET_GET(double,			Phase,			mPhase);				///< set/get signal frequency offset
	SET_GET(TriggerMode,	TriggerMode,	mTriggerMode);			///< set/get which way multiple waveforms should be produced		
	SET_GET(TriggerSource,	TriggerSource,	mTriggerSource);		///< choose if the triggersource should be external of not
	SET_GET(double,			DutyCycleHigh,	mDutyCycleHigh);		///< sets the pulsewidth of a square waveform. Only valid for square. Sets the percentage that the signal should be high.
	SET_GET(int,			BurstCount,		mBurstCount);			///< how many waveform burst should be produced (requires burst mode)

	void			SetUserWaveform(double waveform[512]);			///< set user waveform (array of 512 doubles)
	const double*	GetUserWaveform() const;						///< get user waveform

	// string enum wrappers
	SET_GET_STR(WaveFormStr);
	//std::string		GetWaveFormStr();
	//void			SetWaveFormStr(std::string str);
	SET_GET_STR(TriggerModeStr);
	//std::string		GetTriggerModeStr();
	//void			SetTriggerModeStr(std::string str);
	SET_GET_STR(TriggerSourceStr);
	//std::string		GetTriggerSourceStr();
	//void			SetTriggerSourceStr(std::string str);

	virtual bool	IsSourceEq() const	{ return true; }

	/// part of the visitor pattern
	virtual void	Accept(InstrumentVisitor& visitor);

	virtual bool	Validate();										///< validate instrument state
	virtual bool	CheckMaxValues(const NetList2& maxlist);
	
	virtual void	CopyFrom(Instrument* pInstrument);

	explicit FunctionGenerator(int instrumentID);
	virtual ~FunctionGenerator();
private:
	//				Instrument settings
	WaveForm		mWaveForm;
	double			mAmplitude;
	double			mFrequency;
	double			mDCOffset;
	double			mPhase;
	TriggerMode		mTriggerMode;
	TriggerSource	mTriggerSource;
	double			mDutyCycleHigh;
	int				mBurstCount;
	double			mUserWaveform[512];	///< user defined waveform
};

#endif
