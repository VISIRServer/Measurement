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

#include "gpib_signalanalyzer.h"
#include "gpib_device.h"
#include "gpib_transactions.h"
#include "gpib_log.h"

#include <instruments/signalanalyzer.h>

#include <serializer.h>
#include <syslog.h>
#include <stringop.h>
#include <math.h>

#include <windows.h>

using namespace gpib;
using namespace std;

GPIBSignalAnalyzer::GPIBSignalAnalyzer()
{
	mpPrevState	= new SignalAnalyzer(0);

	mpDevice = new GPIBDevice();

	mSuspended	= true;
	mDoReset	= false;

	mFirstAvg	= true;
	mNeedAutoRangeReset = false;
	mError = "";
}

GPIBSignalAnalyzer::~GPIBSignalAnalyzer()
{
	Destroy();

	delete mpPrevState;
	delete mpDevice;
}

bool GPIBSignalAnalyzer::Init(int boardindex, int primaddr, int secaddr)
{
	if (!mpDevice->Open(boardindex, primaddr, secaddr)) return false;

	try
	{
		ResetInstrument();
	}
	catch(BasicException e)
	{
		gpiblog.Out() << "GPIBSignalAnalyzer::Init exception: " << e.what() << endl;
		return false;
	}

	return true;
}

bool GPIBSignalAnalyzer::ResetInstrument()
{
	DoCommand("SYSTEM:PRESET");
	DoCommand("CAL:AUTO 0"); // shut off auto calibration

	// we don't want to se the average iresults
	DoCommand("AVER:IRES 1");
	DoCommand("AVER:IRES:RATE 99999");

	DoCommand("DISP:SSAV:TIME 1");
	Suspend();

	//Test();

	return true;
}

bool GPIBSignalAnalyzer::Destroy()
{
	return mpDevice->Close();
}

#define DIFF( prop ) if(pPrev->Get##prop() != pCur->Get##prop())
#define DIFF_( prop ) (pPrev->Get##prop() != pCur->Get##prop())

#define RAWDIFF( prev, cur, prop ) ( prev->Get##prop() != cur->Get##prop() )

#define COMMAND( cmd ) { Serializer out; cmd ; DoCommand(out.GetStream()); }
#define COMMAND_AVGRST( cmd ) { Serializer out; cmd ; DoCommand(out.GetStream()); NeedAvgRestart(); }

bool EqualDelta(double x, double y, double delta = 0.000001)
{
	return (fabs( x - y) < delta);
}

bool GPIBSignalAnalyzer::Measure(SignalAnalyzer* pAnalyzer)
{
	try
	{
		// check if we had an previous error
		if (mDoReset)
		{
			// create a new block (hacky reset)
			gpiblog.Out() << "GPIBSignalAnalyzer: Resetting instrument diff block" << endl;

			// this is not correct
			delete mpPrevState;
			mpPrevState = new SignalAnalyzer(0);
			// we should ignore diffing on the first measurement after this..

			mDoReset = false;
			ResetInstrument();
		}

		if (!Setup(pAnalyzer))			return false;
		if (!DoMeasurement(pAnalyzer))	return false;
		if (!FetchData(pAnalyzer))		return false;

		mFirstAvg = false;
	}
	catch(BasicException e)
	{
		gpiblog.Out() << "GPIBSignalAnalyzer::Measure exception: " << e.what() << endl;
		return false;
	}

	mLastMeasurement.restart();
	mSuspended = false;

	mpPrevState->CopyFrom(pAnalyzer);
	return true;
}

