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
* Contributor(s):
* André van Schoubroeck
*/

#include <iviFgen.h>
#include "../log.h"
#include "cIVIFGEN.h"
#include "../doerror.h"

using namespace std;

cFGENChannel::cFGENChannel(char * aName, uint32_t aSession ) {
	mName          = aName;
	mSession       = aSession;
	mEnabled       = 0;
	mAmplitude     = 1;
	mDCOffset      = 0;
	mDutyCycleHigh = 50;
	mFrequency     = 1000;
	mStartPhase    = 0;
	mWaveform      = WF_SINE;  
}

void cFGENChannel::GetError (int32_t error) {
	char str[2048];
	IviFgen_GetError(mSession, (ViStatus *)&error, 2048,str); //what ahppens
	doerror(str,error);
}

int cFGENChannel::Init() {
	int32_t error;
	error = IviFgen_ConfigureOperationMode(mSession, mName.c_str(), IVIFGEN_VAL_OPERATE_CONTINUOUS);
	return error;
}

void cFGENChannel::SetEnabled(uint16_t val) {
	mEnabled = val;
}

void cFGENChannel::SetAmplitude(double val) {
	mAmplitude = val;
}

void cFGENChannel::SetDCOffset(double val) {
	mDCOffset = val;
}

void cFGENChannel::SetOutputImpedance(double val) {
	mOutputImpedance = val;
	int32_t error;
	error = IviFgen_SetAttributeViReal64(mSession, 
		mName.c_str(),
		IVIFGEN_ATTR_OUTPUT_IMPEDANCE , 
		(ViReal64)mOutputImpedance);
	if (error) GetError(error);
}

void cFGENChannel::SetDutyCycleHigh(double val) {
	mDutyCycleHigh = val;
	int32_t error;

	error = IviFgen_SetAttributeViReal64(mSession, 
		mName.c_str(),
		IVIFGEN_ATTR_FUNC_DUTY_CYCLE_HIGH, 
		(ViReal64)mDutyCycleHigh);
	if (error) GetError(error);
}

void cFGENChannel::SetFrequency(double val) {
	mFrequency = val;
}

void cFGENChannel::SetStartPhase(double val) {
	mStartPhase = val;
}

void cFGENChannel::SetWaveform(int32_t val) {
	mWaveform = val;
}

int32_t cFGENChannel::SetOutput() {
	// Typo in official documentation page 49, it says IviFgen_ConfigureStandardWaveform
	int32_t error;
	error  = IviFgen_AbortGeneration(mSession); // needed for ni5402 but not for ni5401!
	if (error) GetError(error);

	error = IviFgen_ConfigureStandardWaveform (mSession, 
		mName.c_str(), mWaveform, mAmplitude, mDCOffset, mFrequency, mStartPhase); 
	if (error) GetError(error);
	error = IviFgen_InitiateGeneration(mSession);
	if (error) GetError(error);
	return error;
}

/////////////

cIVIFGEN::cIVIFGEN() {
	ivilog.Log(5) << "cIVIFGEN::cIVIFGEN()" << endl;
}

cIVIFGEN::~cIVIFGEN() {
	IviFgen_close(Session);
	while(!channels.empty()) {
		delete channels.back();
		channels.pop_back();
	}
}

void cIVIFGEN::GetError (int32_t error) {
	char str[2048];
	IviFgen_GetError(Session, (ViStatus *)&error, 2048,str); //what ahppens
	doerror(str,error);
}

int32_t cIVIFGEN::Init(std::string driver) {
	ivilog.Log(5) << "cIVIFGEN::Init(" << driver <<")" << endl;

	int32_t error = VI_SUCCESS;
	uint32_t lSession;
	error = IviFgen_init ( (ViRsrc)driver.c_str() , VI_TRUE, VI_TRUE, (ViSession*)&lSession);
	Session = lSession;
	if (error) GetError(error);
	GetChannels();
	return error;
}

cFGENChannel *cIVIFGEN::GetChannelByIndex(uint32_t index) {
	if (index < channels.size() ) {
		return channels[index];
	}
	return NULL;
}

int cIVIFGEN::GetChannels() {
	int32_t i = 1 ;
	char buffer[2048];
	int32_t error = VI_SUCCESS;
	int32_t nr;

	error = IviFgen_GetAttributeViInt32(Session, "", IVIFGEN_ATTR_CHANNEL_COUNT, (ViInt32*)&nr);
	if (error) GetError(error);
	while (i <= nr) {
		error = IviFgen_GetChannelName ( Session, i , 2048 , buffer );
		if (error) {
			GetError(error);
		} else {
			cFGENChannel* pChannel = new cFGENChannel(buffer,Session);
			channels.push_back(pChannel);
			pChannel->Init();
		}
		i++;
	}
	return i-1;  
}