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

#include "functiongenerator.h"
#include "netlist2.h"

#include <stringop.h>
#include <math.h>

FunctionGenerator::FunctionGenerator(int instrumentID) : Instrument(Instrument::TYPE_FunctionGenerator, instrumentID)
{
	// default settings..
	mWaveForm		= Sine;
	mAmplitude		= 1.0;
	mFrequency		= 1000.0;
	mDCOffset		= 0.0;
	mPhase			= 0.0;
	mTriggerMode	= Continous;
	mTriggerSource	= Immediate;
	mDutyCycleHigh	= 0.5;
	mBurstCount		= 0;
	memset(mUserWaveform, 0, sizeof(mUserWaveform));
}

FunctionGenerator::~FunctionGenerator()
{
}

bool FunctionGenerator::Validate()
{
	if (mAmplitude < 0.0 || mAmplitude > 10.0)			return Error("Amplitude must be between 0.0 and 10.0");
	if (mFrequency > 1000000.0)							return Error("Frequency can't be higher than 1Mhz");
	//if (mWaveForm == Sine && mFrequency > 16000.0)	return Error("Sine waveform can't have higher frequency than 16khz");
	//if (mWaveForm != Sine && mFrequency > 1000.0)		return Error("Waveform can't have higher frequency than 1khz");

	if (mFrequency < 0.0)								return Error("Frequency lower than 0");
	if (mDCOffset < -5.0 || mDCOffset > 5.0)			return Error("DCOffset must be between -5 and 5");
	if (mPhase < -180.0 || mPhase > 180.0)				return Error("Phase must be between -180 and 180");

	if ( (fabs(mDCOffset) + fabs(mAmplitude)) > 5.0)	return Error("Peek (DCOffset + amplitude) can't be higher than 5");

	return true;
}

bool FunctionGenerator::CheckMaxValues(const NetList2& maxlist)
{
	typedef ListComponent::tComponentList tComps;
	const tComps& comps = maxlist.GetNodeList();

	for(tComps::const_iterator it = comps.begin(); it != comps.end(); ++it)
	{
		std::string type = it->GetType();
		
		// we should fix this so that only one node name can match a instrument
		if (type == "VDCA" || type == "VDCB" || type == "VFGENA" || type == "VFGENB")
		{
			if (it->GetSpecial() != "")
			{
				double limit = ToDouble(it->GetSpecialToken("MAX:"));

				double topvalue = fabs(GetAmplitude()) + fabs(GetDCOffset());
				if (topvalue > limit)
				{
					//cerr << "Possible maxlist match found, but fgen limit is reached (limit: " << limit << " )" << endl;
					return false;
				}
			}
		}
	}

	return true;
}


void FunctionGenerator::SetUserWaveform(double waveform[512])
{
	memcpy(mUserWaveform,waveform,sizeof(double) * 512);
}

const double* FunctionGenerator::GetUserWaveform() const
{
	return mUserWaveform;
}

void FunctionGenerator::Accept(InstrumentVisitor& visitor)
{
	visitor.Visit(*this);
}

void FunctionGenerator::CopyFrom(Instrument* pInstrument)
{
	Instrument::CopyFrom(pInstrument);
	FunctionGenerator* pFGEN = (FunctionGenerator*) pInstrument;

	mWaveForm		= pFGEN->mWaveForm;
	mAmplitude		= pFGEN->mAmplitude;
	mFrequency		= pFGEN->mFrequency;
	mDCOffset		= pFGEN->mDCOffset;
	mPhase			= pFGEN->mPhase;
	mTriggerMode	= pFGEN->mTriggerMode;
	mTriggerSource	= pFGEN->mTriggerSource;
	mBurstCount		= pFGEN->mBurstCount;

	memcpy(mUserWaveform,pFGEN->mUserWaveform, sizeof(double)*512);
}

static const char* sWaveForm[] = { "sine", "square", "triangle", "rampup",	"rampdown", "dc", "noise", "userdefined", NULL };
IMPL_SET_GET_STR(FunctionGenerator, WaveFormStr, sWaveForm, mWaveForm, WaveForm);

static const char* sTriggerMode[] = { "single", "continous", "steped", "burst", NULL };
IMPL_SET_GET_STR(FunctionGenerator, TriggerModeStr, sTriggerMode, mTriggerMode, TriggerMode)

static const char* sTriggerSource[] = { "immediate", "external", NULL };
IMPL_SET_GET_STR(FunctionGenerator, TriggerSourceStr, sTriggerSource, mTriggerSource, TriggerSource)
