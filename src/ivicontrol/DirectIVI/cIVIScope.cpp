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

#include <iviScope.h>

#include "../doerror.h"
#include "cIVIScope.h"
#include "../3rdparty/stdint.h"
#include "cIVIScopeChannel.h"
#include "../log.h"

#include <util/basic_exception.h>
using namespace std;

cIVIScope::cIVIScope() {
	ivilog.Log(5) << "cIVIScope::cIVIScope()" << endl;

	mRequestLength   = 500; 
	//mAcqStartTime    = 0;
	mAcqType         = IVISCOPE_VAL_NORMAL  ;
	mTriggerHoldoff  = 0; 
	mTriggerLevel    = 0;   
	mTriggerSlope    = IVISCOPE_VAL_POSITIVE ;  

	mTriggerType     = IVISCOPE_VAL_EDGE_TRIGGER; 
	mSampleRate      = 10000;
	mRecordTime      = (1 / mSampleRate) *  mRequestLength; 
	mMaxTimemSec     = 100;        // 0.1 sec 
//	mSampleTime      = 1 / mSampleRate;
	mTriggerCoupling = IVISCOPE_VAL_DC;

	mTriggerModifier = IVISCOPE_VAL_AUTO;
	mDidTrigger = false;
}

cIVIScope::~cIVIScope() {
	while(!mChannels.empty()) {
		delete mChannels.back();
		mChannels.pop_back();
	}
	IviScope_close(mSession);
}

int32_t cIVIScope::Init(std::string  driver) {
	ivilog.Log(5) << "cIVIScope::Init(" << driver << ")" << endl;

	int32_t error = VI_SUCCESS;
	uint32_t lSession;
	error = IviScope_init ( (ViRsrc)driver.c_str() , VI_TRUE, VI_TRUE, (ViSession*)&lSession);

	mSession=lSession;
	if (!error) {
		GetChannels();
		mTriggerSource = 0;
	} else {
		GetError(error);
	}

	char capString[1024];
	IviScope_GetAttributeViString(mSession, "", IVISCOPE_ATTR_GROUP_CAPABILITIES, 1023, capString);

	return error; 
}

void cIVIScope::GetError (int32_t error) {
	char str[2048];
	IviScope_GetError(mSession, (ViStatus *)&error, 2048,str); //what ahppens
	IviScope_Abort(mSession);
	doerror(str,error);
}

int cIVIScope::GetChannels() {
	int32_t i = 1 ;
	char buffer[2048];

	int32_t error = VI_SUCCESS;
	int32_t nr;

	IviScope_GetAttributeViInt32(mSession, "", IVISCOPE_ATTR_CHANNEL_COUNT, (ViInt32*)&nr);
	while (i <= nr) {
		error = IviScope_GetChannelName(mSession, i , 2048 , buffer );
		if (error) GetError(error);
		cIVIScopeChannel* pChannel = new cIVIScopeChannel(buffer, mSession);
		mChannels.push_back(pChannel);
		pChannel->Init(mSession, 500);
		i++;
	}
	return i-1;  
}

int cIVIScope::Configure () {
	int32_t error;

	// unnecessary?
	error = IviScope_ConfigureAcquisitionType(mSession, mAcqType);
	if (error) GetError(error);

	double aAcqStartTime = - mRecordTime/2;
	error = IviScope_ConfigureAcquisitionRecord(mSession, mRecordTime, mRequestLength, aAcqStartTime);
	if (error) GetError(error);

	error = IviScope_ConfigureTriggerCoupling(mSession, mTriggerCoupling);
	if (error) GetError(error);

	error = IviScope_ConfigureEdgeTriggerSource(
		mSession,
		mChannels[mTriggerSource]->GetName().c_str(),
		mTriggerLevel,
		mTriggerSlope);
	if (error) GetError(error);

	error = IviScope_ConfigureTrigger(mSession, mTriggerType, mTriggerHoldoff); 
	if (error) GetError(error);

	//GetConfiguration();
	return 0;
}

