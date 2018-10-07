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

#include "signalanalyzertrace.h"
#include <stringop.h>

SignalAnalyzerTrace::SignalAnalyzerTrace()
{
	mChannel	= 1;
	mMeasure	= "pow";
	mFormat		= "log";
	mXSpacing	= "lin";
	mAutoScale	= 0;
	mScale		= "range";
	mScaleDiv	= 6; // ?
	mVoltUnit	= "V";

	mYTop		= 0.0;
	mYBottom	= 0.0;
	mXLeft		= 0.0;
	mXRight		= 0.0;

	mDispXUnit	= "HZ";
	mDispYUnit	= "VRMS";
}

bool SignalAnalyzerTrace::Validate()
{
	if ( (mChannel != 1) && (mChannel != 2) && (mChannel != 4) ) return Instrument::Error("Trace Channel must be either 1,2 or 4");

	const char* validMeasure[] = { "pow", "lin", "time", "freq", "coh", "cros", "corrcros", NULL };
	if (!Instrument::CheckStringValue(ToLower(mMeasure), validMeasure)) return Instrument::Error("Invalid trace measurement");

	const char* validFormat[] = { "lin", "log", "db", "phase", "uphase", "real", "imag", "nyq", NULL };
	if (!Instrument::CheckStringValue(ToLower(mFormat), validFormat)) return Instrument::Error("Invalid trace format");

	const char* validXSpacing[] = { "lin", "log", NULL };
	if (!Instrument::CheckStringValue(ToLower(mXSpacing), validXSpacing)) return Instrument::Error("Invalid trace x spacing");

	const char* validVoltUnit[] = { "", "v", "v2", "v/rthz", "v2/hz", "v2s/hz", NULL };
	if (!Instrument::CheckStringValue(ToLower(mVoltUnit), validVoltUnit)) return Instrument::Error("Invalid trace volt unit");

	return true;
}

void SignalAnalyzerTrace::CopyFrom(SignalAnalyzerTrace* pTrace)
{
	mChannel	= pTrace->mChannel;
	mMeasure	= pTrace->mMeasure;
	mFormat		= pTrace->mFormat;
	mXSpacing	= pTrace->mXSpacing;
	mAutoScale	= pTrace->mAutoScale;
	mScale		= pTrace->mScale;
	mScaleDiv	= pTrace->mScaleDiv;
	mVoltUnit	= pTrace->mVoltUnit;

	// needed? output params
	mYTop		= pTrace->mYTop;
	mYBottom	= pTrace->mYBottom;
	mXLeft		= pTrace->mXLeft;
	mXRight		= pTrace->mXRight;
	mGraph		= pTrace->mGraph;
	mGraphY		= pTrace->mGraphY;
	mDispXUnit	= pTrace->mDispXUnit;
	mDispYUnit	= pTrace->mDispYUnit;
}

