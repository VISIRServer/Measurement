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
#ifndef __MEASUREMENT_H__
#define __MEASUREMENT_H__

#include <setget.h>

#include <string>

// forward decl.
class Oscilloscope;

/// Oscilloscope measurement configuration class.
/// Holds and manipulates information about the oscilloscope measurement configuration.
/// Aggregate class of oscilloscope.
class Measurement
{
public:
	enum MeasurementChannel
	{
		Chan0	= 0,
		Chan1	= 1,
	};

	enum MeasurementSelection
	{
		ACEstimate				= 1012,
		Area					= 1003,
		AverageFrequency		= 1016,
		AveragePeriod			= 1015,
		CycleArea				= 1004,
		DCEstimate				= 1013,
		FallTime				= 1,
		FallingSlewRate			= 1011,
		FFTAmplitude			= 1009,
		FFTFrequency			= 1008,
		Frequency				= 2,
		Integral				= 1005,
		NegativeDutyCycle		= 13,
		NegativeWidth			= 11,
		None					= 4000, /// default
		Overshoot				= 18,
		Period					= 3,
		PhaseDelay				= 1018,
		PositiveDutyCycle		= 14,
		PositiveWidth			= 12,
		Preshoot				= 19,
		RiseTime				= 0,
		RisingSlewRate			= 1010,
		TimeDelay				= 1014,
		VoltageAmplitude		= 15,
		VoltageAverage			= 10,
		VoltageBase				= 1006,
		VoltageBaseToTop		= 1017,
		VoltageCycleAverage		= 17,
		VoltageCycleRMS			= 16,
		VoltageHigh				= 8,
		VoltageLow				= 9,
		VoltageMax				= 6,
		VoltageMin				= 7,
		VoltagePeakToPeak		= 5,
		VoltageRMS				= 4,
		VoltageTop				= 1007,
	};

	SET_GET(MeasurementChannel,		Channel,		mChannel);
	SET_GET(MeasurementSelection,	Selection,		mSelection);

	SET_GET_STR(ChannelStr);
	SET_GET_STR(SelectionStr);

	double		GetMeasureResult();
	void		SetMeasureResult(double result);

	bool Validate();
	void CopyFrom(Measurement* pMeasurement);

	Measurement();
	virtual ~Measurement();
private:
	MeasurementChannel		mChannel;
	MeasurementSelection	mSelection;
	double					mMeasureValue;
};

#endif
