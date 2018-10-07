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
 * Copyright (c) 2004-2009 Johan Zackrisson
 * All Rights Reserved.
 */

#ifndef __SIGNALANALYZERCHANNEL_H__
#define __SIGNALANALYZERCHANNEL_H__

#include "instrument.h"

#include <setget.h>
#include <string>

class SignalAnalyzerChannel
{
public:
	SET_GET(double,			Range,		mRange);
	SET_GET(std::string,	RangeUnit,	mRangeUnit);
	SET_GET(std::string,	RangeMode,	mRangeMode);

	SET_GET(std::string,	Input,		mInput);
	SET_GET(std::string,	Coupling,	mCoupling);
	SET_GET(int,			AntiAlias,	mAntiAlias);
	SET_GET(int,			FilterAW,	mFilterAW);
	SET_GET(int,			Bias,		mBias);

	SET_GET(int,			Flags,		mFlags);

	SET_GET(int,			XDCR,			mXDCR);
	SET_GET(double,			XDCRSens,		mXDCRSens);
	SET_GET(std::string,	XDCRSensUnit,	mXDCRSensUnit);
	SET_GET(std::string,	XDCRLabel,		mXDCRLabel);

	bool	Validate();
	void	CopyFrom(SignalAnalyzerChannel* pChannel);
			SignalAnalyzerChannel();	
private:
	double	mRange;
	std::string	mRangeUnit;
	std::string	mRangeMode;
	
	std::string	mInput;
	std::string	mCoupling;
	int		mAntiAlias;
	int		mFilterAW;
	int		mBias;
	int		mFlags; // 1 == ovld, 2 == half

	int		mXDCR;
	double	mXDCRSens;
	std::string	mXDCRSensUnit;
	std::string	mXDCRLabel;
};

#endif