bool GPIBSignalAnalyzer::Setup(SignalAnalyzer* pCur)
{
	// diff against the previous measurement
	SignalAnalyzer* pPrev = mpPrevState;

	Serializer out;

	// FIXME: can we do the differential update or not??
	/*DIFF(Mode) */
	COMMAND( out << "INST:SEL " << pCur->GetMode() )

	DIFF(Channels)
	{
		switch(pCur->GetChannels())
		{
			case 1: DoCommand("INPUT2 OFF"); break;
			case 2: DoCommand("INPUT2 ON"); break;
			case 4: DoCommand("INPUT4 ON"); break;
			default: gpiblog.Out() << "*** Unsupported channels setting in signal analyzer" << endl; break;
		}
		NeedAvgRestart();
	}



	DIFF(Reference)	COMMAND_AVGRST( out << "REF " << pCur->GetReference() ) // check output

	DIFF(FreqStart)	COMMAND_AVGRST( out << "FREQ:START " << pCur->GetFreqStart() )
	DIFF(FreqStop)	COMMAND_AVGRST( out << "FREQ:STOP " << pCur->GetFreqStop() )
	DIFF(FreqRes)	COMMAND_AVGRST( out << "FREQ:RES " << pCur->GetFreqRes() )

	if (pCur->GetMode() == "corr")
	{
		DIFF(Blocksize) COMMAND( out << "FREQ:BLOC " << pCur->GetBlocksize() )
	}

	DIFF(WindowType) COMMAND( out << "WINDOW " << pCur->GetWindowType() ) // check output
	DIFF(WindowType) NeedAvgRestart();

	// source doesn't need avg reset
	/*DIFF(SourceOn)*/	COMMAND( out << "OUTPUT " << pCur->GetSourceOn() )
	DIFF(SourceLevel)	COMMAND( out << "SOURCE:VOLT " << pCur->GetSourceLevel() << " " << pCur->GetSourceLevelUnit() )
	DIFF(SourceOffset)	COMMAND( out << "SOURCE:VOLT:OFFSET " << pCur->GetSourceOffset() )

	string srcfunc = pCur->GetSourceFunc();
	if (srcfunc == "sin") // check this
	{
		if (DIFF_(SourceFunc) || DIFF_(SourceFreq)) COMMAND( out << "SOURCE:FUNC SIN; FREQ " << pCur->GetSourceFreq() )
	}
	else if (srcfunc == "brandom")
	{
		if (DIFF_(SourceFunc) || DIFF_(SourceBurst)) COMMAND( out << "SOURCE:FUNC BRAN; BURST " << pCur->GetSourceBurst() )
	}
	else if (srcfunc == "bchirp")
	{
		if (DIFF_(SourceFunc) || DIFF_(SourceBurst)) COMMAND( out << "SOURCE:FUNC BCH; BURST " << pCur->GetSourceBurst() )
	}
	else
	{
		DIFF(SourceFunc) COMMAND( out << "SOURCE:FUNC " << pCur->GetSourceFunc() )
	}

	DIFF(AvgOn)		COMMAND( out << "AVER " << pCur->GetAvgOn() )
	if (pCur->GetAvgOn())
	{
		if (pCur->GetAvgNum() > 0) /*DIFF(AvgNum)*/	COMMAND( out << "AVER:COUNT " << pCur->GetAvgNum() )
	}

	if (DIFF_(AvgType))
	{
		string type = pCur->GetAvgType();
		if		(type == "rms")		COMMAND( out << "AVER:TYPE RMS" )
		else if (type == "rmsexp")	COMMAND( out << "AVER:TYPE RMS; TCON EXP" )
		else if (type == "time")	COMMAND( out << "AVER:TYPE TIME" )
		else if (type == "timeexp")	COMMAND( out << "AVER:TYPE TIME; TCON EXP" )
		else if (type == "max")		COMMAND( out << "AVER:TYPE MAX" )

		NeedAvgRestart();
	}

	//DIFF(AvgType)	COMMAND( out << "AVER:TYPE " << pCur->GetAvgNum() ) // fix
	//DIFF(AvgFast)	COMMAND( out << "AVER:IRES " << pCur->GetAvgFast() )
	//DIFF(AvgRate)	COMMAND( out << "AVER:IRES:RATE " << pCur->GetAvgRate() )
	//DIFF(AvgRepeat)	COMMAND( out << "AVER:TCON " << (pCur->GetAvgRepeat() ? "FRE" : "REP") )
	DIFF(AvgOverlap)	COMMAND_AVGRST( out << "SWEEP:OVERLAP " << pCur->GetAvgOverlap() )

	// disable overload rejection for now.. it hangs the hardware
	//DIFF(AvgOvldRej)	COMMAND_AVGRST( out << "REJ:STAT " << pCur->GetAvgOvldRej() )

	// no avg reset needed here..
	DIFF(DispFormat)	COMMAND( out << "DISP:FORM " << pCur->GetDispFormat() )
	DIFF(ActiveTrace)	COMMAND( out << "CALC:ACT " << pCur->GetActiveTrace() )

	// channels
	for(int i=0;i<4;i++)
	{
		SignalAnalyzerChannel* pChPrev	= pPrev->Channel(i);
		SignalAnalyzerChannel* pChCur	= pCur->Channel(i);

		// seems like when you set the voltage range, the range mode resets
		bool doRangeMode = false;

		if ( pChPrev->GetRange() != pChCur->GetRange() || pChPrev->GetRangeUnit() != pChCur->GetRangeUnit())
		{
			// because of rounding problems, we probably don't need to set the range again
			if (!EqualDelta(pChPrev->GetRange(), pChCur->GetRange()))
			{
				COMMAND( out << "VOLT" << (i+1) << ":RANGE " << ToStringScientific(pChCur->GetRange()) << " " << pChCur->GetRangeUnit())
				doRangeMode = true;
			}
		}
		
		if (doRangeMode || ( pChPrev->GetRangeMode() != pChCur->GetRangeMode() ) )
		{
			string mode = pChCur->GetRangeMode();
			if		(mode == "fixed")	COMMAND( out << "VOLT" << (i+1) << ":RANGE:AUTO OFF" )
			else if (mode == "up")		COMMAND( out << "VOLT" << (i+1) << ":RANGE:AUTO ON; AUTO:DIR UP" )
			else if (mode == "auto")	COMMAND( out << "VOLT" << (i+1) << ":RANGE:AUTO ON; AUTO:DIR EITHER" )
			else gpiblog.Out() << "*** Unsupported channel range in signal analyzer" << endl;
			
			mNeedAutoRangeReset = false;
			NeedAvgRestart();
		}

		if (RAWDIFF(pChPrev, pChCur, Input))	COMMAND_AVGRST( out << "INP" << (i+1) << ":LOW " << pChCur->GetInput() )
		if (RAWDIFF(pChPrev, pChCur, Coupling)) COMMAND_AVGRST( out << "INP" << (i+1) << ":COUP " << pChCur->GetCoupling() )
		if (RAWDIFF(pChPrev, pChCur, AntiAlias))COMMAND_AVGRST( out << "INP" << (i+1) << ":FILTER " << pChCur->GetAntiAlias() )
		if (RAWDIFF(pChPrev, pChCur, FilterAW)) COMMAND_AVGRST( out << "INP" << (i+1) << ":FILTER:AWE " << pChCur->GetFilterAW() )
		if (RAWDIFF(pChPrev, pChCur, Bias))		COMMAND_AVGRST( out << "INP" << (i+1) << ":BIAS " << pChCur->GetBias() )

		if (RAWDIFF(pChPrev, pChCur, XDCR))		COMMAND_AVGRST( out << "VOLT" << (i+1) << ":RANG:UNIT:USER " << pChCur->GetXDCR() )

		if (pChCur->GetXDCR())
		{
            if (RAWDIFF(pChPrev, pChCur, XDCRSens))
				COMMAND_AVGRST( out << "VOLT" << (i+1) << ":RANG:UNIT:USER:SFAC " << pChCur->GetXDCRSens() << " " << pChCur->GetXDCRSensUnit())

			// it seems like the setting doesn't take if you enable the xdcr and have set the label before.. so we always do it.
			if (RAWDIFF(pChPrev, pChCur, XDCRLabel))
			{
				//sysout << "Setting: " << pChCur->GetXDCRLabel() << endl;
				COMMAND_AVGRST( out << "VOLT" << (i+1) << ":RANG:UNIT:XDCR:LAB " << pChCur->GetXDCRLabel() << "" )
			}
		}
	}

	// traces
	for(int i=0;i<4;i++)
	{
		SignalAnalyzerTrace* pTrPrev	= pPrev->Trace(i);
		SignalAnalyzerTrace* pTrCur		= pCur->Trace(i);

		// measure and channel
		if (pTrPrev->GetMeasure() != pTrCur->GetMeasure() || pTrPrev->GetChannel() != pTrCur->GetChannel())
		{
			string measure = pTrCur->GetMeasure();
			int ch = pTrCur->GetChannel();
			if		(measure == "pow")	COMMAND( out << "CALC" << (i+1) << ":FEED 'XFR:POW " << ch << "';MATH:STAT OFF;*WAI" )
			else if (measure == "lin")	COMMAND( out << "CALC" << (i+1) << ":FEED 'XFR:POW:LIN " << ch << "';MATH:STAT OFF;*WAI" )
			else if (measure == "time")	COMMAND( out << "CALC" << (i+1) << ":FEED 'XTIM:VOLT " << ch << "';MATH:STAT OFF;*WAI" )

			else if (measure == "freq")	COMMAND( out << "CALC" << (i+1) << ":FEED 'XFR:POW:RAT "
				<< DualChannelString(pCur, true) << "';MATH:STAT OFF;*WAI" )
			else if (measure == "coh")	COMMAND( out << "CALC" << (i+1) << ":FEED 'XFR:POW:COH "
				<< DualChannelString(pCur, false) << "';MATH:STAT OFF;*WAI" )
			else if (measure == "cros")	COMMAND( out << "CALC" << (i+1) << ":FEED 'XFR:POW:CROS "
				<< DualChannelString(pCur, false) << "';MATH:STAT OFF;*WAI" )
			else if (measure == "corrcros")	COMMAND( out << "CALC" << (i+1) << ":FEED 'XTIM:CORR:CROS "
				<< DualChannelString(pCur, false) << "';MATH:STAT OFF;*WAI" )

			// fix freq, coh, cros
			//else if (measure == "freq") COMMAND( out << "CALC#:FEED 'XTIM:POW:RATIO" << (i+1) << "';MATH:STAT OFF;*WAI" )
			else gpiblog.Out() << "*** Unsupported trace measure in signal analyzer" << endl;
		}

		// format
		if (pTrPrev->GetFormat() != pTrCur->GetFormat())
		{
			string format = pTrCur->GetFormat();
			if		(format == "lin")	COMMAND( out << "CALC" << (i+1) << ":FORM MLIN;:DISP:WIND" << (i+1) << ":TRAC:Y:SPAC LIN" )
			else if	(format == "log")	COMMAND( out << "CALC" << (i+1) << ":FORM MLIN;:DISP:WIND" << (i+1) << ":TRAC:Y:SPAC LOG" )
			else if (format == "db")	COMMAND( out << "CALC" << (i+1) << ":FORM MLOG" )
			else if (format == "phase")	COMMAND( out << "CALC" << (i+1) << ":FORM PHAS" )
			else if (format == "uphase")COMMAND( out << "CALC" << (i+1) << ":FORM UPH" )
			else if (format == "real")	COMMAND( out << "CALC" << (i+1) << ":FORM REAL" )
			else if (format == "imag")	COMMAND( out << "CALC" << (i+1) << ":FORM IMAG" )
			else if (format == "nyq")	COMMAND( out << "CALC" << (i+1) << ":FORM NYQ" )
			else gpiblog.Out() << "*** Unsupported trace format in signal analyzer: " << format << endl;
		}

		// x spacing
		if (pTrPrev->GetXSpacing() != pTrCur->GetXSpacing())
			COMMAND( out << "DISP:WIND" << (i+1) << ":TRAC:X:SPAC " << pTrCur->GetXSpacing() )

		// autoscale
		if (pTrPrev->GetAutoScale() != pTrCur->GetAutoScale())
			COMMAND( out << "DISP:WIND" << (i+1) << ":TRAC:Y:AUTO " << pTrCur->GetAutoScale() )

		// volt unit
		if (pTrCur->GetVoltUnit() != "" && (pTrPrev->GetVoltUnit() != pTrCur->GetVoltUnit()) )
		{
			COMMAND( out << "CALC" << (i+1) << ":UNIT:VOLT \"" << pTrCur->GetVoltUnit() << "\"" )
		}
	}

	return true;
}


