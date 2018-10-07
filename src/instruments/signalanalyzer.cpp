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

#include "signalanalyzer.h"

#include <stringop.h>

SignalAnalyzer::SignalAnalyzer(int instrumentID) : Instrument(Instrument::TYPE_SignalAnalyzer, instrumentID)
{
	mInstChannels	= 2;
	mInstRef		= "single";
	mInstMode		= "fft";

	mFreqStart	= 0.0;
    mFreqStop	= 51200.0;
	mFreqRes	= 400;

	mBlocksize = 256;

	mWindowType	= "hanning";

	mSourceOn			= 0;
	mSourceLevel		= 0.0;
	mSourceLevelUnit	= "Vpk";
	mSourceOffset		= 0.0;
	mSourceFunc			= "sin";
    mSourceFreq			= 10240.0;
	mSourceBurst		= 50.0;

	mAvgOn		= 0;
	mAvgNum		= 10;
	/*
	mAvgFast	= 0;
	mAvgRate	= 5;
	mAvgRepeat	= 0;
	*/
	mAvgOverlap = 0.0;
	mAvgType	= "rms";
	mAvgOvldRej	= 0;
	mAvgTotNum	= 0;
	
	mDispFormat	= "single";
	mActiveTrace = "a";

	mMeasureType = "new";

	mTraces[0].SetChannel(1);
	mTraces[1].SetChannel(2);
	mTraces[2].SetChannel(1);
	mTraces[3].SetChannel(2);

	mTraces[0].SetMeasure("pow");
	mTraces[1].SetMeasure("pow");
	mTraces[2].SetMeasure("time");
	mTraces[3].SetMeasure("time");

	mTraces[0].SetFormat("log");
	mTraces[1].SetFormat("log");
	mTraces[2].SetFormat("real");
	mTraces[3].SetFormat("real");
}

SignalAnalyzer::~SignalAnalyzer()
{
}

int SignalAnalyzer::GetActiveTraceNr()
{
	if (mActiveTrace.size() > 1) return 0;
	int rv = ToLower(mActiveTrace)[0] - 'a';
	if (rv < 0 || rv > 3) throw BasicException("Active trace is outside range");
	return rv;
}

void SignalAnalyzer::Accept(InstrumentVisitor& visitor)
{
	visitor.Visit(*this);
}

bool CheckStringValue(std::string value, const char* array[])
{
	for(int i=0; array[i]; i++)	if (value == array[i]) return true;
	return false;
}

