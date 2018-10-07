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

#pragma once
#ifndef __IVI_SCOPE_CHANNEL_H__
#define __IVI_SCOPE_CHANNEL_H__

#include "../3rdparty/stdint.h" 
#include <string>
#include <vector>

/*#define COUP_AC          (0L)
#define COUP_DC          (1L)
#define COUP_GND         (2L)
*/

class cIVIScopeChannel {
public:
	typedef std::vector<double> tResult;
		
	cIVIScopeChannel(char * aName, uint32_t aSession );
	int Init(uint32_t aSession, uint32_t ReqLen);

	void SetRequestLength(uint32_t length);

	int Configure();

	double DoMeasurement(int32_t measurement);

	std::string		GetName()			{ return mName;}
	const tResult&	GetResult()			{ return mResult; }
	int32_t			GetActualLength()	{ return mActualLength; }

	uint16_t	GetEnabled()                    { return  mEnabled; }
	int32_t		GetVertCoupling()               { return  mVertCoupling; }
	double		GetInputImpedance()             { return  mMaxFrequency; }
	double		GetMaxFrequency()               { return  mMaxFrequency; }
	double		GetProbeAttenuation()           { return  mProbeAttenuation; }
	double		GetVertRange()                  { return  mVertRange; }
	double		GetVertOffset()                 { return  mVertOffset; }

	void	SetActualLength(int32_t val)    { mActualLength = val ; }
	void	SetVertOffset(double val)       { mVertOffset = val; }
	void	SetVertRange(double val)        { mVertRange = val; }
	void	SetProbeAttenuation(double val) { mProbeAttenuation = val; }
	void	SetMaxFrequency(double val)     { mMaxFrequency = val; }
	void	SetInputImpedance(double val)   { mMaxFrequency = val; }
	void	SetVertCoupling(int32_t val)    { mVertCoupling = val; }
	void	SetEnabled(uint16_t val)        { mEnabled = val; }

	int		FetchWaveform(bool initiate, size_t requestLength, uint32_t timeoutms);
	double	EstimateTriggerLevel();

	uint32_t GetConfiguration();
private:
	void GetError ( int32_t error );

	std::string		mName;
	uint32_t		mSession;
	
	tResult mResult;

	uint16_t      mEnabled;

	double        mInputImpedance;
	double        mMaxFrequency;   // enables bandwith limit
	double        mProbeAttenuation;

	int32_t       mVertCoupling;
	double        mVertRange;
	double        mVertOffset;

	int32_t       mActualLength;   // number of samples in returnvalue
};

#endif