bool GPIBSignalAnalyzer::DoMeasurement(SignalAnalyzer* pAnalyzer)
{
	gpiblog.Out() << "Meas type: " << pAnalyzer->GetMeasureType() << endl;

	// normal measurement
	if (!pAnalyzer->GetAvgOn())
	{
		NeedAvgRestart();
		return DoNormalMeasurement(pAnalyzer);
	}
	else
	{
		string meastype = pAnalyzer->GetMeasureType();
		if (meastype == "new" || meastype == "fetch") mFirstAvg = true;
		if (mFirstAvg) return DoFirstAvgMeasurement(pAnalyzer);
		else return DoAvgMeasurement(pAnalyzer);
	}

	return true;
}

bool GPIBSignalAnalyzer::AutoRangeReset(SignalAnalyzer* pAnalyzer)
{
	gpiblog.Out() << "AutoRangeReset" << endl;
	for(int i=0;i<4;i++)
	{
		SignalAnalyzerChannel* pChCur	= pAnalyzer->Channel(i);
		string mode = pChCur->GetRangeMode();
		if		(mode == "fixed")	COMMAND( out << "VOLT" << (i+1) << ":RANGE:AUTO OFF" )
		else if (mode == "up")		COMMAND( out << "VOLT" << (i+1) << ":RANGE:AUTO ON; AUTO:DIR UP" )
		else if (mode == "auto")	COMMAND( out << "VOLT" << (i+1) << ":RANGE:AUTO ON; AUTO:DIR EITHER" )
		else gpiblog.Out() << "*** Unsupported channel range in signal analyzer" << endl;
	}
	mNeedAutoRangeReset = false;
	return true;
}

