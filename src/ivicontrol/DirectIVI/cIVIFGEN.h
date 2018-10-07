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
#ifndef __IVI_FGEN_H__
#define __IVI_FGEN_H__

#include <vector>
#include <string>
#include "../3rdparty/stdint.h" 

#define WF_SINE      (1L)                         
#define WF_SQUARE    (2L)                         
#define WF_TRIANGLE  (3L)                        
#define WF_RAMP_UP   (4L)                       
#define WF_RAMP_DOWN (5L)                      
#define WF_DC        (6L)     

// NI
#define WF_NOISE     (101L)
#define WF_USER      (102L)


class cFGENChannel {
public:
	cFGENChannel(char * aName, uint32_t aSession );
	int Init();

	void GetError ( int32_t error ) ;

	void SetEnabled(uint16_t val);
	void SetAmplitude(double val);
	void SetDCOffset(double val);
	void SetDutyCycleHigh(double val);
	void SetFrequency(double val);
	void SetStartPhase(double val);
	void SetOutputImpedance(double val);
	void SetWaveform(int32_t val);  
	int32_t SetOutput();
private:
	std::string	mName;
	uint16_t	mEnabled;
	double		mAmplitude;
	double		mDCOffset;
	double		mDutyCycleHigh;
	double		mFrequency;
	double		mStartPhase;
	double		mOutputImpedance;
	int32_t		mWaveform;        
	uint32_t	mSession;
};

class cIVIFGEN {
public:
	cIVIFGEN();
	~cIVIFGEN();
	int Init(std::string  driver );

	void GetError ( int32_t error ) ;

	cFGENChannel * GetChannelByIndex(uint32_t index);
private:
	uint32_t Session; 
	int  GetChannels();
	typedef std::vector <cFGENChannel*> tChannels;
	tChannels channels;
};

#endif
