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
#ifndef __IVI_SCOPE_H__
#define __IVI_SCOPE_H__

#include <string>
#include <vector>

class cIVIScopeChannel;

#define COUP_AC          (0L)
#define COUP_DC          (1L)
#define COUP_GND         (2L)

class cIVIScope {
public:
	cIVIScope();
	~cIVIScope();
	int Init(std::string driver );

	int32_t Acquire();

	cIVIScopeChannel* GetChannelByIndex(uint32_t index);
	uint32_t AutoSetup(); 
	uint32_t AutoTrigger(); 
	uint32_t GetConfiguration();
	int Configure();

	//void SetAcqStartTime(double val)		{ mAcqStartTime = val; }
	//void SetAcqType(int32_t val)			{ mAcqType = val; }
	void SetTriggerHoldoff(double val)		{ mTriggerHoldoff = val; }
	void SetTriggerLevel(double val)		{ mTriggerLevel = val; }
	void SetTriggerSlope(int32_t val)		{ mTriggerSlope = val; }
	void SetTriggerSource(uint32_t val)	{ mTriggerSource = val; }
	void SetTriggerType(int32_t val)		{ mTriggerType = val; }
	void SetTriggerModifier(int32_t val)	{ mTriggerModifier = val; }
	void SetMaxTimemSec(int32_t val)		{ mMaxTimemSec = val; }
	void SetTriggerCoupling(int32_t val)	{ mTriggerCoupling = val; }  

	void SetRecordTime(double val)			{ mRecordTime = val; } 
	//void SetSampleRate(double val);
	void SetRequestLength(uint32_t val);

	int32_t	GetTriggerType()		{ return mTriggerType; }
	int32_t	GetTriggerModifier()	{ return mTriggerModifier; }

	int32_t	GetTriggerCoupling()	{ return mTriggerCoupling; }
	//int32_t	GetAcqType()			{ return mAcqType; }
	int32_t	GetTriggerSlope()		{ return mTriggerSlope; }
	//double	GetAcqStartTime()		{ return mAcqStartTime; }
	double	GetTriggerHoldoff()		{ return mTriggerHoldoff; }
	double	GetTriggerLevel()		{ return mTriggerLevel; }
	//double	GetSampleRate()			{ return mSampleRate; }
	double	GetRecordTime()			{ return mRecordTime; }     
	uint32_t	GetTriggerSource()	{ return mTriggerSource; }

	bool		DidTrigger()		{ return mDidTrigger; }
private:
	int  GetChannels();
	void GetError ( int32_t error ) ;

	uint32_t	mSession; 
	typedef std::vector <cIVIScopeChannel*> tChannels;
	tChannels mChannels;

	int32_t		mAcqType;
	double		mTriggerHoldoff; //
	double		mTriggerLevel;   //
	int32_t		mTriggerSlope;  //
	uint32_t	mTriggerSource;   //
	int32_t		mTriggerType; //
	int32_t		mRequestLength;  // number of samples in request   //  same??

	double		mSampleRate;     // samples per second //
	double		mRecordTime;     // duration of measurement // 
	int32_t		mMaxTimemSec;        // timeout
	int32_t		mTriggerCoupling;

	// this tries to immitate the behaviour fo the trigger modifier in ivi
	int32_t		mTriggerModifier;

	bool		mDidTrigger;
};

#endif