bool GPIBSignalAnalyzer::DoNormalMeasurement(SignalAnalyzer* pAnalyzer)
{
	gpiblog.Out() << "Normal measurement" << endl;
	if (mNeedAutoRangeReset) AutoRangeReset(pAnalyzer);
	DoCommand("ABOR;:INIT; *WAI");
	return true;
}

bool GPIBSignalAnalyzer::DoFirstAvgMeasurement(SignalAnalyzer* pAnalyzer)
{
	// let the instrument do a normal measurment
	// without average, to let the auto-range settle before doing average, where we disable the auto-range
	gpiblog.Out() << "First measurement..." << endl;

	if (mNeedAutoRangeReset) AutoRangeReset(pAnalyzer);

	gpiblog.Out() << "FirstAvg: " << pAnalyzer->GetAvgOn() << " " << pAnalyzer->GetAvgNum() << endl;

	COMMAND( out << "AVER:COUNT " << 1)
	DoCommand("*WAI");
	int avgnum = pAnalyzer->GetAvgNum();
	if (avgnum == 0)
	{
		syserr << "Average count set to 0" << endl;
		avgnum = 1;
	}
	COMMAND( out << "AVER:COUNT " << avgnum )

	DoCommand("AVER 1");
	for(int i=0;i<4;i++)
	{
		COMMAND( out << "VOLT" << (i+1) << ":RANGE:AUTO OFF" )
	}

	mNeedAutoRangeReset = true;
	DoCommand("ABOR;:INIT; *WAI");
	DoCommand(":INIT:CONT OFF");

	pAnalyzer->SetAvgTotNum(pAnalyzer->GetAvgNum());
	return true;
}

