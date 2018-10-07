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

#include "xmlcommands.h"
#include "xmlversions.h"

#include <instruments/instrumentblock.h>
#include <instruments/functiongenerator.h>
#include <instruments/digitalmultimeter.h>
#include <instruments/tripledc.h>
#include <instruments/oscilloscope.h>
#include <instruments/nodeinterpreter.h>
#include <instruments/signalanalyzer.h>

#include <syslog.h>
#include <stringop.h>
#include <sstream>

using namespace xmlprotocol;
using namespace std;

// XXX: Search and replace
namespace XmlDom {
	typedef XMLUtil::DOMNode::tChildren tXmlDomNodes;
}

XmlInstrumentCommand::XmlInstrumentCommand()
{
}

XmlInstrumentCommand::~XmlInstrumentCommand()
{
}

void XmlInstrumentCommand::SetData(XmlDomNode* pNode)
{
	mDom = *pNode;
}

std::string XmlInstrumentCommand::GetAttrValue(XmlDomNode* pNode)
{
	return pNode->GetAttr("value", true);
}

// a utility function that will throw exceptions on conversion error
double XmlInstrumentCommand::GetAttrValueDouble(XmlDomNode* pNode)
{
	string temp = pNode->GetAttr("value", true);
	char* endp = NULL;
	double result = strtod(temp.c_str(), &endp);
	if (*endp != '\0')
	{
		stringstream out;
		out << "Invalid floating point data in " << pNode->GetName();
		throw ValidationException(out.str());
	}
	return result;
}

int XmlInstrumentCommand::GetAttrValueInt(XmlDomNode* pNode, bool throws = true)
{
	return GetAttrValueInt(pNode, "value", throws);
}

int XmlInstrumentCommand::GetAttrValueInt(XmlDomNode* pNode, string attr, bool throws = true)
{
	string temp = pNode->GetAttr(attr, throws);
	char* endp = NULL;
	long result = strtol(temp.c_str(), &endp, 10);
	if ((*endp != '\0') && throws)
	{
		stringstream out;
		out << "Invalid integer data in " << pNode->GetName();
		throw ValidationException(out.str());
	}
	return result;
}

//////////////////////////////

XmlInstrumentCommand* XmlInstrumentCommandFactory::CreateFromDom(XmlDomNode* pNode)
{
	const string& name = pNode->GetName();

	XmlInstrumentCommand* pCommand = NULL;
	if		(name == "functiongenerator")	pCommand = new XmlFunctionGenerationCommand();
	else if (name == "multimeter")			pCommand = new XmlMultimeterCommand();
	else if (name == "dcpower")				pCommand = new XmlTripleDCCommand();
	else if (name == "oscilloscope")		pCommand = new XmlOscilloscopeCommand();
	else if (name == "circuit")				pCommand = new XmlCircuitCommand();
	else if (name == "analyzer")			pCommand = new XmlSignalAnalyzerCommand();

	return pCommand;
}

//////////////////////////////

void XmlFunctionGenerationCommand::ApplySettings(InstrumentBlock* pBlock)
{
	FunctionGenerator* pFGen = pBlock->Acquire<FunctionGenerator>();

	const XmlDom::tXmlDomNodes& children = mDom.GetChildren();
	for(XmlDom::tXmlDomNodes::const_iterator it = children.begin(); it != children.end(); it++)
	{
		const std::string& name = (*it)->GetName();
        
		if		(name == "fg_waveform")			pFGen->SetWaveFormStr(		GetAttrValue(*it));
		else if (name == "fg_amplitude")		pFGen->SetAmplitude(		GetAttrValueDouble(*it));
		else if (name == "fg_frequency")		pFGen->SetFrequency(		GetAttrValueDouble(*it));
		else if (name == "fg_offset")			pFGen->SetDCOffset(			GetAttrValueDouble(*it));
		else if (name == "fg_startphase")		pFGen->SetPhase(			GetAttrValueDouble(*it));
		else if (name == "fg_triggermode")		pFGen->SetTriggerModeStr(	GetAttrValue(*it));
		else if (name == "fg_triggersource")	pFGen->SetTriggerSourceStr(	GetAttrValue(*it));
		else if (name == "fg_burstcount")		pFGen->SetBurstCount(		GetAttrValueInt(*it));
		else if (name == "fg_dutycycle")		pFGen->SetDutyCycleHigh(	GetAttrValueDouble(*it));
		else
		{
			LogLevel(syslog,5) << "Unknown token in functiongenerator: " << name << endl;
		}
	}
}