int32_t cIVIScope::Acquire() {
	uint32_t i;
	int32_t error;

	for ( i = 0 ; i < mChannels.size() ; i++ ) mChannels[i]->Configure();
	Configure();

	mDidTrigger = true;

	error = mChannels[0]->FetchWaveform(true, mRequestLength, mMaxTimemSec);
	if (error == IVISCOPE_ERROR_MAX_TIME_EXCEEDED)
	{
		// probably no trigger
		switch(mTriggerModifier)
		{
		case IVISCOPE_VAL_NO_TRIGGER_MOD:
			mDidTrigger = false;
			return 0;
			break;

		case IVISCOPE_VAL_AUTO:
			// make a immediate trigger measurement
			error = IviScope_ConfigureTrigger(mSession, IVISCOPE_VAL_IMMEDIATE_TRIGGER, mTriggerHoldoff); 
			if (error) GetError(error);
			// and fetch on the first channel
			error = mChannels[0]->FetchWaveform(true, mRequestLength, mMaxTimemSec);
			if (error) GetError(error);
			break;

		case IVISCOPE_VAL_AUTO_LEVEL:
			{
				// set immediate trigger for the measurements
				error = IviScope_ConfigureTrigger(mSession, IVISCOPE_VAL_IMMEDIATE_TRIGGER, mTriggerHoldoff); 
				if (error) GetError(error);

				error = mChannels[0]->FetchWaveform(true, mRequestLength, mMaxTimemSec);
				if (error) GetError(error);

				// set the new estimated level and try another measurement
				mTriggerLevel = mChannels[mTriggerSource]->EstimateTriggerLevel();

				// reset the trigger type
				error = IviScope_ConfigureTrigger(mSession, mTriggerType, mTriggerHoldoff); 
				if (error) GetError(error);

				// and set the new level
				error = IviScope_ConfigureEdgeTriggerSource(
					mSession,
					mChannels[mTriggerSource]->GetName().c_str(),
					mTriggerLevel,
					mTriggerSlope);
				if (error) GetError(error);

				error = mChannels[0]->FetchWaveform(true, mRequestLength, mMaxTimemSec);
				if (error == IVISCOPE_ERROR_MAX_TIME_EXCEEDED)
				{
					// last change, no trigger even with the new level
					// make a final immediate measurement then give up
					mDidTrigger = false;
					error = IviScope_ConfigureTrigger(mSession, IVISCOPE_VAL_IMMEDIATE_TRIGGER, mTriggerHoldoff); 
					if (error) GetError(error);

					error = mChannels[0]->FetchWaveform(true, mRequestLength, mMaxTimemSec);
					if (error) GetError(error);
				}
				else if (error) GetError(error);
			}
			break;
		default:
			throw BasicException("Unknown trigger modifier");
		}
	}
	else if (error != VI_SUCCESS)
	{
		GetError(error);
		return error;
	}

	mDidTrigger = true;

	// success, fetch the other channels as well

	for ( i = 1 ; i < mChannels.size() ; i++ ) {
		error = mChannels[i]->FetchWaveform(false, mRequestLength, mMaxTimemSec);
		if (error) GetError(error);
	}

	return 0;
}

cIVIScopeChannel *cIVIScope::GetChannelByIndex(uint32_t index) {
	if (index < mChannels.size() ) {
		return mChannels[index];
	}
	return NULL;
}

void cIVIScope::SetRequestLength( uint32_t length) {
	mRequestLength = length;
	uint32_t i;
	for ( i = 0 ; i < mChannels.size(); i++ ) mChannels[i]->SetRequestLength(length);
}

/*void cIVIScope::SetSampleRate(double val) { 
	mSampleRate = val;
	mRecordTime = (1 / mSampleRate) *  mRequestLength; 
}*/

uint32_t cIVIScope::GetConfiguration() {
	int32_t /*lAcqType,*/       lTriggerCoupling, lTriggerSlope, lTriggerType;
	double  /*lAcqStartTime,*/  lTriggerHoldoff,  lTriggerLevel, lSampleRate; 

/*	IviScope_GetAttributeViString(mSession, "", 
		IVISCOPE_ATTR_TRIGGER_SOURCE, 256, (ViChar*)str);
	mTriggerSource= str;
*/

	/*IviScope_GetAttributeViInt32(mSession, "", 
		IVISCOPE_ATTR_ACQUISITION_TYPE, (ViInt32*)&lAcqType);
	mAcqType=lAcqType;
	*/

	IviScope_GetAttributeViInt32(mSession, "", 
		IVISCOPE_ATTR_TRIGGER_COUPLING, (ViInt32*)&lTriggerCoupling);
	mTriggerCoupling=lTriggerCoupling;

	IviScope_GetAttributeViInt32(mSession, "", 
		IVISCOPE_ATTR_TRIGGER_SLOPE, (ViInt32*)&lTriggerSlope);
	mTriggerSlope=lTriggerSlope;

	IviScope_GetAttributeViInt32(mSession, "", 
		IVISCOPE_ATTR_TRIGGER_TYPE, (ViInt32*)&lTriggerType);
	mTriggerType=lTriggerType;

/*
	IviScope_GetAttributeViReal64(mSession, "", 
		IVISCOPE_ATTR_ACQUISITION_START_TIME, (ViReal64*)&lAcqStartTime);
	mAcqStartTime=lAcqStartTime;
*/

	IviScope_GetAttributeViReal64(mSession, "", 
		IVISCOPE_ATTR_TRIGGER_HOLDOFF, (ViReal64*)&lTriggerHoldoff);
	mTriggerHoldoff=lTriggerHoldoff;

	IviScope_GetAttributeViReal64(mSession, "", 
		IVISCOPE_ATTR_TRIGGER_LEVEL, (ViReal64*)&lTriggerLevel);
	mTriggerLevel=lTriggerLevel;

	IviScope_GetAttributeViReal64(mSession, "", 
		IVISCOPE_ATTR_HORZ_SAMPLE_RATE, (ViReal64*)&lSampleRate);
	mSampleRate=lSampleRate;

	for( uint32_t i = 0 ; i < mChannels.size() ; i++ ) {
		mChannels[i]->GetConfiguration();
	}
	return 0;
}