bool GPIBSignalAnalyzer::DoAvgMeasurement(SignalAnalyzer* pAnalyzer)
{
	gpiblog.Out() << "Continue measurement..." << endl;

	int avgnum = pAnalyzer->GetAvgNum();
	if (avgnum == 0)
	{
		syserr << "Average count set to 0" << endl;
		avgnum = 1;
	}
	COMMAND( out << "AVER:COUNT " << avgnum)

	DoCommand("INIT:CONT ON");

	bool done = false;
	bool high = false;
	int counter = 0;
	while(!done)
	{
		counter++;
		int x = ToInt(DoAndRead("STAT:OPER:COND?"));

		int esr = ToInt(DoAndRead("*ESR?"));
		if (esr & 0x3c) // b00111100
		{
			NeedAvgRestart();
			string error = DoAndRead("SYST:ERR?");
			gpiblog.Error() << "Error continuing average: " << error 
				<< " c: " << counter << " "
				<< "X: " << ToStringbitfield(x,16) << " "
				<< "ESR: " << ToStringbitfield(esr,16) << " "
				<< endl;
			SetError(error);
			// throw?
			return false;
		}
		
		// check esr for error
		int voltcond = ToInt(DoAndRead("STAT:QUES:VOLT:COND?"));

		gpiblog.Log(3)
				<< "c: " << counter << " "
				<< "X: " << ToStringbitfield(x,16) << " "
				<< "ESR: " << ToStringbitfield(esr,16) << " "
				<< "FL: " << ToStringbitfield(voltcond,16)
				<< endl;

		if ( !( x & ((1 << 10) - 1)) )
		{
			if (high) done = true;
		}
		else
		{
			high = true;
			Sleep(100);
		}

		if (!high && counter > 10)
		{
			gpiblog.Log(3) <<"failed.." << endl;
			high = true;
		}
	}

	// update average count
	pAnalyzer->SetAvgTotNum(pAnalyzer->GetAvgTotNum() + pAnalyzer->GetAvgNum());
	gpiblog.Log(3) << "done" << endl;
	return true;
}

