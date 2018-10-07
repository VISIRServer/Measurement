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

#pragma once
#ifndef __TRIGGER_H__
#define __TRIGGER_H__

#include <setget.h>
#include <string>

// forward decl.
class Oscilloscope;

/// Oscilloscope trigger configuration class.
/// Holds and manipulates information about the oscilloscope trigger configuration.
/// Aggregate class of oscilloscope.

class Trigger
{
public:
	/// \todo reorder this as soon as protocol version 2 is out of the way
	enum TriggerSource
	{
		Chan0		= 0,
		Chan1		= 1,
		Immediate	= 2,	/// default
		External	= 3,
		Chan2		= 4,
		Chan3		= 5, 
	};

	/// \todo remove
	enum TriggerType
	{
		Edge		= 0,	/// default
		Hystereses	= 1,
		Digital		= 2,
		Window		= 3,
	};

	enum TriggerSlope
	{
		Positive	= 0,	/// default
		Negative	= 1,
	};

	enum TriggerCoupling
	{
		AC			= 0,
		DC			= 1,	/// default
	};

	enum TriggerMode
	{
		Normal		= 0,
		Auto		= 1,	/// default
		AutoLevel	= 2,
	};

	SET_GET(TriggerSource,		Source,		mSource);
	SET_GET(TriggerType,		Type,		mType);
	SET_GET(TriggerSlope,		Slope,		mSlope);
	SET_GET(double,				Level,		mLevel);

	//SET_GET(double,				HoldOff,	mHoldOff);
	SET_GET(double,				Delay,		mDelay);
	SET_GET(TriggerCoupling,	Coupling,	mCoupling);
	SET_GET(TriggerMode,		Mode,		mMode);

	SET_GET(bool,				TriggerReceived, mTriggerReceived);

	SET_GET_STR(SourceStr);
	SET_GET_STR(SlopeStr);
	SET_GET_STR(CouplingStr);
	SET_GET_STR(ModeStr);

	void	SetTimeout(double timeout) {} /// \todo implement me!!
	double	GetTimeout();

	bool Validate();
	void CopyFrom(Trigger* pTrigger);

	Trigger();
	virtual ~Trigger();

private:
	TriggerSource	mSource;
	TriggerType		mType;
	TriggerSlope	mSlope;
	double			mLevel;
	double			mHoldOff;
	double			mDelay;
	TriggerCoupling	mCoupling;
	TriggerMode		mMode;
	bool			mTriggerReceived;
};

#endif
