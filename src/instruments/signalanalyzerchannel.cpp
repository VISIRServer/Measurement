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

#include "signalanalyzerchannel.h"
#include <stringop.h>

SignalAnalyzerChannel::SignalAnalyzerChannel()
{
	mRangeMode	= "auto";
	mRange		= 0.0039858;
	mRangeUnit	= "Vpk";

	mInput		= "float";
	mCoupling	= "dc";
	mAntiAlias	= 1;
	mFilterAW	= 0;
	mBias		= 0;

	mXDCR		= false;
	mXDCRSens	= 0.02;
	mXDCRSensUnit = "V/EU";
	mXDCRSensUnit = "G";
}

bool SignalAnalyzerChannel::Validate()
{
	const char* validRangeUnit[] = { "dbvrms", "vpk", "dbvpk", "v", "dbv", "eu", "dbeu", "vrms", 
		"mvpk", "mvrms", "meu", NULL }; // prefixed units..

	if (!Instrument::CheckStringValue(ToLower(mRangeUnit), validRangeUnit)) return Instrument::Error("Invalid channel range unit");

	const char* validRangeMode[] = { "fixed", "up", "auto", NULL };
	if (!Instrument::CheckStringValue(ToLower(mRangeMode), validRangeMode)) return Instrument::Error("Invalid channel range mode");

	const char* validInput[] = { "ground" , "float", NULL };
	if (!Instrument::CheckStringValue(ToLower(mInput), validInput)) return Instrument::Error("Invalid channel input");

	const char* validCoupling[] = { "ac", "dc", NULL };
	if (!Instrument::CheckStringValue(ToLower(mCoupling), validCoupling)) return Instrument::Error("Invalid channel coupling");

	const char* validXDCRSensUnit[] = { "v/eu", "eu/v", NULL };
	if (!Instrument::CheckStringValue(ToLower(mXDCRSensUnit), validXDCRSensUnit)) return Instrument::Error("Invalid channel XDCR sensitivity unit");

	const char* validXDCRLabel[] = { "pa", "g", "m/s2", "m/s", "m", "kg", "n", "dyn", "inch/s2", "inch/s", "inch", "mil", "lb", "user", NULL };
	if (!Instrument::CheckStringValue(ToLower(mXDCRLabel), validXDCRLabel)) return Instrument::Error("Invalid channel XDCR label");

	return true;
}

void SignalAnalyzerChannel::CopyFrom(SignalAnalyzerChannel* pChannel)
{
	mRange		= pChannel->mRange;
	mRangeUnit	= pChannel->mRangeMode;
	mRangeMode	= pChannel->mRangeMode;

	mInput		= pChannel->mInput;
	mCoupling	= pChannel->mCoupling;
	mAntiAlias	= pChannel->mAntiAlias;
	mFilterAW	= pChannel->mFilterAW;
	mBias		= pChannel->mBias;
	mFlags		= pChannel->mFlags;

	mXDCR			= pChannel->mXDCR;
	mXDCRSens		= pChannel->mXDCRSens;
	mXDCRSensUnit	= pChannel->mXDCRSensUnit;
	mXDCRLabel		= pChannel->mXDCRLabel;
}
