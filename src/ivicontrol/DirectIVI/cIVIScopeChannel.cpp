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
#include "cIVIScopeChannel.h"

#include "../doerror.h"

#include "../3rdparty/stdint.h" 
#include <util/basic_exception.h>

#include <windows.h>
#include <mmsystem.h>

#pragma comment(lib, "winmm.lib")

#include <float.h>
using namespace std;

void cIVIScopeChannel::GetError(int32_t error) {
	char str[2048];
	IviScope_GetError(mSession, (ViStatus *) &error, 2048, str);
	IviScope_Abort(mSession);
	doerror(str,error);
}

int cIVIScopeChannel::Init(uint32_t aSession, uint32_t ReqLen){
	mSession = aSession;
	SetRequestLength(ReqLen);
	return 0;
}

int cIVIScopeChannel::Configure() {
	int32_t error = VI_SUCCESS;
	error = IviScope_ConfigureChannel(
		mSession, 
		mName.c_str(),
		mVertRange,
		mVertOffset,
		mVertCoupling,
		mProbeAttenuation,
		mEnabled
 		);
	if (error) GetError(error);

	error = IviScope_ConfigureChanCharacteristics(mSession, 
		mName.c_str(),
		mInputImpedance,
		mMaxFrequency);
	if (error) GetError(error);

	return 0;
}

cIVIScopeChannel::cIVIScopeChannel(char * aName, uint32_t aSession ) {
	mName				= aName;
	mSession			= aSession;
	mEnabled			= 1;
	mInputImpedance		= 1000000;
	mMaxFrequency		= 5000;   
	mProbeAttenuation	= 1;
	mVertCoupling		= IVISCOPE_VAL_DC;
	mVertRange			= 4;
	mVertOffset			= 0;
};

void cIVIScopeChannel::SetRequestLength( uint32_t length)
{
	mResult.resize(length);
}

double cIVIScopeChannel::DoMeasurement(int32_t measurement) {
	if (measurement == 4000) return 0.0;

	double MeasResult;
	int32_t error;

#ifndef _NI_EXT_
	if (measurement >= 1000) throw BasicException("NI Extentions have been disabled, cannot perform requested measurement!");
#endif

	error = IviScope_FetchWaveformMeasurement(mSession,mName.c_str(),measurement,&MeasResult);
	if (error) GetError(error);

	return MeasResult;
}


/*
Because the ivi standard doesn't allow us to have a sane timeout we have to poll the AcquisitionStatus
and handle the timeout ourselves

The problem is that is can't handle timeouts below 1000 ms

*/
int cIVIScopeChannel::FetchWaveform(bool initiate, size_t requestLength, uint32_t timeoutms)
{
	int error = 0;
	int32_t lActualLength;
	double lAcqStartTime,lSampleTime;

	int32_t status = 0;
	error = IviScope_AcquisitionStatus(mSession, (ViInt32*) &status);
	if (error) return error;

	if (status != IVISCOPE_VAL_ACQ_COMPLETE)
	{
		error = IviScope_Abort(mSession);
		if (error) GetError(error);
	}

	if (initiate)
	{
		uint32_t start = timeGetTime(); // warning for windows specific code..
		error = IviScope_InitiateAcquisition(mSession);

		bool acqDone = false;
		bool done = false;

		while(!done)
		{
			error = IviScope_AcquisitionStatus(mSession, (ViInt32*) &status);
			if (error) return error;

			if (status == IVISCOPE_VAL_ACQ_COMPLETE)
			{
				acqDone = true;
				done = true;
				continue;
			}

			uint32_t end = timeGetTime();
			uint32_t timediff = (end < start) ? (start + end + 1) : (end - start); // this may be wrong
			if (timediff > 10000) throw BasicException("Wonky wraparound code");
			if (timediff > timeoutms) done = true;

			//ivilog << "Acquisition: " << timediff << endl;
			Sleep(1);
		}

		if (!acqDone) return IVISCOPE_ERROR_MAX_TIME_EXCEEDED;
	}

	SetRequestLength(requestLength);
	double* pGraph = (double*)&(*mResult.begin());

	error = IviScope_FetchWaveform(
		mSession,
		mName.c_str(),
		requestLength,
		pGraph,
		(ViInt32*) &lActualLength,
		(ViReal64*) &lAcqStartTime,
		(ViReal64*) &lSampleTime
		);
	if (error) GetError(error);
	SetActualLength(lActualLength);
	return error;
}

double cIVIScopeChannel::EstimateTriggerLevel()
{
	// could also be done through measuring min and max directly

	if (mResult.empty()) return 0.0;
	double min = DBL_MAX;
	double max = DBL_MIN;
	for (size_t i = 0; i < mResult.size(); i++)
	{
		double sample = mResult[i];
		if (sample < min) min = sample;
		if (sample > max) max = sample;
	}

	return ((max + min) / 2.0);
}

uint32_t cIVIScopeChannel::GetConfiguration()
{
	double lInputImpedance, lMaxFrequency, lVertRange, lVertOffset;
	int32_t lVertCoupling;

	IviScope_GetAttributeViReal64(mSession, mName.c_str(), 
		IVISCOPE_ATTR_INPUT_IMPEDANCE, (ViReal64*)(&lInputImpedance));

	SetInputImpedance(lInputImpedance);

	IviScope_GetAttributeViReal64(mSession, mName.c_str(), 
		IVISCOPE_ATTR_MAX_INPUT_FREQUENCY, (ViReal64*)(&lMaxFrequency));

	SetMaxFrequency(lMaxFrequency);

	IviScope_GetAttributeViReal64(mSession, mName.c_str(), 
		IVISCOPE_ATTR_VERTICAL_RANGE, (ViReal64*)(&lVertRange));

	SetVertRange(lVertRange);

	IviScope_GetAttributeViReal64(mSession, mName.c_str(), 
		IVISCOPE_ATTR_VERTICAL_OFFSET, (ViReal64*)(&lVertOffset));

	SetVertOffset(lVertOffset);

	IviScope_GetAttributeViInt32(mSession, mName.c_str(), 
		IVISCOPE_ATTR_VERTICAL_COUPLING, (ViInt32*)(&lVertCoupling));

	SetVertCoupling(lVertCoupling);

	return 0;
}