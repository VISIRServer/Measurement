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

#include "ividrivers.h"

#include <instruments/instrumentblock.h>
#include <instruments/oscilloscope.h>
#include <instruments/digitalmultimeter.h>
#include <instruments/functiongenerator.h>
#include <instruments/nodeinterpreter.h>
#include <instruments/tripledc.h>

#include <instruments/measurement.h>

#include <util/basic_exception.h>
#include "doerror.h"
#include "log.h"

#include "DirectIVI/cIVIDCPower.h" 
#include "DirectIVI/cIVIDMM.h"
#include "DirectIVI/cIVIFGEN.h"
#include "DirectIVI/cIVIScope.h"

#include "DirectIVI/cIVIScopeChannel.h"

#include "commands.h"
#include "experiment.h"

#include <stringop.h>
#include <syslog.h>
#include <quantize.h>

#include <string>
using namespace std;

#include <iviScope.h> // needed for constants

#include <ctime>

using namespace IVIControl;

// hardcoded number of channels
#define HC_OSC_CHANNELS 2

InstrumentCommands::InstrumentCommands(Drivers* pDrivers) : mpDrivers(pDrivers)
{
}

//-----------------------------------------------------------------------------
void InstrumentCommands::TripleDCFetch(TripleDC& tripledc){
	ivilog.Log(5) << "InstrumentCommands::TripleDCFetch" << endl;
	clock_t cbegin = clock();

	cIVIDCPower* pDCPower = mpDrivers->GetDCPower(0);
	if (pDCPower == NULL) return;

	tripledc.GetChannel(TRIPLEDC_6)       -> SetActualVoltage(pDCPower->GetChannelByType("+6V" )->GetOutputVoltage());
	tripledc.GetChannel(TRIPLEDC_25PLUS)  -> SetActualVoltage(pDCPower->GetChannelByType("+20V")->GetOutputVoltage());
	tripledc.GetChannel(TRIPLEDC_25MINUS) -> SetActualVoltage(pDCPower->GetChannelByType("-20V")->GetOutputVoltage());
	tripledc.GetChannel(TRIPLEDC_6)       -> SetActualCurrent(pDCPower->GetChannelByType("+6V" )->GetOutputCurrent());
	tripledc.GetChannel(TRIPLEDC_25PLUS)  -> SetActualCurrent(pDCPower->GetChannelByType("+20V")->GetOutputCurrent());
	tripledc.GetChannel(TRIPLEDC_25MINUS) -> SetActualCurrent(pDCPower->GetChannelByType("-20V")->GetOutputCurrent());
	tripledc.GetChannel(TRIPLEDC_6)       -> SetOutputLimited(pDCPower->GetChannelByType("+6V" )->GetOutputLimited());
	tripledc.GetChannel(TRIPLEDC_25PLUS)  -> SetOutputLimited(pDCPower->GetChannelByType("+20V")->GetOutputLimited());
	tripledc.GetChannel(TRIPLEDC_25MINUS) -> SetOutputLimited(pDCPower->GetChannelByType("-20V")->GetOutputLimited());

	clock_t cend = clock();
	clock_t cdiff = cend - cbegin;
	double msec = ((double)cdiff * (double)1000) / CLOCKS_PER_SEC;
	ivilog.Log(5) << "took " << msec << "msec" << endl;
}
//-----------------------------------------------------------------------------

int DmmToIVIFunc(int dmmfunction)
{
	switch (dmmfunction) {
case 0:		return DMM_DC_VOLTS;		break;
case 1:		return DMM_AC_VOLTS;		break;
case 2:		return DMM_DC_CURRENT;		break;
case 3:		return DMM_AC_CURRENT;		break;
case 4:		return DMM_2_WIRE_RES;		break;
case 5:		return DMM_4_WIRE_RES;		break;
case 7:		return DMM_FREQ;			break;
case 8:		return DMM_PERIOD;			break;
#ifdef _NI_EXT_
case 6:		return DMM_DIODE;			break;
case 9:		return DMM_AC_VOLTS_DC_COUPLED;	break;     
case 10:	return DMM_CAPACITANCE;		break;
case 11:	return DMM_INDUCTANCE;		break;
#endif
default:	return -1;					break;
	}
}

