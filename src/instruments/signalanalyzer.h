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

#ifndef __SIGNALANALYZER_H__
#define __SIGNALANALYZER_H__

#include "instrument.h"
#include "signalanalyzerchannel.h"
#include "signalanalyzertrace.h"

#include <string>

class SignalAnalyzer : public Instrument
{
public:
	INSTRUMENT_TYPE(Instrument::TYPE_SignalAnalyzer)

	SET_GET(int,			Channels,		mInstChannels);
	SET_GET(std::string,	Reference,		mInstRef);
	SET_GET(std::string,	Mode,			mInstMode);

	SET_GET(double, FreqStart,		mFreqStart);
	SET_GET(double, FreqStop,		mFreqStop);
	SET_GET(int,	FreqRes,		mFreqRes);

	SET_GET(int,	Blocksize,		mBlocksize);

	SET_GET(std::string,	WindowType,		mWindowType);

	SET_GET(int,			SourceOn,		mSourceOn);
	SET_GET(double,		SourceLevel,	mSourceLevel);
	SET_GET(std::string,SourceLevelUnit,mSourceLevelUnit);
	SET_GET(double,		SourceOffset,	mSourceOffset);
	SET_GET(std::string,SourceFunc,		mSourceFunc);
	SET_GET(double,		SourceFreq,		mSourceFreq);
	SET_GET(double,		SourceBurst,	mSourceBurst);

	SET_GET(int,		AvgOn,			mAvgOn);
	SET_GET(int,		AvgNum,			mAvgNum); // number of NEW averages to take
	SET_GET(double,		AvgOverlap,		mAvgOverlap);
	SET_GET(std::string,AvgType,		mAvgType);
	SET_GET(int,		AvgOvldRej,		mAvgOvldRej);

	SET_GET(int,		AvgTotNum,		mAvgTotNum); // return value only, keeps track of how many averages that has been taken

	SET_GET(std::string,DispFormat,		mDispFormat);
	SET_GET(std::string,ActiveTrace,	mActiveTrace);

	SET_GET(std::string, MeasureType,	mMeasureType);

	int		GetActiveTraceNr();

	SignalAnalyzerChannel*	Channel(int channr);
	SignalAnalyzerTrace*	Trace(int tracenr);

	virtual bool	IsMeasurementEq() const	{ return true; }
	virtual bool	IsSourceEq() const	{ return true; }

	virtual void	Accept(InstrumentVisitor& visitor);
	virtual bool	Validate();
	virtual void	CopyFrom(Instrument* pInstrument);

	virtual bool	IsEnabled() const	{ return true;	} // not sure this is ok..


	explicit SignalAnalyzer(int instrumentID);
	virtual ~SignalAnalyzer();
private:
	// inst
	int			mInstChannels;
	std::string	mInstRef;
	std::string	mInstMode;

	// freq
	double	mFreqStart, mFreqStop;
	int		mFreqRes;

	int		mBlocksize;

	// window
	std::string	mWindowType;

	// channels
	SignalAnalyzerChannel mChannels[4];

	// traces
	SignalAnalyzerTrace mTraces[4];

	// source
	int			mSourceOn;
	double		mSourceLevel;
	std::string	mSourceLevelUnit;
	double		mSourceOffset;
	std::string	mSourceFunc;
	double		mSourceFreq;
	double		mSourceBurst;

	// avg
	int			mAvgOn;
	int			mAvgNum;
	double		mAvgOverlap;
	std::string	mAvgType;
	int			mAvgOvldRej;
	int			mAvgTotNum;
	
	std::string	mDispFormat;
	std::string	mActiveTrace;

	std::string	mMeasureType;
};

#endif