bool GPIBSignalAnalyzer::FetchData(SignalAnalyzer* pAnalyzer)
{
	list<int> tracelist;
	// which traces do we have to read?
	if		(pAnalyzer->GetDispFormat() == "single") tracelist.push_back(pAnalyzer->GetActiveTraceNr());
	else if (pAnalyzer->GetDispFormat() == "fback" || pAnalyzer->GetDispFormat() == "ulower")
	{
		int active = pAnalyzer->GetActiveTraceNr();
		if (active <= 1)
		{
			tracelist.push_back(0);
			tracelist.push_back(1);
		}
		else
		{
			tracelist.push_back(2);
			tracelist.push_back(3);
		}
	}
	else
	{
		syserr << "Unsupported display format" << endl;
		return false;
	}

	for(int j=0;j<4;j++)
	{
		vector<double> samples;
		pAnalyzer->Trace(j)->SetGraph(samples);
	}

	// read the result
	for(list<int>::const_iterator it = tracelist.begin(); it != tracelist.end(); it++)
	{
		DoGraph(*it, pAnalyzer);
	}

	std::string mode = pAnalyzer->GetMode();
	if (mode == "fft")
	{ // freq
		double freq_start	= ToDouble(DoAndRead("FREQ:START?"));
		double freq_stop	= ToDouble(DoAndRead("FREQ:STOP?"));

		// WARNING! converting scientific double strings to int fails!
		int freq_res		= (int)ToDouble(DoAndRead("FREQ:RES?"));

		pAnalyzer->SetFreqStart(freq_start);
		pAnalyzer->SetFreqStop(freq_stop);
		pAnalyzer->SetFreqRes(freq_res);
	}
	else if (mode == "corr")
	{
	}
	else
	{
		SetError("invalid instrument mode");
		return false;
	}


	{ // source
		double src_level	= ToDouble(DoAndRead("SOURCE:VOLT?"));
		string src_level_unit = Token(DoAndRead("SOURCE:VOLT? UNIT"),1,"\"");
		double src_offset	= ToDouble(DoAndRead("SOURCE:VOLT:OFFSET?"));
		double src_freq		= ToDouble(DoAndRead("SOURCE:FREQ?"));

		pAnalyzer->SetSourceLevel(src_level);
		pAnalyzer->SetSourceLevelUnit(src_level_unit);
		pAnalyzer->SetSourceOffset(src_offset);
		pAnalyzer->SetSourceFreq(src_freq);
		// burst?
	}

	// diode status
	int diodeflags = ToInt(DoAndRead("STAT:QUES:VOLT:COND?"));

    // channels
	for(int i=0;i<4;i++)
	{
		Serializer out;
		out << "VOLT" << (i+1) << ":RANGE?";
		double range = ToDouble(DoAndRead(out.GetStream()));
		//sysout << "range: " << range << endl;

		out.Reset();
		out << "VOLT" << (i+1) << ":RANGE? UNIT";
		string unit = Token(DoAndRead(out.GetStream()),1,"\"");
		//sysout << "unit: " << unit << endl;

		pAnalyzer->Channel(i)->SetRange(range);
		pAnalyzer->Channel(i)->SetRangeUnit(unit);

		int overload	= (diodeflags & (1 << i)) >> i; // ovld
		int half	= ( (~diodeflags >> 8) & (1 << i) ) >> i; // half

		if (!mFirstAvg && pAnalyzer->GetAvgOn()) overload |= pAnalyzer->Channel(i)->GetFlags() & 0x01;

		// if we're doing average measurements we shouldn't reset the overload flag

		pAnalyzer->Channel(i)->SetFlags( overload | (half << 1));

	}

	// traces
	//for(int i=0;i<4;i++)
	for(list<int>::const_iterator it = tracelist.begin(); it != tracelist.end(); it++)
	{
		int i = *it; //pNew->GetActiveTraceNr(); // we don't care about other than the active channel for now

		Serializer out;

		// its possible to skip these if we havn't changed the trace window settings..
		out.Reset();
		out << "DISP:WIND" << (i+1) << ":TRAC:Y:TOP?";
		double top = ToDouble(DoAndRead(out.GetStream()));

		out.Reset();
		out << "DISP:WIND" << (i+1) << ":TRAC:Y:BOTTOM?";
		double bottom = ToDouble(DoAndRead(out.GetStream()));

		out.Reset();
		out << "DISP:WIND" << (i+1) << ":TRAC:X:LEFT?";
		double left = ToDouble(DoAndRead(out.GetStream()));

		out.Reset();
		out << "DISP:WIND" << (i+1) << ":TRAC:X:RIGHT?";
		double right = ToDouble(DoAndRead(out.GetStream()));

		out.Reset();
		out << "DISP:WIND" << (i+1) << ":TRAC:Y:PDIV?";
		double pdiv = ToDouble(DoAndRead(out.GetStream()));

		//sysout << "t/b/l/r/pdiv:" << top << " " << bottom << " " << left << " " << right << " " << pdiv << endl;

		// check the output units..
		out.Reset();
		out << "DISP:WIND" << (i+1) << ":TRAC:Y:TOP? UNIT";
		string yunit = Token(DoAndRead(out.GetStream()), 1, "\"");

		out.Reset();
		out << "DISP:WIND" << (i+1) << ":TRAC:X:LEFT? UNIT";
		string xunit = Token(DoAndRead(out.GetStream()), 1, "\"");

		//sysout << "units: " << xunit << " " << yunit << endl;

		/*out.Reset();
		out << "CALC" << (i+1) << ":UNIT:VOLT?";
		sysout << DoAndRead(out.GetStream()) << endl;
		*/

		pAnalyzer->Trace(i)->SetYTop(top);
		pAnalyzer->Trace(i)->SetYBottom(bottom);
		pAnalyzer->Trace(i)->SetXLeft(left);
		pAnalyzer->Trace(i)->SetXRight(right);
		pAnalyzer->Trace(i)->SetScaleDiv(pdiv);

		pAnalyzer->Trace(i)->SetDispYUnit(yunit);
		pAnalyzer->Trace(i)->SetDispXUnit(xunit);
	}

	return true;
}