void InstrumentCommands::DigitalMultimeterFetch(DigitalMultimeter& dmm){
	cIVIDMM* pDMM = mpDrivers->GetDMM(dmm.GetID() - 1);
	if (pDMM == NULL) {
		throw BasicException("Command to non-existent DigitalMultimeter");
		return;
	}

	ivilog.Log(5) << "DigitalMultimeterFetch" << endl;
	clock_t cbegin = clock();

	int32_t autozero = dmm.GetAutoZero();
	if (autozero != -1 ) pDMM->SetAutoZero(autozero);

	double resolutions[] = { 3.5, 4.5, 5.5, 6.5 };

	size_t dmmres = dmm.GetResolution();

	if (dmmres >= (sizeof(resolutions) / sizeof(resolutions[0])) ) {
		ivilog.Log(1) << "Digital Multi Meter: Unknown resolution requested!" ;
		doerror("Digital Multi Meter: Unknown resolution requested!");
	}

	double aResolution = resolutions[dmmres];

	int ivifunc = DmmToIVIFunc(dmm.GetFunction());
	if (ivifunc < 0) {
		ivilog.Log(1) << "Digital Multi Meter: Unknown Function Requested!" ;
		doerror("Digital Multi Meter: Unknown Function Requested!");
	}

	double aResult = pDMM->Measure(ivifunc,dmm.GetRange(),aResolution);

	dmm.SetMeasureResult(aResult);

	clock_t cend = clock();
	clock_t cdiff = cend - cbegin;
	double msec = ((double)cdiff * (double)1000) / CLOCKS_PER_SEC;
	ivilog.Log(5) << "took " << msec << "msec" << endl;
}


//-----------------------------------------------------------------------------
void InstrumentCommands::TripleDCSetup(TripleDC& tripledc, NodeInterpreter* pNodeIntr){
	cIVIDCPower* pDCPower = mpDrivers->GetDCPower(tripledc.GetID() - 1);
	if (pDCPower == NULL)
	{
		throw BasicException("Command to non-existent TripleDC");
		return;
	}

	ivilog.Log(5) << "TripleDCSetup" << endl;
	clock_t cbegin = clock();

	pDCPower->GetChannelByType("+6V" )->SetEnabled(tripledc.GetChannel(TRIPLEDC_6)      ->GetOutputEnabled());
	pDCPower->GetChannelByType("+6V" )->SetVoltage(tripledc.GetChannel(TRIPLEDC_6)      ->GetVoltage());
	pDCPower->GetChannelByType("+6V" )->SetCurrent(tripledc.GetChannel(TRIPLEDC_6)      ->GetCurrent());

	pDCPower->GetChannelByType("+20V")->SetEnabled(tripledc.GetChannel(TRIPLEDC_25PLUS) ->GetOutputEnabled());
	pDCPower->GetChannelByType("+20V")->SetVoltage(tripledc.GetChannel(TRIPLEDC_25PLUS) ->GetVoltage());
	pDCPower->GetChannelByType("+20V")->SetCurrent(tripledc.GetChannel(TRIPLEDC_25PLUS) ->GetCurrent());

	pDCPower->GetChannelByType("-20V")->SetEnabled(tripledc.GetChannel(TRIPLEDC_25MINUS)->GetOutputEnabled());
	pDCPower->GetChannelByType("-20V")->SetVoltage(tripledc.GetChannel(TRIPLEDC_25MINUS)->GetVoltage());
	pDCPower->GetChannelByType("-20V")->SetCurrent(tripledc.GetChannel(TRIPLEDC_25MINUS)->GetCurrent());


	clock_t cend = clock();
	clock_t cdiff = cend - cbegin;
	double msec = ((double)cdiff * (double)1000) / CLOCKS_PER_SEC;
	ivilog.Log(5) << "took " << msec << "msec" << endl;


	// some checking if the DCPower is actually connected. The other module did this,
	// so i've implemented this check here as well.

#ifdef _DO_CONNECTED_CHECK_
	if (pNodeIntr){
		if (!(pNodeIntr->TripleDCConnection(tripledc, TRIPLEDC_6).IsConnected())) {
			pDCPower->GetChannelByType("+6V" )->SetEnabled(0);
		}
		if (!(pNodeIntr->TripleDCConnection(tripledc, TRIPLEDC_25PLUS).IsConnected())) {
			pDCPower->GetChannelByType("+20V" )->SetEnabled(0);
		}
		if (!(pNodeIntr->TripleDCConnection(tripledc, TRIPLEDC_25MINUS).IsConnected())) {
			pDCPower->GetChannelByType("-20V" )->SetEnabled(0);
		}
	} else {
		pDCPower->GetChannelByType("+6" )->SetEnabled(0);
		pDCPower->GetChannelByType("+20V")->SetEnabled(0);// ?? NullPointer here, why? 
		pDCPower->GetChannelByType("-20V")->SetEnabled(0);
	}
#endif
}
//-----------------------------------------------------------------------------
void InstrumentCommands::FunctionGeneratorSetup(FunctionGenerator& funcgen){
	cIVIFGEN* pFGEN = mpDrivers->GetFGEN(funcgen.GetID() - 1);
	if (pFGEN == NULL)
	{
		throw BasicException("Command to non-existent FunctionGenerator");
		return;
	}

	ivilog.Log(5) << "FunctionGeneratorSetup" << endl;
	clock_t cbegin = clock();

	pFGEN->GetChannelByIndex(0)->SetAmplitude(funcgen.GetAmplitude() * 2.0); // unloaded amplitude
	pFGEN->GetChannelByIndex(0)->SetDCOffset(funcgen.GetDCOffset());
	pFGEN->GetChannelByIndex(0)->SetFrequency(funcgen.GetFrequency());
	pFGEN->GetChannelByIndex(0)->SetStartPhase(funcgen.GetPhase());

	// mapping to IVI or NI waveforms
	int32_t waveform = funcgen.GetWaveForm()+1;

#ifdef _NI_EXT_  
	if (waveform > 6) waveform += 94; // magic number 94?
#else
	if (waveform > 6)  {
		ivilog.Log(1) << "FGEN: Unsupported Waveform Requested!" << waveform << endl;   
		doerror("FGEN: Unsupported Waveform Requested!");
	}
#endif 

	pFGEN->GetChannelByIndex(0)->SetWaveform(waveform);

	// variable seems not to be set so will generate invalid value error!
	//drv->FGEN[0]->GetChannelByIndex(0)->SetDutyCycleHigh(funcgen.GetDutyCycleHigh());


	// not implemented by driver class yet: 
	//funcgen.GetBurstCount()
	//funcgen.GetTriggerMode()
	//funcgen.GetTriggerSource()
	//funcgen.GetUserWaveform()

	pFGEN->GetChannelByIndex(0)->SetOutput();

	clock_t cend = clock();
	clock_t cdiff = cend - cbegin;
	double msec = ((double)cdiff * (double)1000) / CLOCKS_PER_SEC;
	ivilog.Log(5) << "took " << msec << "msec" << endl;
}

