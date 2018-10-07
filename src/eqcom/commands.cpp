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
 * Copyright (c) 2008-2009 Johan Zackrisson
 * Copyright (c) 2008 André van Schoubroeck
 * All Rights Reserved.
 */

#include "commands.h"
#include "experiment.h"
#include "header.h"
#include "symbols.h"

#include "eqlog.h"

#include <instruments/instrumentblock.h>
#include <instruments/oscilloscope.h>
#include <instruments/digitalmultimeter.h>
#include <instruments/functiongenerator.h>
#include <instruments/nodeinterpreter.h>
#include <instruments/tripledc.h>

#include <serializer.h>
#include <stringop.h>

#include <string>
using namespace std;

#include <contrib/base64.h>

//#include <float.h>
#include <math.h>
#include <limits>

using namespace EqSrv;

// hardcoded number of channels
#define HC_OSC_CHANNELS 2

void InstrumentCommands::OscilloscopeSetup(ostream& out, Oscilloscope& osc)
{
	out << InstrumentHead(osc);
	out << "0 " << osc.GetAutoScale()
		<< " " << osc.GetMinSampleRate()
		<< " " << osc.GetRefPos()
		<< " " << osc.GetReqNumSamples();

	for(int i=0;i<HC_OSC_CHANNELS;i++)	OscilloscopeChannelSetup(out, osc.GetChannelPointer(i));
	OscilloscopeTriggerSetup(out, osc.GetTriggerPointer());
	for(int i=0;i<OSC_MEASUREMENTS;i++)	OscilloscopeMeasureSetup(out, osc.GetMeasurementPointer(i));

	out << endtoken;
}

void InstrumentCommands::OscilloscopeFetch(std::ostream& out, Oscilloscope& osc)
{
	out << InstrumentHead(osc);
	out << "1" << endtoken;
}

void InstrumentCommands::OscilloscopeTriggerSetup(std::ostream& out, Trigger* trigger)
{
	out << " " << trigger->GetSource()
		<< " " << trigger->GetSlope()
		<< " " << trigger->GetCoupling()
		<< " " << trigger->GetLevel()
		<< " " << 0.0 //trigger->GetHoldOff()
		<< " " << trigger->GetDelay()
		<< " " << trigger->GetMode()
		<< " " << trigger->GetTimeout();
}

void InstrumentCommands::OscilloscopeChannelSetup(std::ostream& out, Channel* chan)
{
	// for now the channel is always enabled..
	out << " " << 1;
	
	//out << " " << chan->GetEnabled();
	//if (chan->GetEnabled())
	{
		out	<< " " << chan->GetVerticalCoupling()
			<< " " << (chan->GetVerticalRange())
			<< " " << chan->GetVerticalOffset()
			<< " " << chan->GetProbeAttenuation();
	}
}

void InstrumentCommands::OscilloscopeMeasureSetup(std::ostream& out, Measurement* measure)
{
	out << " " << measure->GetChannel() << " " << measure->GetSelection();
}

void InstrumentCommands::DigitalMultimeterFetch(std::ostream& out, DigitalMultimeter& dmm)
{
	// <Function><Nodes><Resolution><Range><Autozero>

	out << InstrumentHead(dmm);

	// new protocol
	out << "0 "; // 0 == Measure
	out << dmm.GetFunction() << " " << dmm.GetResolution() << " " << dmm.GetRange() << " " << dmm.GetAutoZero() << endtoken;
	cout << "dmm res: " << dmm.GetResolution() << " range: " << dmm.GetRange() << " az: " << dmm.GetAutoZero() << endl;
}

void InstrumentCommands::FunctionGeneratorSetup(std::ostream& out, FunctionGenerator& funcgen)
{
	//mSerializer << (funcgen.GetID()) << "%";
	out << InstrumentHead(funcgen);

	// new protocol
	out << "0 "; // 0 == Setup

	out	<< funcgen.GetWaveForm() << " "
		<< funcgen.GetAmplitude() << " "
		<< funcgen.GetFrequency() << " "
		<< funcgen.GetDCOffset() << " "
		<< funcgen.GetPhase() << " "
		<< funcgen.GetTriggerMode() << " "
		<< funcgen.GetTriggerSource() << " "
		<< funcgen.GetBurstCount() << " "
		<< funcgen.GetDutyCycleHigh()
		;

	if (funcgen.GetWaveForm() == FunctionGenerator::UserDefined)
	{
		for(int i=0;i<512;i++)
		{
			out << " " << funcgen.GetUserWaveform()[i]; // should this doubles or not?
		}
	}
	else
	{
		out << " 0";
	}

	out << endtoken;
}