bool GPIBSignalAnalyzer::DoCommand(string command)
{
	//sysout << "GPIB Command: " << command << endl;

	LogLevel(syslog, 4) << "GPIB Command: " << command << endl;
	bool rv = mpDevice->SendCommand(command.c_str());
	if (!rv)
	{
		mDoReset = true;
		throw BasicException("GPIB Command failed");
		return false; // not reached
	}
	return rv;
}

string GPIBSignalAnalyzer::DoAndRead(string command)
{
	if (!DoCommand(command)) return false;

	//mpDevice->Wait();

	// each data element in a graph is about 20 characters
	// max graph length is 4096?
#define MAX_BUFFER_SIZE 2*65536
	char buffer[MAX_BUFFER_SIZE];
	int rv = mpDevice->ReadData(buffer, MAX_BUFFER_SIZE);
	if (!rv)
	{
		mDoReset = true;
		throw BasicException("GPIB Read failed");
		return ""; // not reached
	}
	char* cur = NULL;
	for(cur = buffer; *cur != '\n'; cur++);
	*cur = 0x00;

	return buffer;
}

bool GPIBSignalAnalyzer::DoGraph(int trace, SignalAnalyzer* pNew)
{
	int i = trace;

	//for(int i=0;i<4;i++)
	{
		Serializer out;
		out << "CALC" << (i+1) << ":DATA?";
		string data = DoAndRead(out.GetStream());
		//DoCommand("INIT:CONT 0");
		//sysout << "dump" << endl;
		//sysout << data << endl;

		size_t pos = 0;
		vector<double> samples;

		double prevvalue = 0.0;

		SignalAnalyzerTrace* pTrc = pNew->Trace(i);
		bool dolog = (pTrc->GetFormat() == "log");

		while( pos != string::npos)
		{
			size_t next = data.find_first_of(",", pos);
			string value = (next == string::npos) ? string(data, pos) : string(data, pos, next - pos);
			//sysout << value << " ";
			double outvalue = ToDouble(value);

			if(dolog && value == "+0.00000000000E+000")
			{
				//sysout << "wierd value (log)" << endl;
				outvalue = prevvalue; // safeguard
			}
			if(value == "-3.40282346639E+038")
			{
				//sysout << "wierd value" << endl;
				outvalue = prevvalue; // safeguard
			}
			samples.push_back(outvalue);
			prevvalue = outvalue;
			pos = (next == string::npos) ? string::npos : next + 1;
		}

		if (pNew->Trace(i)->GetFormat() == "nyq")
		{
			// the graph consist of x,y pairs.. we need to split and set then accordingly
			vector<double> samplesx, samplesy;
			for(size_t idx=0; idx<samples.size(); idx++)
			{
				if (idx & 1) samplesy.push_back(samples[idx]);
				else samplesx.push_back(samples[idx]);
			}

			pNew->Trace(i)->SetGraph(samplesx);
			pNew->Trace(i)->SetGraphY(samplesy);
		}
		else
		{		
			pNew->Trace(i)->SetGraph(samples);
		}
	}

	return true;
}