void XmlMultimeterCommand::ApplySettings(InstrumentBlock* pBlock)
{
	int instrid = 1;
	instrid = GetAttrValueInt(&mDom, "id", false);
	if (instrid < 1) instrid = 1;

	DigitalMultimeter* pDmm = pBlock->Acquire<DigitalMultimeter>(instrid);

	const XmlDom::tXmlDomNodes& children = mDom.GetChildren();
	for(XmlDom::tXmlDomNodes::const_iterator it = children.begin(); it != children.end(); it++)
	{
		const std::string& name = (*it)->GetName();

		if		(name == "dmm_function")	pDmm->SetFunctionStr(		GetAttrValue(*it));
		else if (name == "dmm_resolution")	pDmm->SetResolutionStr(		GetAttrValue(*it));
		else if (name == "dmm_range")		pDmm->SetRange(				GetAttrValueDouble(*it));
		//else if (name == "dmm_autozero")	pDmm->SetAutoZero(			(DigitalMultimeter::AutoZero) GetAttrValueInt(*it));
		else
		{
			LogLevel(syslog,5) << "Unknown token in dmm: " << name << endl;
		}
	}
}

void XmlTripleDCCommand::ApplySettings(InstrumentBlock* pBlock)
{
	TripleDC* pTripleDC = pBlock->Acquire<TripleDC>();

	const XmlDomNode* pDomOutputs = mDom.GetChild("dc_outputs", true);

	const XmlDom::tXmlDomNodes& outputs = pDomOutputs->GetChildren();
	for(XmlDom::tXmlDomNodes::const_iterator it = outputs.begin(); it != outputs.end(); it++)
	{
		const std::string& channel = (*it)->GetAttr("channel", true);
		const XmlDom::tXmlDomNodes& settings = (*it)->GetChildren();

		for(XmlDom::tXmlDomNodes::const_iterator it2 = settings.begin(); it2 != settings.end(); it2++)
		{
			const std::string& setting = (*it2)->GetName();
			if (setting == "dc_voltage")
			{
				if		(channel == "6V+")	pTripleDC->GetChannel(TRIPLEDC_6)->SetVoltage(GetAttrValueDouble(*it2));
				else if (channel == "25V+")	pTripleDC->GetChannel(TRIPLEDC_25PLUS)->SetVoltage(GetAttrValueDouble(*it2));
				else if (channel == "25V-")	pTripleDC->GetChannel(TRIPLEDC_25MINUS)->SetVoltage(GetAttrValueDouble(*it2));
				else
				{
					LogLevel(syslog,5) << "Unknown channel used in tripledc: " << channel << endl;
				}
			}
			else if (setting == "dc_current")
			{
				if		(channel == "6V+")	pTripleDC->GetChannel(TRIPLEDC_6)->SetCurrent(GetAttrValueDouble(*it2));
				else if (channel == "25V+")	pTripleDC->GetChannel(TRIPLEDC_25PLUS)->SetCurrent(GetAttrValueDouble(*it2));
				else if (channel == "25V-")	pTripleDC->GetChannel(TRIPLEDC_25MINUS)->SetCurrent(GetAttrValueDouble(*it2));
				else
				{
					LogLevel(syslog,5) << "Unknown channel used in tripledc: " << channel << endl;
				}
			}

			else if (setting == "dc_output_enabled")
			{
				if		(channel == "6V+")	pTripleDC->GetChannel(TRIPLEDC_6)->SetOutputEnabled(GetAttrValueInt(*it2));
				else if (channel == "25V+")	pTripleDC->GetChannel(TRIPLEDC_25PLUS)->SetOutputEnabled(GetAttrValueInt(*it2));
				else if (channel == "25V-")	pTripleDC->GetChannel(TRIPLEDC_25MINUS)->SetOutputEnabled(GetAttrValueInt(*it2));
				else
				{
					LogLevel(syslog,5) << "Unknown channel used in tripledc: " << channel << endl;
				}
			}
			else
			{
				LogLevel(syslog,5) << "Unknown setting in trippledc: " << setting << endl;
			}
		}
	}
}