void InstrumentCommands::TripleDCSetup(std::ostream& out, TripleDC& tripledc, NodeInterpreter* pNodeIntr)
{
	bool doAll = false;
	//if (V3Service::Instance()->IsMatrixDisabled()) doAll = true;

	bool v25p	= doAll;
	bool v25m	= doAll;
	bool v6		= doAll;

	if (pNodeIntr)
	{
		v25p	|= pNodeIntr->TripleDCConnection(tripledc, TRIPLEDC_25PLUS).IsConnected();
		v25m	|= pNodeIntr->TripleDCConnection(tripledc, TRIPLEDC_25MINUS).IsConnected();
		v6		|= pNodeIntr->TripleDCConnection(tripledc, TRIPLEDC_6).IsConnected();
	}

	bool enabled = v25p || v25m || v6;

	out << InstrumentHead(tripledc);
	out << "0 "; // setup
	out << enabled;

	TripleDCChannel* ch6 = tripledc.GetChannel(TRIPLEDC_6);
	if (v6)
	{
		out << " " << ch6->GetOutputEnabled()
		<< " " << ch6->GetVoltage()
		<< " " << ch6->GetCurrent();
	}
	else		out << " 0 0.0 0.0";

	TripleDCChannel* ch25p = tripledc.GetChannel(TRIPLEDC_25PLUS); 
	if (v25p)
	{
		out << " " << ch25p->GetOutputEnabled()
			<< " " << ch25p->GetVoltage()
			<< " " << ch25p->GetCurrent();
	}
	else		out << " 0 0.0 0.0";

	TripleDCChannel* ch25m = tripledc.GetChannel(TRIPLEDC_25MINUS);
	if (v25m)
	{
		out << " " << ch25m->GetOutputEnabled()
			<< " " << ch25m->GetVoltage()
			<< " " << (-ch25m->GetCurrent()); // labview control is insane.. you need to invert the current limit
	}
	else		out << " 0 0.0 0.0";
	out << endtoken;
}

void InstrumentCommands::CircuitFetch(std::ostream& out)
{
	out << InstrumentHeadType(V3_CircuitBuilder) << "4" << endtoken; // check this
}

void InstrumentCommands::ExtDelay(std::ostream& out, int timeinms)
{
	out << InstrumentHeadType(V3_ExtendedPeripherals) << "0 " << timeinms << endtoken;
}

void InstrumentCommands::ExtResetAll(std::ostream& out)
{
	out << InstrumentHeadType(V3_ExtendedPeripherals) << "3 " << endtoken;
}

void InstrumentCommands::TripleDCFetch(std::ostream& out, TripleDC& tripledc)
{
	out << InstrumentHead(tripledc);
	out << "1" << endtoken;
}

////////////////////////////////

/*void InstrumentResponses::OscilloscopeSetup(Serializer& in, Oscilloscope& osc)
{
}*/

