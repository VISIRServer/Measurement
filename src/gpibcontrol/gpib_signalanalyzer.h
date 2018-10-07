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
 * Copyright (c) 2007-2009 Johan Zackrisson
 * All Rights Reserved.
 */

#ifndef __GPIB_SIGNAL_ANALYZER_H__
#define __GPIB_SIGNAL_ANALYZER_H__

#include <timer.h>
#include <string>

class SignalAnalyzer;

namespace gpib
{

class GPIBDevice;
class GPIBTransactionHandler;

class GPIBSignalAnalyzer
{
public:
	bool	Init(int boardindex, int primaddr, int secaddr);
	bool	Measure(SignalAnalyzer* pAnalyzer);

	bool	Tick();

	const char*	GetError();
	void	ResetError();

	GPIBSignalAnalyzer();
	virtual ~GPIBSignalAnalyzer();
private:
	bool	Destroy();

	bool	Setup(SignalAnalyzer* pAnalyzer);

	bool	DoMeasurement(SignalAnalyzer* pAnalyzer);
	bool	AutoRangeReset(SignalAnalyzer* pAnalyzer);
	bool	DoNormalMeasurement(SignalAnalyzer* pAnalyzer);
	bool	DoFirstAvgMeasurement(SignalAnalyzer* pAnalyzer);
	bool	DoAvgMeasurement(SignalAnalyzer* pAnalyzer);

	bool	FetchData(SignalAnalyzer* pAnalyzer);

	void	NeedAvgRestart();

	bool	DoCommand(std::string command);
	std::string DoAndRead(std::string command);

	bool	DoGraph(int trace, SignalAnalyzer* pNew);

	bool	Suspend();
	bool	ResetInstrument();

	void	Test();

	void	SetError(std::string error);

	std::string	DualChannelString(SignalAnalyzer* pNew, bool rev);

	GPIBDevice*		mpDevice;
	SignalAnalyzer*	mpPrevState;

	timer	mLastMeasurement;
	bool	mSuspended;
	bool	mDoReset;
	bool	mFirstAvg;
	bool	mNeedAutoRangeReset;

	std::string	mError;
};

} // end of namespace

#endif