bool SignalAnalyzer::Validate()
{
	if ( (mInstChannels != 1) && (mInstChannels != 2) && (mInstChannels != 4) ) return Error("InstChannels must be either 1,2 or 4");

	const char* validInstRef[] = { "single", "pair", NULL };
	if (!CheckStringValue(mInstRef, validInstRef)) return Error("Invalid inst ref");

	const char* validInstMode[] = { "fft", "corr", NULL };
	if (!CheckStringValue(mInstMode, validInstMode)) return Error("Invalid inst mode");

	const char* validWindowType[] = { "hanning", "flattop", "uniform", "llag", "lag", NULL };
	if (!CheckStringValue(mWindowType, validWindowType)) return Error("Invalid window type");

	const char* validSourceLevelUnit[] = { "dbvrms", "vpk", "dbvpk", "v", "dbv", "vrms", NULL };
	if (!CheckStringValue(ToLower(mSourceLevelUnit), validSourceLevelUnit)) Error("Invalid source level unit");

	// bran and brandom are aliases, same for bch and bchirp
	const char* validSourceFunc[] = { "random" , "pchirp" , "pink" , "sin" , "bran" , "bch" , "brandom", "bchirp", NULL };
	if (!CheckStringValue(ToLower(mSourceFunc) , validSourceFunc)) return Error("Invalid source function");

	const char* validAvgType[] = { "rms", "rmsexp", "time", "timeexp", "max", NULL };
	if (!CheckStringValue(ToLower(mAvgType), validAvgType)) return Error("Invalid average type");

	const char* validDispFormat[] = { "single", "fback", "ulower", NULL };
	if (!CheckStringValue(ToLower(mDispFormat), validDispFormat)) return Error("Invalid display format");

	const char* validMeasureType[] = { "new", "fetch", "continue", NULL };
	if (!CheckStringValue(ToLower(mMeasureType), validMeasureType)) return Error("Invalid measurement type");

	// check levels and context dependent settings

	if (mSourceLevel > 0.8) return Error("Source level are restricted to under 0.8");

	if (mFreqStart < 0.0 || mFreqStart > 115000.0)		return Error("Freq start is outside range");
	if (mFreqStop < 0.03125 || mFreqStop > 115000.0)	return Error("Freq stop outside range");
	if (mFreqStop < mFreqStart + 0.03125)				return Error("Freq span is to small");

	if (mBlocksize < 256 || mBlocksize > 2048)			return Error("Block size outside bounds");

	if (mInstChannels == 1)
	{
		std::string measure = mTraces[GetActiveTraceNr()].GetMeasure();
		if (measure == "freq" || measure == "coh" || measure == "cros")
			return Error("Selected measurement is not valid in one channel mode");
	}

	if (mTraces[GetActiveTraceNr()].GetMeasure() == "coh")
	{
		if (!mAvgOn || ( mAvgType != "rms" && mAvgType != "rmsexp") )
			return Error("When measuring coherence you must have average enabled and either rms or rmsexp average type");
	}

	if (mAvgOn && mAvgNum > 50) return Error("Average number must be less than 50");

	for(int i=0;i<4;i++) if (!mChannels[i].Validate()) return false;
	for(int i=0;i<4;i++) if (!mTraces[i].Validate()) return false;

	return true;
}

void SignalAnalyzer::CopyFrom(Instrument* pInstrument)
{
	Instrument::CopyFrom(pInstrument);

	SignalAnalyzer* pAnal = (SignalAnalyzer*) pInstrument;

	mInstChannels	= pAnal->mInstChannels;
	mInstRef		= pAnal->mInstRef;

	mFreqStart		= pAnal->mFreqStart;
	mFreqStop		= pAnal->mFreqStop;
	mFreqRes		= pAnal->mFreqRes;

	mWindowType		= pAnal->mWindowType;

	for(int i=0;i<4;i++) mChannels[i].CopyFrom(&pAnal->mChannels[i]);
	for(int i=0;i<4;i++) mTraces[i].CopyFrom(&pAnal->mTraces[i]);

	mSourceOn		= pAnal->mSourceOn;
	mSourceLevel	= pAnal->mSourceLevel;
	mSourceLevelUnit= pAnal->mSourceLevelUnit;
	mSourceOffset	= pAnal->mSourceOffset;
	mSourceFunc		= pAnal->mSourceFunc;
	mSourceFreq		= pAnal->mSourceFreq;
	mSourceBurst	= pAnal->mSourceBurst;

	mAvgOn			= pAnal->mAvgOn;
	mAvgNum			= pAnal->mAvgNum;
	/*
	mAvgFast		= pAnal->mAvgFast;
	mAvgRate		= pAnal->mAvgRate;
	mAvgRepeat		= pAnal->mAvgRepeat;
	*/
	mAvgOverlap		= pAnal->mAvgOverlap;
	mAvgType		= pAnal->mAvgType;
	mAvgOvldRej		= pAnal->mAvgOvldRej;
	mAvgTotNum		= pAnal->mAvgTotNum;

	mDispFormat		= pAnal->mDispFormat;
	mActiveTrace	= pAnal->mActiveTrace;

	mMeasureType	= pAnal->mMeasureType;
}

SignalAnalyzerChannel* SignalAnalyzer::Channel(int channr)
{
	if (channr >= 0 && channr <= 3) return &mChannels[channr];
	else return NULL;
}

SignalAnalyzerTrace* SignalAnalyzer::Trace(int tracenr)
{
	if (tracenr >= 0 && tracenr <= 3) return &mTraces[tracenr];
	else return NULL;
}
