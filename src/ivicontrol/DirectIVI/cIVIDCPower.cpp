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

#include <IviDCPwr.h>

#include "../3rdparty/stdint.h" 
#include "../doerror.h"
#include "cIviDCPower.h"

#include "../log.h"

using namespace std; 

//-----------------------------------------------------------------------------
void cIVIDCPower::GetError (int32_t error) {
	char str[2048];
	ViStatus rv = IviDCPwr_GetError(mSession, (ViStatus *)&error, 2048, str);
	if (rv < 0) doerror("Unknown error");
	else doerror(str,error);
}
//-----------------------------------------------------------------------------
void cDCPowerChannel::GetError (int32_t error) {
	char str[2048];
	IviDCPwr_GetError(mSession, (ViStatus *)&error, 2048,str);
	doerror(str,error);
}
//-----------------------------------------------------------------------------
cIVIDCPower::cIVIDCPower(){
	mSession = NULL;
	ivilog.Log(5) << "cIVIDCPower::cIVIDCPower()" << endl;
}
//-----------------------------------------------------------------------------
cIVIDCPower::~cIVIDCPower(){
	while(!mChannels.empty()){
		delete mChannels.back();
		mChannels.pop_back();
	}
	IviDCPwr_close(mSession);
}
//-----------------------------------------------------------------------------
int32_t cIVIDCPower::Init(std::string  driver){
	ivilog.Log(5) << "cIVIDCPower::Init(" << driver << ")" << endl;

	int32_t error = VI_SUCCESS;
	uint32_t lSession;
	error = IviDCPwr_init( (ViRsrc)driver.c_str(), VI_TRUE, VI_TRUE, (ViSession*)&lSession);
	mSession = lSession;
	if (error) GetError(error);
	GetChannels();
	return error; 
}
//-----------------------------------------------------------------------------
cDCPowerChannel *cIVIDCPower::GetChannelByType(std::string aType) {
	cDCPowerChannel* pChannel = NULL;
	for (uint32_t i = 0 ; i < mChannels.size(); i++ ) {
		if ( mChannels[i]->GetType() == aType) {
			pChannel  = mChannels [i];
			break;
		}
	}
	return pChannel;
}
//-----------------------------------------------------------------------------
cDCPowerChannel *cIVIDCPower::GetChannelByIndex(uint32_t index) {
	if (index < mChannels.size() ) {
		return mChannels[index];
	}
	return NULL;
}
//-----------------------------------------------------------------------------
int cIVIDCPower::GetChannels(){
	int32_t i = 1 ;
	char buffer[2048];

	int32_t status = VI_SUCCESS;
	int32_t nr;

	IviDCPwr_GetAttributeViInt32(mSession, "", IVIDCPWR_ATTR_CHANNEL_COUNT, (ViInt32*)&nr);
	while (i <= nr) {
		status = IviDCPwr_GetChannelName(mSession, i, 2048, buffer);
		if (status) GetError(status);

		if(status) { 
			IviDCPwr_error_message(mSession, status, buffer);
			doerror(buffer);
		} else {
			cDCPowerChannel* pChannel = new cDCPowerChannel(buffer, mSession);
			mChannels.push_back(pChannel);
			pChannel->Init();
		}
		i++;
	}
	return i-1;
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
cDCPowerChannel::cDCPowerChannel(char * aName, uint32_t aSession ) {
	mSession			= aSession;
	mName				= aName;
	mEnabled			= VI_FALSE;
	mCurrentLimit		= 0.1;
	mCurrentBehavior	= IVIDCPWR_VAL_CURRENT_REGULATE;
	mVoltageLevel		= 0;
	mOVP				= 0;
	mOVPenabled			= VI_FALSE;
}

//-----------------------------------------------------------------------------
int cDCPowerChannel::Init() {
	double lMaxCurrentLimit, lMaxVoltageLevel;

	IviDCPwr_QueryMaxCurrentLimit(mSession, mName.c_str(), 0, &lMaxCurrentLimit);
	mMaxCurrentLimit = lMaxCurrentLimit;
	mCurrentRange    = mMaxCurrentLimit/2;

	IviDCPwr_QueryMaxVoltageLevel(mSession, mName.c_str(), mCurrentRange, &lMaxVoltageLevel);
	mMaxVoltageLevel = lMaxVoltageLevel;
	mVoltageRange    = mMaxVoltageLevel/2;

	if ( (mMaxVoltageLevel>5) && (mMaxVoltageLevel<7) ) {
		mType = "+6V";
	} else if (mMaxVoltageLevel>15) {
		mType = "+20V";
	} else if (mMaxVoltageLevel<-15) {
		mType = "-20V";
	} else mType = "???";
	//debug
	ivilog.Log(5)
		<< "Channel Name            " << mName << endl
		<< "Channel MaxVoltageLevel " << mMaxVoltageLevel << endl
		<< "Channel Type            " << mType << endl;

	SetOutput();
	return 0;
}
//-----------------------------------------------------------------------------
int32_t cDCPowerChannel::SetOutput() {
	int32_t error = VI_SUCCESS;
	error = IviDCPwr_ConfigureOutputRange(mSession, mName.c_str() , IVIDCPWR_VAL_RANGE_CURRENT, mCurrentRange ) ; // page 26
	if (error != VI_SUCCESS) return error;
	error = IviDCPwr_ConfigureOutputRange(mSession, mName.c_str() , IVIDCPWR_VAL_RANGE_VOLTAGE, mVoltageRange ) ;
	if (error != VI_SUCCESS) return error;
	error = IviDCPwr_ConfigureCurrentLimit(mSession, mName.c_str(), mCurrentBehavior, mCurrentLimit);
	if (error != VI_SUCCESS) return error;
	error = IviDCPwr_ConfigureOVP(mSession, mName.c_str(), mOVPenabled, mOVP); 
	if (error != VI_SUCCESS) return error;
	error = IviDCPwr_ConfigureVoltageLevel(mSession, mName.c_str(), mVoltageLevel);
	if (error != VI_SUCCESS) return error;
	error = IviDCPwr_ConfigureOutputEnabled(mSession, mName.c_str(), mEnabled) ;
	return error;
}
//-----------------------------------------------------------------------------
int cDCPowerChannel::SetVoltage(double voltage) {
	if (mMaxVoltageLevel > 0 ) { // positive output channel
		if ( voltage < mMaxVoltageLevel ) {
			mVoltageLevel = voltage;
		} else return -1;
	} else if ( voltage > mMaxVoltageLevel ) { // negative output channel
		mVoltageLevel = voltage;
	} else return -1;

	int32_t error = SetOutput();
	if (error) GetError(error);
	return error;
}
//-----------------------------------------------------------------------------
int cDCPowerChannel::SetCurrent(double current) {
	if ( current < mMaxCurrentLimit ) {
		mCurrentLimit = current;
	} else return -1;

	int32_t error = SetOutput();
	if (error) GetError(error);
	return error;

}
//-----------------------------------------------------------------------------
int cDCPowerChannel::SetEnabled(uint16_t aEnabled) {
	mEnabled = aEnabled;
	int32_t error = SetOutput();
	if (error) GetError(error);
	return error;
}
//-----------------------------------------------------------------------------
double cDCPowerChannel::GetOutputCurrent() {
	double outputCurrent = 0; 
	int32_t error;
	error = IviDCPwr_Measure (mSession, mName.c_str(), IVIDCPWR_VAL_MEASURE_CURRENT ,&outputCurrent);
	if (error) GetError(error);
	return outputCurrent;
}
//-----------------------------------------------------------------------------
double cDCPowerChannel::GetOutputVoltage() {
	double outputVoltage = 0; 
	int32_t error;
	error = IviDCPwr_Measure(mSession, mName.c_str(), IVIDCPWR_VAL_MEASURE_VOLTAGE ,&outputVoltage);
	if (error) GetError(error);
	return outputVoltage;
}
//-----------------------------------------------------------------------------
int cDCPowerChannel::GetOutputLimited(){
	uint16_t limited;
	int32_t error;
	error = IviDCPwr_QueryOutputState(mSession, mName.c_str(), IVIDCPWR_VAL_OUTPUT_CONSTANT_CURRENT , &limited); 
	if (error) GetError(error);
	return limited;
}
//-----------------------------------------------------------------------------