void XmlOscilloscopeCommand::ApplySettings(InstrumentBlock* pBlock)
{
	Oscilloscope* pOsc = pBlock->Acquire<Oscilloscope>();

	const XmlDom::tXmlDomNodes& roots = mDom.GetChildren();
	
	for(XmlDom::tXmlDomNodes::const_iterator root_it = roots.begin(); root_it != roots.end(); root_it++)
	{
		const std::string& root_name = (*root_it)->GetName();
		
		if		(root_name == "horizontal")
		{
			const XmlDom::tXmlDomNodes& horz_settings = (*root_it)->GetChildren();
			for(XmlDom::tXmlDomNodes::const_iterator horz_it = horz_settings.begin(); horz_it != horz_settings.end(); horz_it++)
			{
				const std::string& horz_name = (*horz_it)->GetName();
#if XML_PROTOCOL_MAJOR_VERSION < 2
				if (horz_name == "horz_samplerate")				pOsc->SetSampleRate(	GetAttrValueDouble(*horz_it) / 10.0);
#else
				if (horz_name == "horz_samplerate")				pOsc->SetSampleRate(	GetAttrValueDouble(*horz_it));
#endif
				else if (horz_name == "horz_refpos")			pOsc->SetRefPos(		GetAttrValueDouble(*horz_it));
				else if (horz_name == "horz_recordlength")		pOsc->SetReqNumSamples(	GetAttrValueDouble(*horz_it));
			}
		}
		else if (root_name == "channels")
		{
			const XmlDom::tXmlDomNodes& channels = (*root_it)->GetChildren();
			for(XmlDom::tXmlDomNodes::const_iterator chan_it = channels.begin(); chan_it != channels.end(); chan_it++)
			{
				int channr = ToInt( (*chan_it)->GetAttr("number", true) );
				channr -= 1; // begin at offset 0
				
				Channel* pChannel = pOsc->GetChannelPointer(channr);
				if (!pChannel) throw BasicException("Non-existent channel requested");

				const XmlDom::tXmlDomNodes& chan_settings = (*chan_it)->GetChildren();
				for(XmlDom::tXmlDomNodes::const_iterator chanset_it = chan_settings.begin(); chanset_it != chan_settings.end(); chanset_it++)
				{
					const std::string& chs_name = (*chanset_it)->GetName();

					if		(chs_name == "chan_enabled")		pChannel->SetEnabled(				GetAttrValueInt(*chanset_it));
					else if (chs_name == "chan_coupling")		pChannel->SetVerticalCouplingStr(	GetAttrValue(*chanset_it));
					
#if XML_PROTOCOL_MAJOR_VERSION < 2
					else if (chs_name == "chan_range")			pChannel->SetVerticalRange(			8.0 * GetAttrValueDouble(*chanset_it));
#else
					else if (chs_name == "chan_range")			pChannel->SetVerticalRange(			GetAttrValueDouble(*chanset_it));					
#endif
					else if (chs_name == "chan_offset")			pChannel->SetVerticalOffset(		GetAttrValueDouble(*chanset_it));
					else if (chs_name == "chan_attenuation")	pChannel->SetProbeAttenuation(		GetAttrValueDouble(*chanset_it));
					else
					{
						LogLevel(syslog,5) << "Unknown token in osc channel: " << chs_name << endl;
					}
				}
			}			
		}
		else if (root_name == "trigger")
		{
			const XmlDom::tXmlDomNodes& trigger_settings = (*root_it)->GetChildren();
			for(XmlDom::tXmlDomNodes::const_iterator trigger_it = trigger_settings.begin(); trigger_it != trigger_settings.end(); trigger_it++)
			{
				const std::string trs_name = (*trigger_it)->GetName();
				Trigger* pTrigger = pOsc->GetTriggerPointer();
                
				if		(trs_name == "trig_source")		pTrigger->SetSourceStr(		GetAttrValue(*trigger_it));
				else if (trs_name == "trig_slope")		pTrigger->SetSlopeStr(		GetAttrValue(*trigger_it));
				else if (trs_name == "trig_coupling")	pTrigger->SetCouplingStr(	GetAttrValue(*trigger_it));
				else if (trs_name == "trig_level")		pTrigger->SetLevel(			GetAttrValueDouble(*trigger_it));
				else if (trs_name == "trig_mode")		pTrigger->SetModeStr(		GetAttrValue(*trigger_it));
				else if (trs_name == "trig_timeout")	pTrigger->SetTimeout(		GetAttrValueDouble(*trigger_it));
				else if (trs_name == "trig_delay")		pTrigger->SetDelay(			GetAttrValueDouble(*trigger_it));
				else
				{
					LogLevel(syslog,5) << "Unknown token in osc trigger: " << trs_name << endl;
				}
			}
		}
		else if (root_name == "measurements")
		{
			const XmlDom::tXmlDomNodes& measurements = (*root_it)->GetChildren();
			for(XmlDom::tXmlDomNodes::const_iterator meas_it = measurements.begin(); meas_it != measurements.end(); meas_it++)
			{
				int measnr = ToInt( (*meas_it)->GetAttr("number", true) );
				measnr -= 1; // begin at offset 0

				Measurement* pMeas = pOsc->GetMeasurementPointer(measnr);

				const XmlDom::tXmlDomNodes& meas_settings = (*meas_it)->GetChildren();
				for(XmlDom::tXmlDomNodes::const_iterator measset_it = meas_settings.begin(); measset_it != meas_settings.end(); measset_it++)
				{
					const std::string mes_name = (*measset_it)->GetName();

					if		(mes_name == "meas_channel")	pMeas->SetChannelStr(	GetAttrValue(*measset_it));
					else if (mes_name == "meas_selection")	pMeas->SetSelectionStr(	GetAttrValue(*measset_it));
					else
					{
						LogLevel(syslog,5) << "Unknown token in osc measurement: " << mes_name << endl;
					}
				}
			}
		}
		else if	(root_name == "osc_autoscale")
		{
			pOsc->SetAutoScale(GetAttrValueInt(*root_it));
		}
		else
		{
			LogLevel(syslog,5) << "Unknown root node in osc: " << root_name << endl;
		}
	}
}