int ConvertTriggerSource(Trigger::TriggerSource s)
{
	int out = 0;
	switch(s)
	{
	case Trigger::Chan0: out = 0; break;
	case Trigger::Chan1: out = 1; break;
	case Trigger::Chan2: out = 2; break;
	case Trigger::Chan3: out = 3; break;
	case Trigger::Immediate:
	case Trigger::External:
		out = 0;
		break; // don't case about channel
	default:
		throw BasicException("Unknown trigger source");
	}

	return out;
}

void InstrumentCommands::OscilloscopeSetup(Oscilloscope& osc)
{
	cIVIScope* pScope = mpDrivers->GetScope(osc.GetID() - 1);
	if (pScope == NULL)	{
		throw BasicException("Command to non-existent Oscilloscope");
		return;
	}
	ivilog.Log(5) << "OscilloscopeSetup" << endl;
	clock_t cbegin = clock();

	pScope->SetRequestLength(osc.GetReqNumSamples());

	pScope->SetRecordTime(osc.GetTimeRange());

	pScope->SetTriggerCoupling(osc.GetTriggerPointer()->GetCoupling());
	pScope->SetTriggerLevel(osc.GetTriggerPointer()->GetLevel());

	//cast to prevent compiler warning about acuracy loss
	//pScope->SetMaxTimemSec((int32_t)(1000 * osc.GetTriggerPointer()->GetTimeout()));
	pScope->SetMaxTimemSec(100);

	pScope->SetTriggerType(IVISCOPE_VAL_EDGE_TRIGGER); // ?
	//osc.GetTriggerPointer()->GetType();

	pScope->SetTriggerModifier( osc.GetTriggerPointer()->GetMode() + 1); // offsetted by one from the ivi standard

	/*
	how to match these???? 

	Edge		= 0,	/// default
	Hystereses	= 1,
	Digital		= 2,
	Window		= 3,

	#define IVISCOPE_VAL_EDGE_TRIGGER                               (1L)
	#define IVISCOPE_VAL_WIDTH_TRIGGER                              (2L)
	#define IVISCOPE_VAL_RUNT_TRIGGER                               (3L)
	#define IVISCOPE_VAL_GLITCH_TRIGGER                             (4L)
	#define IVISCOPE_VAL_TV_TRIGGER                                 (5L)
	#define IVISCOPE_VAL_IMMEDIATE_TRIGGER                          (6L)
	#define IVISCOPE_VAL_AC_LINE_TRIGGER                            (7L)
	#define IVISCOPE_VAL_TRIGGER_TYPE_CLASS_EXT_BASE                (200L)
	#define IVISCOPE_VAL_TRIGGER_TYPE_SPECIFIC_EXT_BASE             (1000L)
	*/

	int trigsource = ConvertTriggerSource(osc.GetTriggerPointer()->GetSource());
	pScope->SetTriggerSource(trigsource);


	// values are reversed!!!
	if (osc.GetTriggerPointer()->GetSlope()==1) pScope->SetTriggerSlope(0);
	if (osc.GetTriggerPointer()->GetSlope()==0) pScope->SetTriggerSlope(1);

	for(int i=0;i<HC_OSC_CHANNELS;i++) {
		pScope->GetChannelByIndex(i)->SetProbeAttenuation(osc.GetChannel(i).GetProbeAttenuation());
		pScope->GetChannelByIndex(i)->SetVertCoupling    (osc.GetChannel(i).GetVerticalCoupling());
		pScope->GetChannelByIndex(i)->SetVertOffset      (osc.GetChannel(i).GetVerticalOffset());
		pScope->GetChannelByIndex(i)->SetVertRange       (osc.GetChannel(i).GetVerticalRange());
	}

	//if (osc.GetAutoScale())  pScope->AutoSetup();

	/*uint32_t i;
	for ( i = 0; i < 3 ; i ++ ) { 
		uint32_t selection = osc.GetMeasurementPointer(i)->GetSelection();
		if (selection == 4000) continue;
		int MeasChannel = osc.GetMeasurementPointer(i)->GetChannel();
		osc.GetMeasurementPointer(i)->SetMeasureResult(pScope->GetChannelByIndex(MeasChannel)->DoMeasurement(selection));
	}*/

	clock_t cend = clock();
	clock_t cdiff = cend - cbegin;
	double msec = ((double)cdiff * (double)1000) / CLOCKS_PER_SEC;
	ivilog.Log(5) << "took " << msec << "msec" << endl;

}

