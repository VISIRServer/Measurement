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

#include "trigger.h"
#include "oscilloscope.h"

Trigger::Trigger()
{
	mSource		= Immediate;
	mType		= Edge;
	mSlope		= Positive;
	mLevel		= 0.0;
	mHoldOff	= 0.0;
	mDelay		= 0.0;
	mCoupling	= DC;
	mMode		= Auto;
}

Trigger::~Trigger()
{
}

bool Trigger::Validate()
{
	return true;
}

double Trigger::GetTimeout()
{
	return 2.0;
}

void Trigger::CopyFrom(Trigger* pTrigger)
{
	mSource		= pTrigger->mSource;
	mType		= pTrigger->mType;
	mSlope		= pTrigger->mSlope;
	mLevel		= pTrigger->mLevel;
	mHoldOff	= pTrigger->mHoldOff;
	mDelay		= pTrigger->mDelay;
	mCoupling	= pTrigger->mCoupling;
	mMode		= pTrigger->mMode;
	mTriggerReceived = pTrigger->mTriggerReceived;
}

static const char* sSource[] = { "channel 1", "channel 2", "immediate", "external", NULL };
IMPL_SET_GET_STR(Trigger, SourceStr, sSource, mSource, TriggerSource)

static const char* sSlope[] = { "positive", "negative", NULL };
IMPL_SET_GET_STR(Trigger, SlopeStr, sSlope, mSlope, TriggerSlope)

static const char* sCoupling[] = { "ac", "dc", NULL };
IMPL_SET_GET_STR(Trigger, CouplingStr, sCoupling, mCoupling, TriggerCoupling)

static const char* sMode[] = { "normal", "auto", "autolevel", NULL };
IMPL_SET_GET_STR(Trigger, ModeStr, sMode, mMode, TriggerMode)