uint32_t cIVIScope::AutoSetup() {
	return 0; // just disable this for now.. its insanely wrong

	double vmin, vmax, vrange;

	for (uint32_t i = 0 ; i < mChannels.size(); i++ ) {
		try {
			vmin = mChannels[i]->DoMeasurement(7);
			vmax = mChannels[i]->DoMeasurement(6);
			vrange = vmax-vmin; 
			if (vrange < ( 0.01   * 8 ) ) vrange = ( 0.01 * 8 );
			else if (vrange < ( 0.02   * 8 ) ) vrange = ( 0.02 * 8 );
			else if (vrange < ( 0.05   * 8 ) ) vrange = ( 0.05 * 8 );
			else if (vrange < ( 0.1    * 8 ) ) vrange = ( 0.1 * 8 );
			else if (vrange < ( 0.2    * 8 ) ) vrange = ( 0.2 * 8 );
			else if (vrange < ( 0.5    * 8 ) ) vrange = ( 0.5 * 8 );
			else if (vrange < ( 1.0    * 8 ) ) vrange = ( 1.0 * 8 );
			else if (vrange < ( 2.0    * 8 ) ) vrange = ( 2.0 * 8 );
			else vrange = ( 5.0 * 8 );

			mChannels[i]->SetVertRange(vrange);
			mChannels[i]->SetVertOffset((vmax+vmin)/2);
			mChannels[i]->Configure();
		} catch (BasicException e) {
			ivilog.Log(1) << "ERROR! Failed to measure voltage range!\n";
		}
	}

	// Need to set a valid time per division
	double period0 = 0.00, period1 = 0.00, NewHrange = 0.01;
	bool valid0=true, valid1=true;
	try {
		period0 = mChannels[0]->DoMeasurement(3);
	} catch (BasicException e) {
		valid0=false; // measurement failed
	}

	try {
		period1 = mChannels[1]->DoMeasurement(3);
	} catch (BasicException e) {
		valid1=false; // measurement failed
	}

	if ( valid0 && valid1 ) {
		if (period0 > period1 ) NewHrange = period0; else NewHrange = period1;
	} else if (valid0) NewHrange = period0; else if (valid1) NewHrange = period1;
	else {/*Seems there is no valid signal on the scope??*/ NewHrange = 0.01;}

	// now we have a valid period, fit the range to a valid range for the flash client
	NewHrange /= 4 ; 

	ivilog.Log(5)
		<< "Auto Setup Status:" << endl
		<< "Channel 0 Valid " << valid0 << " value " << period0 << endl
		<< "Channel 1 Valid " << valid1 << " value " << period1 << endl;

	if (NewHrange < 0.000002)   NewHrange = 0.000002  * ( 500.0 / 30.0);
	else if (NewHrange < 0.000005)   NewHrange = 0.000005  * ( 500.0 / 30.0);
	else if (NewHrange < 0.00001)    NewHrange = 0.00001   * ( 500.0 / 30.0) ;
	else if (NewHrange < 0.00002)    NewHrange = 0.00002   * ( 500.0 / 30.0) ;
	else if (NewHrange < 0.00005)    NewHrange = 0.00005   * ( 500.0 / 30.0) ;
	else if (NewHrange < 0.0001)     NewHrange = 0.0001    * ( 500.0 / 30.0) ;
	else if (NewHrange < 0.0002)     NewHrange = 0.0002    * ( 500.0 / 30.0) ;
	else if (NewHrange < 0.0005)     NewHrange = 0.0005    * ( 500.0 / 30.0) ;
	else if (NewHrange < 0.002)      NewHrange = 0.001     * ( 500.0 / 30.0) ;
	else if (NewHrange < 0.005)      NewHrange = 0.002     * ( 500.0 / 30.0) ;
	else if (NewHrange < 0.01)       NewHrange = 0.005     * ( 500.0 / 30.0) ;
	else if (NewHrange < 0.02)       NewHrange = 0.01      * ( 500.0 / 30.0) ;
	else if (NewHrange < 0.02)       NewHrange = 0.02      * ( 500.0 / 30.0) ;
	else if (NewHrange < 0.05)       NewHrange = 0.05      * ( 500.0 / 30.0) ;
	else if (NewHrange < 0.1)        NewHrange = 0.1       * ( 500.0 / 30.0) ;


	//
	mRecordTime = NewHrange;
//	mAcqStartTime = - mRecordTime/2;
//	mSampleTime= 1 / mSampleRate;

	return 0;
}

