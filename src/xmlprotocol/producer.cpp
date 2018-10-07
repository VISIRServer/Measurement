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

#include "producer.h"
#include "xmltools.h"
#include "xmlversions.h"

#include <instruments/instrumentblock.h>
#include <instruments/functiongenerator.h>
#include <instruments/oscilloscope.h>
#include <instruments/digitalmultimeter.h>
#include <instruments/tripledc.h>
#include <instruments/signalanalyzer.h>

#include <protocol/protocol.h>
#include <protocol/basic_types.h>
#include <protocol/auth.h>

#include <stringop.h>

#include <float.h>
#include <contrib/base64.h>

// for testing only
#include <math.h>

#include <string>
#include <sstream>
#include <locale>

using namespace xmlprotocol;
using namespace std;

#define DIFF( prop ) \
	if (IgnoreDiff() || pPrev->Get##prop() != pCur->Get##prop())

#define VALUEOUT_DIFF( xml, name, prop ) \
	if(IgnoreDiff() || pPrev->Get##prop() != pCur->Get##prop()) xml.AddProperty(name, "value", pCur->Get##prop())

#define VALUEOUT_DIFF2( xml, name, prop, cur, prev ) \
	if(IgnoreDiff() || prev->Get##prop() != cur->Get##prop()) xml.AddProperty(name, "value", cur->Get##prop())

class ProducerVisitor : public InstrumentVisitor
{
public:
	ProducerVisitor(XmlContainer& root, InstrumentBlock* prev, InstrumentBlock* cur, bool response, bool ignorediff) : mRoot(root)
	{
		mPrev = prev; mCur = cur;
		mResponse = response;
		mIgnoreDiff = ignorediff;
	}

	virtual void Visit(Oscilloscope&		);
	virtual void Visit(DigitalMultimeter&	);
	virtual void Visit(FunctionGenerator&	);
	virtual void Visit(NodeInterpreter&		);
	virtual void Visit(TripleDC&			);
	virtual void Visit(SignalAnalyzer&		);

	inline InstrumentBlock* PrevBlock() { return mPrev; }
	inline InstrumentBlock* CurBlock() { return mCur; }
	
	void Write(std::ostream& out) { mRoot.Write(out); }

	bool IgnoreDiff() { return mIgnoreDiff; }

private:
	void EncodeSamples(XmlContainer& samples, const std::vector<double>& pTrace, bool dolog);

	InstrumentBlock* mPrev, *mCur;
	XmlContainer& mRoot;
	bool	mResponse;
	bool	mIgnoreDiff;
};

void ProducerVisitor::Visit(FunctionGenerator& fgen)
{
	FunctionGenerator* pCur = &fgen;
	FunctionGenerator* pPrev = PrevBlock()->Acquire<FunctionGenerator>(pCur->GetID());

	XmlContainer fgenxml("functiongenerator");

	VALUEOUT_DIFF(	fgenxml, "fg_waveform",		WaveFormStr		);
	VALUEOUT_DIFF(	fgenxml, "fg_amplitude",	Amplitude		);
	VALUEOUT_DIFF(	fgenxml, "fg_frequency",	Frequency		);
	VALUEOUT_DIFF(	fgenxml, "fg_offset",		DCOffset		);
	VALUEOUT_DIFF(	fgenxml, "fg_startphase",	Phase			);
	VALUEOUT_DIFF(	fgenxml, "fg_triggermode",	TriggerModeStr	);
	VALUEOUT_DIFF(	fgenxml, "fg_triggersource", TriggerSourceStr	);
	VALUEOUT_DIFF(	fgenxml, "fg_burstcount",	BurstCount		);
	VALUEOUT_DIFF(	fgenxml, "fg_dutycycle",	DutyCycleHigh	);

	// not implemented yet
	/*DIFF(UserDefinedWaveform)
	{
		XmlContainer fgenwavexml("fg_userdefinedwave");
		fgenwavexml.SetProperty("length", pFGen->Foo());
		fgenwavexml.SetProperty("encoding", "BASE64");

		fgenwavexml.SetData(base64encode(pFGen->GetUserDefinedWaveform()));
		fgenxml.AddContainer(fgenwavexml);
	}*/

	mRoot.AddContainer(fgenxml);
}

void ProducerVisitor::Visit(Oscilloscope& osc)
{
	Oscilloscope* pCur = &osc;
	Oscilloscope* pPrev = PrevBlock()->Acquire<Oscilloscope>(pCur->GetID());

	XmlContainer oscxml("oscilloscope");

	VALUEOUT_DIFF(	oscxml, "osc_autoscale",		AutoScale	);

	// horizontal
	{
		XmlContainer horizxml("horizontal");
		VALUEOUT_DIFF(	horizxml, "horz_samplerate",		MinSampleRate	);
		VALUEOUT_DIFF(	horizxml, "horz_refpos",			RefPos			);
		VALUEOUT_DIFF(	horizxml, "horz_recordlength",		ReqNumSamples	); // XXX: may the instrument return another value than requested?

		oscxml.AddContainer(horizxml);
	}

	// channels
	{
		XmlContainer channels("channels");

		for(int i=0; i<2; i++)
		{
			Channel* pChanCur = pCur->GetChannelPointer(i);
			Channel* pChanPrev = pPrev->GetChannelPointer(i);

			XmlContainer chanxml("channel");
			chanxml.AddValue("number", i+1);

			VALUEOUT_DIFF2(chanxml, "chan_enabled",		Enabled,				pChanCur, pChanPrev);
			VALUEOUT_DIFF2(chanxml, "chan_coupling",	VerticalCouplingStr,	pChanCur, pChanPrev);
			VALUEOUT_DIFF2(chanxml, "chan_range",		VerticalRange,			pChanCur, pChanPrev);
			VALUEOUT_DIFF2(chanxml, "chan_offset",		VerticalOffset,			pChanCur, pChanPrev);
			VALUEOUT_DIFF2(chanxml, "chan_attenuation", ProbeAttenuation,		pChanCur, pChanPrev);

			if (mResponse)
			{
				chanxml.AddProperty("chan_gain", "value", pChanCur->GetGraphGain());

				XmlContainer oscdata("chan_samples");
				oscdata.AddValue("encoding", "base64");

				size_t len	= pChanCur->GetNumSamples();

				if (len > 0)
				{
					char* src	= pChanCur->GetRawGraph();
					oscdata.AddData(base64::base64_encode((const unsigned char*)src, len));
					chanxml.AddContainer(oscdata);
				}
			}

			channels.AddContainer(chanxml);
		}

		oscxml.AddContainer(channels);
	}

	// trigger
	{
		Trigger* pTrigCur = pCur->GetTriggerPointer();
		Trigger* pTrigPrev = pPrev->GetTriggerPointer();

		XmlContainer trigger("trigger");

		VALUEOUT_DIFF2(trigger, "trig_source",		SourceStr,		pTrigCur, pTrigPrev);
		VALUEOUT_DIFF2(trigger, "trig_slope",		SlopeStr,		pTrigCur, pTrigPrev);
		VALUEOUT_DIFF2(trigger, "trig_coupling",	CouplingStr,	pTrigCur, pTrigPrev);
		VALUEOUT_DIFF2(trigger, "trig_level",		Level,			pTrigCur, pTrigPrev);
		VALUEOUT_DIFF2(trigger, "trig_mode",		ModeStr,		pTrigCur, pTrigPrev);
		VALUEOUT_DIFF2(trigger, "trig_delay",		Delay,			pTrigCur, pTrigPrev);

		if (mResponse)
		{
			trigger.AddProperty("trig_received", "value", pTrigCur->GetTriggerReceived());
			// trig_level could change also
		}

		oscxml.AddContainer(trigger);
	}

	// measurements
	{
		XmlContainer measurements("measurements");

		for(int i=0;i<3;i++)
		{
			Measurement* pMeasCur = pCur->GetMeasurementPointer(i);
			Measurement* pMeasPrev = pPrev->GetMeasurementPointer(i);

			XmlContainer measurement("measurement");
			measurement.AddValue("number", i+1);

			VALUEOUT_DIFF2(measurement, "meas_channel",		ChannelStr,		pMeasCur, pMeasPrev);
			VALUEOUT_DIFF2(measurement, "meas_selection",	SelectionStr,	pMeasCur, pMeasPrev);

			if (mResponse)
			{
				measurement.AddProperty("meas_result", "value", pMeasCur->GetMeasureResult());
			}

			measurements.AddContainer(measurement);
		}

		oscxml.AddContainer(measurements);
	}

	mRoot.AddContainer(oscxml);
}

void ProducerVisitor::Visit(DigitalMultimeter&	dmm)
{
	DigitalMultimeter* pCur = &dmm;
	DigitalMultimeter* pPrev = PrevBlock()->Acquire<DigitalMultimeter>(pCur->GetID());

	XmlContainer dmmxml("multimeter");
	dmmxml.AddValue("id", pCur->GetID());
	VALUEOUT_DIFF(dmmxml, "dmm_function",	FunctionStr);
	VALUEOUT_DIFF(dmmxml, "dmm_resolution",	ResolutionStr);
	VALUEOUT_DIFF(dmmxml, "dmm_range",		Range);

	if (mResponse)
	{
		dmmxml.AddProperty("dmm_result", "value", pCur->GetMeasureResult());
	}

	mRoot.AddContainer(dmmxml);
}

void ProducerVisitor::Visit(NodeInterpreter&	)
{
}

void ProducerVisitor::Visit(TripleDC& tripledc)
{
	TripleDC* pCur = &tripledc;
	TripleDC* pPrev = PrevBlock()->Acquire<TripleDC>();

	XmlContainer dcpower("dcpower");

	XmlContainer dcoutputs("dc_outputs");

    { // 6v+
		XmlContainer dcoutput("dc_output");
		dcoutput.AddValue("channel", "6V+");

		TripleDCChannel* pChCur = pCur->GetChannel(TRIPLEDC_6);
		TripleDCChannel* pChPrev = pPrev->GetChannel(TRIPLEDC_6);

		VALUEOUT_DIFF2(dcoutput, "dc_voltage",		Voltage,	pChCur, pChPrev);
		VALUEOUT_DIFF2(dcoutput, "dc_current",		Current,	pChCur, pChPrev);

		VALUEOUT_DIFF2(dcoutput, "dc_voltage_actual",	ActualVoltage,	pChCur, pChPrev);
		VALUEOUT_DIFF2(dcoutput, "dc_current_actual",	ActualCurrent,	pChCur, pChPrev);

		VALUEOUT_DIFF2(dcoutput, "dc_output_enabled",	OutputEnabled,	pChCur, pChPrev);
		VALUEOUT_DIFF2(dcoutput, "dc_output_limited",	OutputLimited,	pChCur, pChPrev);

		dcoutputs.AddContainer(dcoutput);
	}

	{ // 25v+
		XmlContainer dcoutput("dc_output");
		dcoutput.AddValue("channel", "25V+");

		TripleDCChannel* pChCur = pCur->GetChannel(TRIPLEDC_25PLUS);
		TripleDCChannel* pChPrev = pPrev->GetChannel(TRIPLEDC_25PLUS);

		VALUEOUT_DIFF2(dcoutput, "dc_voltage",		Voltage,	pChCur, pChPrev);
		VALUEOUT_DIFF2(dcoutput, "dc_current",		Current,	pChCur, pChPrev);

		VALUEOUT_DIFF2(dcoutput, "dc_voltage_actual",	ActualVoltage,	pChCur, pChPrev);
		VALUEOUT_DIFF2(dcoutput, "dc_current_actual",	ActualCurrent,	pChCur, pChPrev);

		VALUEOUT_DIFF2(dcoutput, "dc_output_enabled",	OutputEnabled,	pChCur, pChPrev);
		VALUEOUT_DIFF2(dcoutput, "dc_output_limited",	OutputLimited,	pChCur, pChPrev);
		
		dcoutputs.AddContainer(dcoutput);
	}

	{ // 25v-
		XmlContainer dcoutput("dc_output");
		dcoutput.AddValue("channel", "25V-");

		TripleDCChannel* pChCur = pCur->GetChannel(TRIPLEDC_25MINUS);
		TripleDCChannel* pChPrev = pPrev->GetChannel(TRIPLEDC_25MINUS);

		VALUEOUT_DIFF2(dcoutput, "dc_voltage",		Voltage,	pChCur, pChPrev);
		VALUEOUT_DIFF2(dcoutput, "dc_current",		Current,	pChCur, pChPrev);

		VALUEOUT_DIFF2(dcoutput, "dc_voltage_actual",	ActualVoltage,	pChCur, pChPrev);
		VALUEOUT_DIFF2(dcoutput, "dc_current_actual",	ActualCurrent,	pChCur, pChPrev);
		
		VALUEOUT_DIFF2(dcoutput, "dc_output_enabled",	OutputEnabled,	pChCur, pChPrev);
		VALUEOUT_DIFF2(dcoutput, "dc_output_limited",	OutputLimited,	pChCur, pChPrev);

		dcoutputs.AddContainer(dcoutput);
	}

	dcpower.AddContainer(dcoutputs);
	
	mRoot.AddContainer(dcpower);
}

unsigned char clamp(double x)
{
	if (x < 0) return 0;
	if (x > 255.0) return 255;
	return (unsigned char) x;
}

unsigned char downsample8(double val, double gain)
{
	double max = 255.0;
	double out = val / gain * max;
	if (out < 0) out = 0.0;
	if (out > max) out = max;
	return (unsigned char) out;
}

unsigned short downsample16(double val, double gain)
{
	double max = 65535.0;
	double out = val / gain * max;
	if (out < 0) out = 0.0;
	if (out > max) out = max;
	return (unsigned short) out;
}

void ProducerVisitor::EncodeSamples(XmlContainer& samples, const std::vector<double>& graph, bool dolog)
{
	//const SignalAnalyzerTrace::tGraph& graph = pTrace->GetGraph();
	size_t len = graph.size();
	double min = DBL_MAX;
	double max = -DBL_MAX;

	//sysout << "graphlen: " << len << endl;

	for(size_t i=0;i<len;i++)
	{
		double val = graph[i]; //(dolog ? log(graph[i]) : graph[i]);
		//sysout << val << " ";
		if (val > max) max = val;
		if (val < min) min = val;
	}

	double tmax = max;
	double tmin = min;

	if (dolog)
	{
		tmax = log(max);
		tmin = log(min);
	}

	double tgain = tmax - tmin;
	unsigned short* inbuffer = new unsigned short[len];

	for(size_t i=0;i<len;i++)
	{
		double sample = graph[i];
		if (dolog) sample = log(sample);
		inbuffer[i] = downsample16(sample - tmin, tgain);
	}

	//samples.AddValue("res", pCur->GetFreqRes());
	samples.AddValue("len", len);
	samples.AddValue("offset", ToStringScientific(tmin));
	samples.AddValue("gain", ToStringScientific(tgain));
	samples.AddValue("bits", 16);
	samples.AddValue("scale", (dolog) ? "log" : "lin");

	samples.AddData(base64::base64_encode( (unsigned char*)inbuffer, len * 2) );

	delete [] inbuffer;
}

void ProducerVisitor::Visit(SignalAnalyzer& instanalyzer)
{
	SignalAnalyzer* pCur = &instanalyzer;
	SignalAnalyzer* pPrev = PrevBlock()->Acquire<SignalAnalyzer>(pCur->GetID());

	XmlContainer analyzer("analyzer");

	VALUEOUT_DIFF(analyzer, "freq_start",	FreqStart);
	VALUEOUT_DIFF(analyzer, "freq_stop",	FreqStop);
	VALUEOUT_DIFF(analyzer, "freq_res",		FreqRes);

	VALUEOUT_DIFF(analyzer, "src_lvl",		SourceLevel);
	VALUEOUT_DIFF(analyzer, "src_lvl_unit",	SourceLevelUnit);
	VALUEOUT_DIFF(analyzer, "src_offset",	SourceOffset);
	VALUEOUT_DIFF(analyzer, "src_freq",		SourceFreq);

	VALUEOUT_DIFF(analyzer, "avg_totnum",	AvgTotNum);


	XmlContainer channels("channels");
	for(int i=0;i<4;i++)
	{
		XmlContainer channel("channel");
		channel.AddValue("number", i+1);
		SignalAnalyzerChannel* pChCur = pCur->Channel(i);
		SignalAnalyzerChannel* pChPrev = pPrev->Channel(i);
		
		VALUEOUT_DIFF2(channel, "ch_range",		Range,			pChCur, pChPrev);
		VALUEOUT_DIFF2(channel, "ch_range_unit",RangeUnit,		pChCur, pChPrev);
		VALUEOUT_DIFF2(channel, "ch_flags",		Flags,			pChCur, pChPrev);

		channels.AddContainer(channel);
	}

	analyzer.AddContainer(channels);

	// fake a graph
	/*vector<double> fakegraph;
	for(int i=0;i<401;i++)
	{
		fakegraph.push_back( sin(double(i)/50.0) );
	}

	pCur->Trace(0)->SetGraph(fakegraph);
	pCur->Trace(0)->SetYTop(1.0);
	pCur->Trace(0)->SetYBottom(-1.0);
	*/

	XmlContainer traces("traces");
	for(int i=0;i<4;i++)
	{
		XmlContainer trace("trace");
		trace.AddValue("number", i+1);
		SignalAnalyzerTrace* pTrCur = pCur->Trace(i);
		//SignalAnalyzerTrace* pTrPrev = pPrev->Trace(i);

		trace.AddProperty("tr_top",		"value", ToStringScientific(pTrCur->GetYTop()));
		trace.AddProperty("tr_bottom",	"value", ToStringScientific(pTrCur->GetYBottom()));
		trace.AddProperty("tr_left",	"value", ToStringScientific(pTrCur->GetXLeft()));
		trace.AddProperty("tr_right",	"value", ToStringScientific(pTrCur->GetXRight()));
		trace.AddProperty("tr_scalediv","value", ToStringScientific(pTrCur->GetScaleDiv()));

		trace.AddProperty("tr_dsp_xunit",	"value", pTrCur->GetDispXUnit());
		trace.AddProperty("tr_dsp_yunit",	"value", pTrCur->GetDispYUnit());

		if (!pTrCur->GetGraph().empty())
		{
			XmlContainer samples("samples");
			//bool dolog = (pTrCur->GetFormat() == "log");
			EncodeSamples(samples, pTrCur->GetGraph(), (pTrCur->GetFormat() == "log"));
			samples.AddValue("res", pCur->GetFreqRes());
			samples.AddValue("axis", "x");
			trace.AddContainer(samples);
		}

		if (pTrCur->GetFormat() == "nyq" && !pTrCur->GetGraphY().empty())
		{
			XmlContainer samples("samples");
			EncodeSamples(samples, pTrCur->GetGraphY(), false);
			samples.AddValue("res", pCur->GetFreqRes());
			samples.AddValue("axis", "y");
			trace.AddContainer(samples);
		}

		traces.AddContainer(trace);
	}

	analyzer.AddContainer(traces);
	mRoot.AddContainer(analyzer);
}

////////////////////////////////////////////

bool XmlProducer::ProduceRequest(std::ostream& out, InstrumentBlock* prev, InstrumentBlock* current, bool diff)
{
	XmlContainer request("request");
	ProducerVisitor visitor(request, prev, current, false, !diff);

	InstrumentBlock::tInstruments instruments = current->GetInstruments();
	for(InstrumentBlock::tInstruments::iterator it = instruments.begin(); it != instruments.end(); it++)
	{
		//if ( (*it)->IsEnabled() )
		(*it)->Accept(visitor);
	}

	XmlContainer protocol("protocol");
	protocol.AddValue("version", XML_PROTOCOL_VERSION_STR);

	protocol.AddContainer(request);
	protocol.Write(out);
	return true;
}


bool XmlProducer::ProduceResponse(std::ostream& out, InstrumentBlock* prev, InstrumentBlock* current, bool diff)
{
	XmlContainer response("response");

	ProducerVisitor visitor(response, prev, current, true, !diff);

	InstrumentBlock::tInstruments instruments = current->GetInstruments();
	for(InstrumentBlock::tInstruments::iterator it = instruments.begin(); it != instruments.end(); it++)
	{
		(*it)->Accept(visitor);
	}

	XmlContainer protocol("protocol");
	protocol.AddValue("version", XML_PROTOCOL_VERSION_STR);

	protocol.AddContainer(response);
	protocol.Write(out);
	return true;
}

std::string XmlEncode(const std::string& in)
{
	std::string out;
	for(size_t i=0;i<in.length();i++)
	{
		if (in[i] == '<') out += "&lt;";
		else if (in[i] == '>') out += "&gt;";
		else out += in[i];
	}
	return out;
}

bool XmlProducer::ProduceError(std::ostream& out, const std::string& error)
{
	XmlContainer protocol("protocol");
	protocol.AddValue("version", XML_PROTOCOL_VERSION_STR);

	XmlContainer xmlerror("error");
	xmlerror.AddData(XmlEncode(error));
	protocol.AddContainer(xmlerror);

	protocol.Write(out);
	return true;
}

bool XmlProducer::ProduceHeartBeat(std::ostream& out)
{
	XmlContainer protocol("protocol");
	protocol.AddValue("version", XML_PROTOCOL_VERSION_STR);

	XmlContainer ok("heartbeat");
	ok.AddData("OK");
	protocol.AddContainer(ok);
	protocol.Write(out);
	return true;
}

bool XmlProducer::ProduceAuthResponse(std::ostream& out, const std::string& sessionkey)
{
	XmlContainer protocol("protocol");
	protocol.AddValue("version", XML_PROTOCOL_VERSION_STR);

	XmlContainer login("login");
	login.AddValue("sessionkey", sessionkey);
	protocol.AddContainer(login);
	protocol.Write(out);
	return true;
}

bool XmlProducer::ProduceDomainPolicy(std::ostream& out, const std::string& policy)
{
	out << policy;
	/*XmlContainer policy("cross-domain-policy");
	XmlContainer allow("allow-access-from");
	allow.AddValue("domain", "*");
	allow.AddValue("to-ports", "*");
	policy.AddContainer(allow);
	policy.Write(ser);
	*/
	return true;
}

bool XmlProducer::ProduceProxyLogin(std::ostream& out)
{
	XmlContainer protocol("protocol");
	protocol.AddValue("version", XML_PROTOCOL_VERSION_STR);

	XmlContainer ok("proxy");
	protocol.AddContainer(ok);
	protocol.Write(out);
	return true;
}

bool XmlProducer::TransactionResponse(protocol::Transaction* pTransaction, InstrumentBlock* pBlock, std::ostream& out, protocol::IProtocolService* pService)
{
	out.imbue(std::locale::classic());
	typedef protocol::Transaction::tRequests tRequests;
	const tRequests& requests = pTransaction->GetRequests();

	bool rv = true;

	for(tRequests::const_iterator it = requests.begin(); it != requests.end(); it++)
	{
		protocol::Response* pResponse = (*it)->GetResponse();

		switch((*it)->GetType())
		{
		case protocol::RequestType::Authorize:
			{
				protocol::AuthResponse* pAuth = (protocol::AuthResponse*) pResponse;
				rv &= XmlProducer::ProduceAuthResponse(out, pAuth->GetSessionKey());
			}
			break;
		case protocol::RequestType::Measurement:
			{
				if (!pBlock) return false;

				rv &= XmlProducer::ProduceResponse(out, pBlock, pBlock, false);
			}
			break;
		case protocol::RequestType::Heartbeat:
			{
				rv &= XmlProducer::ProduceHeartBeat(out);
			}
			break;
		case protocol::RequestType::DomainPolicy:
			{
				rv &= XmlProducer::ProduceDomainPolicy(out, pService->GetCrossDomainPolicy());
			}
			break;
		default:
			pTransaction->Abort("XmlProducer::TransactionResponse: Unable to repond to transaction, type unknown", protocol::Fatal);
			break;
		}
	}

	if (!rv)
	{
		pTransaction->Abort("Failed to generate xml response", protocol::Fatal);
		return false;
	}

	return true;
}