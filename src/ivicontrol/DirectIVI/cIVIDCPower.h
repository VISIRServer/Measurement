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
#ifndef __IVI_POWER_H__
#define __IVI_POWER_H__

#include <vector>
#include <string>
#include "../3rdparty/stdint.h" 

class cIVIDCPower;

class cDCPowerChannel {
public:
	cDCPowerChannel(char * aName, uint32_t aSession );
	int Init();

	int SetVoltage(double Voltage);
	int SetCurrent(double Current);
	int SetEnabled(uint16_t aEnabled);

	double GetOutputVoltage();
	double GetOutputCurrent();
	int    GetOutputLimited();

	std::string GetType() { return mType; }
private:
	void		GetError(int32_t error);
	int32_t		SetOutput();

	std::string		mName;
	std::string		mType;
	uint16_t		mEnabled;
	double			mVoltageRange;
	double			mCurrentRange;
	double			mCurrentLimit;
	int32_t			mCurrentBehavior;
	double			mVoltageLevel;
	double			mOVP;
	uint16_t		mOVPenabled;
	double			mMaxCurrentLimit;
	double			mMaxVoltageLevel;
	uint32_t		mSession;
};

class cIVIDCPower  {
public:
	cIVIDCPower();
	~cIVIDCPower();
	int Init(std::string driver);

	cDCPowerChannel* GetChannelByType(std::string aType);
	cDCPowerChannel* GetChannelByIndex(uint32_t index);
private:
	void GetError(int32_t error);
	int  GetChannels();

	typedef std::vector<cDCPowerChannel*> tChannels;
	tChannels mChannels;
	uint32_t mSession; 
};

#endif