void InstrumentResponses::OscilloscopeFetch(Serializer& in, Oscilloscope& osc)
{
	int actualsamples = 0;
	double actualsamplerate = 0.0;

	int func = 0;
	in.GetInteger(func, " \t");
	if (func == 0) return;

	int notimeout = 0;
	in.GetInteger(notimeout, " ");
	if (!notimeout) return;

	in.GetDouble(actualsamplerate, " ");
	in.GetInteger(actualsamples, " ");

	int numchannels = 0;
	in.GetInteger(numchannels, " ");

	for(int i=0;i<numchannels;i++)
	{
		double offset = 0.0;
		double gain = 0.0;

		double verticalRange = 0.0;
		double probeAttenuation = 0.0;

		int channel = 0;
		in.GetInteger(channel, " ");

		if (channel < 0 || channel >= 2) throw BasicException("Channel out of range");

		in.GetDouble(probeAttenuation, " ");
		in.GetDouble(verticalRange, " ");
		in.GetDouble(offset," ");
		in.GetDouble(gain, " ", false);

		std::string base64graph;
		in.GetString(base64graph, " ");

		if (actualsamples > 20000) throw BasicException("Graph contains to many samples");

		string outdata = base64::base64_decode(base64graph);
		int outlen = outdata.size();
		const char* buffer = outdata.c_str();

		if (outlen != actualsamples) throw BasicException("Graph length and actual samples doesn't match");
		osc.GetChannelPointer(channel)->SetGraph(buffer, actualsamples, gain /*, offset*/);
	}

	for(int i=0;i<3;i++) // hardcoded number of measurements
	{
		double measurement = 0.0;
		in.GetDouble(measurement, " ");
		osc.GetMeasurementPointer(i)->SetMeasureResult(measurement);
	}

	int triggerReceived = 0;
	double newTriggerLevel = 0.0;

	in.GetInteger(triggerReceived, " ");
	in.GetDouble(newTriggerLevel, " \n"); // last read before end..

	eqlog.Log(5) << "Trigger levels: " << triggerReceived << " "
		<< osc.GetTriggerPointer()->GetLevel() << " " << newTriggerLevel << endl;

	osc.GetTriggerPointer()->SetTriggerReceived(triggerReceived == 1);
	osc.GetTriggerPointer()->SetLevel(newTriggerLevel);

	// dirty temporary fix for "no trigger" case..
	// move this to be a attribute in the trigger instead and return real values to the client
	/*if (osc.GetTriggerPointer()->GetMode() == Trigger::Normal && !triggerReceived)
	{
		throw BasicException("no trigger");
	}*/
}

#ifdef USE_ISTREAM

void InstrumentResponses::OscilloscopeFetch(istream& in, Oscilloscope& osc)
{
	string line;
	getline(in, line);
	stringstream ss(line);

	int func = 0;
	in >> func;
	if (func == 0) return;

	int notimeout = 0;
	in >> notimeout;
	if (!notimeout) return;

	double actualsamplerate = 0.0;
	int actualsamples = 0;

	in >> actualsamplerate >> actualsamples;

	if (actualsamples > 20000) throw BasicException("Graph contains to many samples");

	int numchannels = 0;
	in >> numchannels;

	for(int i=0;i<numchannels;i++)
	{
		double offset = 0.0;
		double gain = 0.0;

		double verticalRange = 0.0;
		double probeAttenuation = 0.0;

		int channel = 0;
		in >> channel;
		if (channel < 0 || channel >= 2) throw BasicException("Channel out of range");

		in >> probeAttenuation >> verticalRange >> offset >> gain;

		std::string base64graph;
		in >> base64graph;

		string outdata = base64::base64_decode(base64graph);
		int outlen = outdata.size();
		const char* buffer = outdata.c_str();

		if (outlen != actualsamples) throw BasicException("Graph length and actual samples doesn't match");
		osc.GetChannelPointer(channel)->SetGraph(buffer, actualsamples, gain /*, offset*/);
	}

	for(int i=0;i<3;i++) // hardcoded number of measurements
	{
		double measurement = 0.0;
		in >> measurement;
		osc.GetMeasurementPointer(i)->SetMeasureResult(measurement);
	}

	int triggerReceived = 0;
	double newTriggerLevel = 0.0;

	in >> triggerReceived >> newTriggerLevel;

	eqlog.Log(5) << "Trigger levels: " << triggerReceived << " "
		<< osc.GetTriggerPointer()->GetLevel() << " " << newTriggerLevel << endl;

	osc.GetTriggerPointer()->SetTriggerReceived(triggerReceived == 1);
	osc.GetTriggerPointer()->SetLevel(newTriggerLevel);

	// dirty temporary fix for "no trigger" case..
	// move this to be a attribute in the trigger instead and return real values to the client
	/*if (osc.GetTriggerPointer()->GetMode() == Trigger::Normal && !triggerReceived)
	{
		throw BasicException("no trigger");
	}*/
}

#endif


void InstrumentResponses::DigitalMultimeterFetch(Serializer& in, DigitalMultimeter& dmm)
{
	int func = 0;
	in.GetInteger(func, " \t");

	string result;
	in.GetString(result,"\n");

	string measure = Token(result,0);
	double fresult = 0.0;
	if (measure == "NaN") fresult = std::numeric_limits<double>::quiet_NaN();
	else fresult = atof(measure.c_str());

	dmm.SetMeasureResult(fresult);
}