void InstrumentCommands::OscilloscopeFetch(Oscilloscope& osc)
{
	//TODO: add autoscale somewhere around here..

	cIVIScope* pScope = mpDrivers->GetScope(osc.GetID() - 1);
	if (pScope == NULL)	{
		throw BasicException("Command to non-existent Oscilloscope");
		return;
	}

	ivilog.Log(5) << "OscilloscopeFetch" << endl;

	pScope->Acquire();

	for ( uint32_t i = 0; i < 3 ; i ++ ) {
		int MeasChannel = osc.GetMeasurementPointer(i)->GetChannel();
		int selection = osc.GetMeasurementPointer(i)->GetSelection();
		osc.GetMeasurementPointer(i)->SetMeasureResult(pScope->GetChannelByIndex(MeasChannel)->DoMeasurement(selection));
	}

	if (osc.GetTriggerPointer()->GetMode() == Trigger::AutoLevel)
	{
		osc.GetTriggerPointer()->SetLevel(pScope->GetTriggerLevel());
	}

	if (pScope->DidTrigger())
	{
		osc.GetTriggerPointer()->SetTriggerReceived(true);
		for(int i=0;i<HC_OSC_CHANNELS;i++) {
			double gain = 0.0, offset = 0.0;
			vector<char> qbytes = Quantizer::Quantize<char>(pScope->GetChannelByIndex(i)->GetResult(), gain, offset);
			//osc.GetChannel(i).SetGraph(pScope->GetChannelByIndex(i)->GetResult());
			osc.GetChannel(i).SetGraph(&qbytes[0], qbytes.size(), gain, offset);

		}
	}
	else
	{
		osc.GetTriggerPointer()->SetTriggerReceived(false);
	}
}