void XmlCircuitCommand::ApplySettings(InstrumentBlock* pBlock)
{
	const XmlDomNode* pCircuitlist = mDom.GetChild("circuitlist", true);
	pBlock->GetNodeInterpreter()->SetCircuitList(pCircuitlist->GetData());
}

void XmlSignalAnalyzerCommand::ApplySettings(InstrumentBlock* pBlock)
{
	SignalAnalyzer* pAnalyzer = pBlock->Acquire<SignalAnalyzer>();
	
	const XmlDom::tXmlDomNodes& roots = mDom.GetChildren();

	for(XmlDom::tXmlDomNodes::const_iterator root_it = roots.begin(); root_it != roots.end(); root_it++)
	{
		const std::string& name = (*root_it)->GetName();

		if		(name == "inst_channels")	pAnalyzer->SetChannels(		GetAttrValueInt(*root_it));
		else if	(name == "inst_ref")		pAnalyzer->SetReference(	GetAttrValue(*root_it));
		else if	(name == "inst_mode")		pAnalyzer->SetMode(			GetAttrValue(*root_it));
		else if	(name == "freq_start")		pAnalyzer->SetFreqStart(	GetAttrValueDouble(*root_it));
		else if	(name == "freq_stop")		pAnalyzer->SetFreqStop(		GetAttrValueDouble(*root_it));
		else if	(name == "freq_res")		pAnalyzer->SetFreqRes(		GetAttrValueInt(*root_it));
		else if	(name == "blksize")			pAnalyzer->SetBlocksize(		GetAttrValueInt(*root_it));
		else if	(name == "window_type")		pAnalyzer->SetWindowType(	GetAttrValue(*root_it));
		else if	(name == "src_on")			pAnalyzer->SetSourceOn(		GetAttrValueInt(*root_it));
		else if	(name == "src_lvl")			pAnalyzer->SetSourceLevel(	GetAttrValueDouble(*root_it));
		else if	(name == "src_lvl_unit")	pAnalyzer->SetSourceLevelUnit(	GetAttrValue(*root_it));
		else if	(name == "src_offset")		pAnalyzer->SetSourceOffset(	GetAttrValueDouble(*root_it));
		else if	(name == "src_func")		pAnalyzer->SetSourceFunc(	GetAttrValue(*root_it));
		else if	(name == "src_freq")		pAnalyzer->SetSourceFreq(	GetAttrValueDouble(*root_it));
		else if	(name == "src_burst")		pAnalyzer->SetSourceBurst(	GetAttrValueDouble(*root_it));

		else if	(name == "avg_on")			pAnalyzer->SetAvgOn(		GetAttrValueInt(*root_it));
		else if	(name == "avg_num_avg")		pAnalyzer->SetAvgNum(		GetAttrValueInt(*root_it));
		else if	(name == "avg_type")		pAnalyzer->SetAvgType(		GetAttrValue(*root_it));
		
		/*
		else if	(name == "avg_fast")		pAnalyzer->SetAvgFast(		GetAttrValueInt(*root_it));
		else if	(name == "avg_rate")		pAnalyzer->SetAvgRate(		GetAttrValueInt(*root_it));
		else if	(name == "avg_repeat")		pAnalyzer->SetAvgRepeat(	GetAttrValueInt(*root_it));
		*/
		
		else if	(name == "avg_overlap")		pAnalyzer->SetAvgOverlap(	GetAttrValueDouble(*root_it));
		else if	(name == "avg_ovldrej")		pAnalyzer->SetAvgOvldRej(	GetAttrValueInt(*root_it));

		else if (name == "disp_format")		pAnalyzer->SetDispFormat(	GetAttrValue(*root_it));
		else if (name == "active_trc")		pAnalyzer->SetActiveTrace(	GetAttrValue(*root_it));

		else if (name == "meas_type")		pAnalyzer->SetMeasureType(	GetAttrValue(*root_it));

		else if	(name == "channels")
		{
			const XmlDom::tXmlDomNodes& channels = (*root_it)->GetChildren();
			for(XmlDom::tXmlDomNodes::const_iterator chan_it = channels.begin(); chan_it != channels.end(); chan_it++)
			{
				if ((*chan_it)->GetName() != "channel") throw BasicException("unknown analyser channel entry");

				int channr = ToInt( (*chan_it)->GetAttr("number", true) );
				channr -= 1; // begin at offset 0

				SignalAnalyzerChannel* pChannel = pAnalyzer->Channel(channr);
				if (!pChannel) throw BasicException("Non-existing analyzer channel");

				const XmlDom::tXmlDomNodes& chan_settings = (*chan_it)->GetChildren();
				for(XmlDom::tXmlDomNodes::const_iterator chanset_it = chan_settings.begin(); chanset_it != chan_settings.end(); chanset_it++)
				{
					const std::string& chs_name = (*chanset_it)->GetName();

					if		(chs_name == "ch_range")		pChannel->SetRange(		GetAttrValueDouble(*chanset_it));
					else if	(chs_name == "ch_range_unit")	pChannel->SetRangeUnit(	GetAttrValue(*chanset_it));
					else if	(chs_name == "ch_range_mode")	pChannel->SetRangeMode(	GetAttrValue(*chanset_it));

					else if	(chs_name == "ch_input")		pChannel->SetInput(		GetAttrValue(*chanset_it));
					else if	(chs_name == "ch_coupling")		pChannel->SetCoupling(	GetAttrValue(*chanset_it));
					else if	(chs_name == "ch_antialias")	pChannel->SetAntiAlias(	GetAttrValueInt(*chanset_it));
					else if	(chs_name == "ch_filteraw")		pChannel->SetFilterAW(	GetAttrValueInt(*chanset_it));
					else if	(chs_name == "ch_bias")			pChannel->SetBias(		GetAttrValueInt(*chanset_it));

					else if	(chs_name == "ch_xdcr")				pChannel->SetXDCR(			GetAttrValueInt(*chanset_it));
					else if	(chs_name == "ch_xdcr_sens")		pChannel->SetXDCRSens(		GetAttrValueDouble(*chanset_it));
					else if	(chs_name == "ch_xdcr_sens_unit")	pChannel->SetXDCRSensUnit(	GetAttrValue(*chanset_it));
					else if	(chs_name == "ch_xdcr_label")		pChannel->SetXDCRLabel(		GetAttrValue(*chanset_it));
					else
					{
						LogLevel(syslog,5) << "Unknown token in analyzer channel: " << chs_name << endl;
					}
				}
			}
		}
		else if (name == "traces")
		{

			const XmlDom::tXmlDomNodes& traces = (*root_it)->GetChildren();
			for(XmlDom::tXmlDomNodes::const_iterator trace_it = traces.begin(); trace_it != traces.end(); trace_it++)
			{
				if ((*trace_it)->GetName() != "trace") throw BasicException("unknown analyser trace entry");

				int tracenr = ToInt( (*trace_it)->GetAttr("number", true) );
				tracenr -= 1; // begin at offset 0

				SignalAnalyzerTrace* pTrace = pAnalyzer->Trace(tracenr);
				if (!pTrace) throw BasicException("Non-existing analyzer trace");

				const XmlDom::tXmlDomNodes& trace_settings = (*trace_it)->GetChildren();
				for(XmlDom::tXmlDomNodes::const_iterator traceset_it = trace_settings.begin(); traceset_it != trace_settings.end(); traceset_it++)
				{
					const std::string& trs_name = (*traceset_it)->GetName();

					if		(trs_name == "tr_chan")			pTrace->SetChannel(		GetAttrValueInt(*traceset_it));
					else if	(trs_name == "tr_measure")		pTrace->SetMeasure(		GetAttrValue(*traceset_it));
					else if	(trs_name == "tr_format")		pTrace->SetFormat(		GetAttrValue(*traceset_it));
					else if	(trs_name == "tr_xspacing")		pTrace->SetXSpacing(	GetAttrValue(*traceset_it));
					else if	(trs_name == "tr_autoscale")	pTrace->SetAutoScale(	GetAttrValueInt(*traceset_it));
					else if	(trs_name == "tr_scale")		pTrace->SetScale(		GetAttrValue(*traceset_it));
					else if	(trs_name == "tr_scalediv")		pTrace->SetScaleDiv(	GetAttrValueDouble(*traceset_it)); // switched to double
					else if	(trs_name == "tr_voltunit")		pTrace->SetVoltUnit(	GetAttrValue(*traceset_it));
					else
					{
						LogLevel(syslog,5) << "Unknown token in analyzer trace: " << trs_name << endl;
					}
				}
			}
		}
		else
		{
			LogLevel(syslog,5) << "Unknown token in analyzer: " << name << endl;
		}
	}
}