#ifdef USE_ISTREAM

void InstrumentResponses::DigitalMultimeterFetch(istream& in, DigitalMultimeter& dmm)
{
	string line;
	getline(in, line);
	stringstream ss(line);
	int func = 0;
	in >> func;

	string result;
	in >> result;

	string measure = Token(result,0);
	double fresult = 0.0;
	if (measure == "NaN") fresult = std::numeric_limits<double>::quiet_NaN();
	else fresult = atof(measure.c_str());

	dmm.SetMeasureResult(fresult);
}

#endif

void InstrumentResponses::TripleDCFetch(Serializer& in, TripleDC& tripledc)
{
	double Ch0V,Ch0A,Ch1V,Ch1A,Ch2V,Ch2A;
	int Ch0L,Ch1L,Ch2L;
	
	in.GetDouble(Ch0V," ");
	in.GetDouble(Ch0A," ");
	in.GetInteger(Ch0L," ");

	in.GetDouble(Ch1V," ");
	in.GetDouble(Ch1A," ");
	in.GetInteger(Ch1L," ");

	in.GetDouble(Ch2V," ");
	in.GetDouble(Ch2A," ");
	in.GetInteger(Ch2L," ");

	tripledc.GetChannel(TRIPLEDC_6)       -> SetActualVoltage(Ch0V);
	tripledc.GetChannel(TRIPLEDC_25PLUS)  -> SetActualVoltage(Ch1V);
	tripledc.GetChannel(TRIPLEDC_25MINUS) -> SetActualVoltage(Ch2V);
	tripledc.GetChannel(TRIPLEDC_6)       -> SetActualCurrent(Ch0A);
	tripledc.GetChannel(TRIPLEDC_25PLUS)  -> SetActualCurrent(Ch1A);
	tripledc.GetChannel(TRIPLEDC_25MINUS) -> SetActualCurrent(Ch2A);
	tripledc.GetChannel(TRIPLEDC_6)       -> SetOutputLimited(Ch0L);
	tripledc.GetChannel(TRIPLEDC_25PLUS)  -> SetOutputLimited(Ch1L);
	tripledc.GetChannel(TRIPLEDC_25MINUS) -> SetOutputLimited(Ch2L);
}

#ifdef USE_ISTREAM

void InstrumentResponses::TripleDCFetch(istream& in, TripleDC& tripledc)
{
	string line;
	getline(in, line);
	stringstream ss(line);

	double Ch0V,Ch0A,Ch1V,Ch1A,Ch2V,Ch2A;
	int Ch0L,Ch1L,Ch2L;

	ss >> Ch0V >> Ch0A >> Ch0L;
	ss >> Ch1V >> Ch1A >> Ch1L;
	ss >> Ch2V >> Ch2A >> Ch2L;

	tripledc.GetChannel(TRIPLEDC_6)       -> SetActualVoltage(Ch0V);
	tripledc.GetChannel(TRIPLEDC_25PLUS)  -> SetActualVoltage(Ch1V);
	tripledc.GetChannel(TRIPLEDC_25MINUS) -> SetActualVoltage(Ch2V);
	tripledc.GetChannel(TRIPLEDC_6)       -> SetActualCurrent(Ch0A);
	tripledc.GetChannel(TRIPLEDC_25PLUS)  -> SetActualCurrent(Ch1A);
	tripledc.GetChannel(TRIPLEDC_25MINUS) -> SetActualCurrent(Ch2A);
	tripledc.GetChannel(TRIPLEDC_6)       -> SetOutputLimited(Ch0L);
	tripledc.GetChannel(TRIPLEDC_25PLUS)  -> SetOutputLimited(Ch1L);
	tripledc.GetChannel(TRIPLEDC_25MINUS) -> SetOutputLimited(Ch2L);
}

#endif

/*

void InstrumentResponses::TripleDCSetup(Serializer& in, TripleDC& tripledc)
{

	// nothing yet..
}

void InstrumentResponses::CircuitFetch(Serializer& in, NetList2& resultlist)
{
}
*/