#if 0

	/*
	try {
		pScope->Acquire();
	} catch (BasicException e) {
		ExceptionHandled = true;
		if (e.errornr() == 0xBFFA2003) {
			if (loglevel > 4 ) ivilog << "No Trigger Received, trying to recover!" << endl;

			// timeout, no trigger received!
			int mode = osc.GetTriggerPointer()->GetMode();
			switch (mode) {
			case 0: // normal mode, return with no trigger
				TriggerStatus = false;
				goto OscDone;
			case 1: //auto
			case 2: //automode
				int32_t OldType  = pScope->GetTriggerType();
				double  OldLevel = pScope->GetTriggerLevel();
				pScope->SetTriggerType(6);
				if (loglevel > 4 ) ivilog << "Trying to Acquire() with immediate trigger" << endl;
				pScope->Acquire();

				CalculateMixMaxAvg(TriggerChannel,min,max,NewLevel);
				if (loglevel > 4 ) ivilog << "Found suitable trigger "<< NewLevel << endl;
				pScope->SetTriggerType(OldType);
				pScope->SetTriggerLevel(NewLevel);
				if (loglevel > 4 ) ivilog << "Aquire() with new trigger level" << endl;
				pScope->Acquire();

				if (mode==1) {
					if (loglevel > 4 ) ivilog << "Restoring old TriggerLevel " << OldLevel << endl;
					pScope->SetTriggerLevel(OldLevel);
					TriggerStatus = false;
				} else {
					if (loglevel > 4 ) ivilog << "Setting new TriggerLevel " << NewLevel << endl;                
					osc.GetTriggerPointer()->SetLevel(NewLevel);
				}
				break;
			} // end swtich 
		} else {
			if (loglevel > 4 ) ivilog << "Unknown Error, rethrowing "  << endl;
			throw (e);
		}
	}
	*/

	//writing some settings back.
	osc.GetTriggerPointer()->SetCoupling((Trigger::TriggerCoupling)pScope->GetTriggerCoupling());
	//osc.GetTriggerPointer()->SetDelay(pScope->GetTriggerHoldoff());

	/*CalculateMixMaxAvg(TriggerChannel,min,max,NewLevel);

	if (!ExceptionHandled) { // check for trigger errors on 5401
		if (osc.GetTriggerPointer()->GetMode()==2) osc.GetTriggerPointer()->SetLevel(NewLevel);
		if ((max < pScope->GetTriggerLevel()) || (min > pScope->GetTriggerLevel())){
			if (loglevel > 4 ) ivilog << "no trigger received!!!!" << endl;
			if (osc.GetTriggerPointer()->GetMode()==0) SendResult = false;
			if (osc.GetTriggerPointer()->GetMode()==1) TriggerStatus = false;
		}
	}*/

	if (SendResult) {
		for(int i=0;i<HC_OSC_CHANNELS;i++){
			/*char * charResult =  pScope->GetChannelByIndex(i)->GetCharResult();

			double gain = pScope->GetChannelByIndex(i)->GetCharResultGain();
			int length = pScope->GetChannelByIndex(i)->GetActualLength();
			double offset = pScope->GetChannelByIndex(i)->GetCharResultOffset();
			*/

			//osc.GetChannel(i).SetGraph(charResult,length,gain,offset);
			osc.GetChannel(i).SetGraph(pScope->GetChannelByIndex(i)->GetResult());
			//osc.GetChannel(i).SetVerticalOffset(offset);

			// weird stuff in oscilloscope.cpp ????
			//double      GetFlashSampleRate()              { return (1.0 /(RecordTime / (RequestLength/30.0))); } 
			//double d = (pScope->GetFlashSampleRate()/length)*10.0;
			//osc.SetSampleRate(d);

			/*if (osc.GetAutoScale()){
			osc.GetChannel(i).SetProbeAttenuation(pScope->GetChannelByIndex(i)->GetProbeAttenuation());
			osc.GetChannel(i).SetVerticalCoupling((Channel::ChannelCoupling)pScope->GetChannelByIndex(i)->GetVertCoupling());

			double vertrange = pScope->GetChannelByIndex(i)->GetCharResultRange();
			double f_vrange;

			if (vertrange < 0.05) f_vrange = (0.00625);

			if (vertrange > 0.05) f_vrange = (0.01);
			if (vertrange > 0.08) f_vrange = (0.02);
			if (vertrange > 0.16) f_vrange = (0.05);

			if (vertrange > 0.40) f_vrange = (0.1);
			if (vertrange > 0.80) f_vrange = (0.2);
			if (vertrange > 1.60) f_vrange = (0.5);

			if (vertrange >  4.0) f_vrange = (1);
			if (vertrange > 8.0) f_vrange = (2);
			if (vertrange > 10.0) f_vrange = (5);

			if (vertrange > 40.0) f_vrange = (6.25);
			osc.GetChannel(i).SetVerticalRange(f_vrange);
			}*/
		}   
	} else TriggerStatus = false;


OscDone:
	osc.GetTriggerPointer()->SetTriggerReceived(TriggerStatus);
	clock_t cend = clock();
	clock_t cdiff = cend - cbegin;
	double msec = ((double)cdiff * (double)1000) / CLOCKS_PER_SEC;
	if (loglevel > 4) ivilog << "took " << msec << "msec" << endl;


}


#endif