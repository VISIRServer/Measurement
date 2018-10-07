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

#include "oscilloscope.h"

#include <math.h>

Oscilloscope::Oscilloscope(int instrumentID) : Instrument(Instrument::TYPE_Oscilloscope, instrumentID)
{
	mTimeRange = 0.005; // possibly * 10...
	mAutoScale = false;
	mReqNumSamples = 500; // default to 500 requested number of samples
	mRefPos = 50.0;

	mTrigger = new Trigger();

	for(int i=0;i<OSC_CHANNELS;i++)		mChannels[i] = new Channel();
	for(int i=0;i<OSC_MEASUREMENTS;i++)	mMeasurements[i] = new Measurement();
}

Oscilloscope::~Oscilloscope()
{
	delete mTrigger;
	for(int i=0;i<OSC_CHANNELS;i++)		delete mChannels[i];
	for(int i=0;i<OSC_MEASUREMENTS;i++)	delete mMeasurements[i];
}

Channel& Oscilloscope::GetChannel(int chan)
{
	if (chan < 0|| chan >= OSC_CHANNELS) throw BasicException("Non-Existing channel requested");
	return *mChannels[chan];
}

Trigger* Oscilloscope::GetTriggerPointer()	const
{
	return mTrigger;
}

Measurement* Oscilloscope::GetMeasurementPointer(int measurement) const
{
	if (measurement < 0 || measurement >= OSC_MEASUREMENTS) return NULL;
	return mMeasurements[measurement];
}

Channel* Oscilloscope::GetChannelPointer(int channel) const
{
	if (channel < 0 || channel >= OSC_CHANNELS) return NULL;
	return mChannels[channel];
}

void Oscilloscope::SetSampleRate(double rate)
{
	// guard against zero or small values
	if (rate < 0.0000001) mTimeRange = 0.0000001;
	mTimeRange = 1.0 / rate;
}

double Oscilloscope::GetMinSampleRate()
{
	if (mTimeRange == 0.0) return 10000.0;
	else
	{
		// we have a precision problem here.. mTimeRange should be in 1 / x format for better precision

		if (mTimeRange < 0.001)
		{
			double t = 1.0 / mTimeRange;
			int r = (int)(t + 0.5); // round of..
			return GetReqNumSamples() * (double)r;
		}
		else
		{
			return GetReqNumSamples() / mTimeRange;
		}
	}
}

void Oscilloscope::ResetData()
{
	std::vector<char> zero;
	zero.insert(zero.begin(), GetReqNumSamples(), 0);

	mChannels[0]->SetGraph(&zero[0],zero.size(),0,0);
	mChannels[1]->SetGraph(&zero[0],zero.size(),0,0);
}

bool Oscilloscope::Validate()
{
	if (mTimeRange > 0.5)			return Error("Timerange is to long(>0.5)");

	if (mTimeRange < 0.0000001)		return Error("Timerange to short (<0.0000001)");

	if (!mTrigger->Validate())		return Error("Error in trigger");

	for(int i=0;i<OSC_CHANNELS; i++)		if (!mChannels[i]->Validate())		return Error("Error in Channel");
	for(int i=0;i<OSC_MEASUREMENTS; i++)	if (!mMeasurements[i]->Validate())	return Error("Error in Measurement");

	Trigger::TriggerSource source = mTrigger->GetSource();
	if (source == Trigger::Chan0)
	{
		double lvl = mTrigger->GetLevel() - mChannels[0]->GetVerticalOffset();

		if ( lvl > mChannels[0]->GetVerticalRange() * 4 || lvl < mChannels[0]->GetVerticalRange() * -4)
			Error("Trigger must be inside graph");
	}
	else if (source == Trigger::Chan1)
	{
		double lvl = mTrigger->GetLevel() - mChannels[1]->GetVerticalOffset();
		if ( lvl > mChannels[1]->GetVerticalRange() * 4 || lvl < mChannels[1]->GetVerticalRange() * -4)
			Error("Trigger must be inside graph");
	}

	if (mReqNumSamples < 0 || mReqNumSamples > 20000) return Error("Number of samples is out of range"); // 20000 is pulled out of a hat
	if (mRefPos < 0.0 ||  mRefPos > 100.0) return Error("Ref position is out of range");

	return true;
}

void Oscilloscope::Accept(InstrumentVisitor& visitor)
{
	visitor.Visit(*this);
}

bool Oscilloscope::IsEnabled() const
{
	// if any of the channels are enabled.. we return true
	for(int i=0;i<OSC_CHANNELS;i++)		if (mChannels[i]->GetEnabled()) return true;
	return false;
}

void Oscilloscope::CopyFrom(Instrument* pInstrument)
{
	Instrument::CopyFrom(pInstrument);
	Oscilloscope* pOsc = (Oscilloscope*) pInstrument;
	
	mTrigger->CopyFrom(pOsc->mTrigger);
	for(int i=0;i<OSC_CHANNELS;i++)			mChannels[i]->CopyFrom(pOsc->mChannels[i]);
	for(int i=0;i<OSC_MEASUREMENTS; i++)	mMeasurements[i]->CopyFrom(pOsc->mMeasurements[i]);
	
	mAutoScale = pOsc->mAutoScale;
	mTimeRange	= pOsc->mTimeRange;
}