void GPIBSignalAnalyzer::NeedAvgRestart()
{
	mFirstAvg = true;
}

bool GPIBSignalAnalyzer::Tick()
{
	if (!mSuspended && mLastMeasurement.elapsed() > 300.0) // 5 min
	{
		gpiblog.Out() << "Suspending GPIB Signal Analyzer" << endl;
		Suspend();
	}

	return true;
}

bool GPIBSignalAnalyzer::Suspend()
{
	mSuspended = true;
	DoCommand("INIT:CONT 0");
	DoCommand("OUTPUT 0");
	//DoCommand("DISP:ENAB 0"); // shut off display
	return true;
}

string GPIBSignalAnalyzer::DualChannelString(SignalAnalyzer* pNew, bool rev)
{
	string first, second;

	int ch = pNew->Trace(pNew->GetActiveTraceNr())->GetChannel();

	if (pNew->GetReference() == "single")
	{
		first = "1";
		second = ToString( (ch > 2) ? ch : 2); 
	}
	else
	{
		if (ch > 2)
		{
			first	= "3";
			second	= "4";
		}
		else
		{
			first	= "1";
			second	= "2";
		}
	}

	Serializer out;
	if (rev)
	{
		out << second << "," << first;
	}
	else
	{
		out << first << "," << second;
	}

	return out.GetStream();
}

const char*	GPIBSignalAnalyzer::GetError()
{
	return mError.c_str();
}

void GPIBSignalAnalyzer::ResetError()
{
	mError = "";
}

void GPIBSignalAnalyzer::SetError(std::string error)
{
	mError = error;
}
