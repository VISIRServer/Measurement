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
#ifndef __OSC_CHANNEL_H__
#define __OSC_CHANNEL_H__

#include "setget.h"

#include <vector>

// forward decl.
class Oscilloscope;

/// Oscilloscope channel configuration class.
/// Holds and manipulates information about the oscilloscope channel configuration.
/// Aggregate class of oscilloscope.
class Channel
{
public:
	typedef	std::vector< double > tGraph; // graphdata
	typedef std::vector< char >	tBinGraph;

	enum ChannelCoupling
	{
		AC			= 0,
		DC			= 1,	/// default
	};

	SET_GET(ChannelCoupling,	VerticalCoupling,	mVerticalCoupling);
	SET_GET(double,				VerticalRange,		mVerticalRange);
	SET_GET(double,				VerticalOffset,		mVerticalOffset);
	SET_GET(double,				ProbeAttenuation,	mProbeAttenuation);

	SET_GET_STR(VerticalCouplingStr);

	void			SetGraph(const char* buffer, size_t len, double gain, double offset = 0.0);
	tBinGraph		GetGraph();
	
	// Convert graph to char buffer. Linear interpolation used if buffersizes doesn't match
	void			GetCharGraph(char* outbuffer, size_t len);

	char*			GetRawGraph();
	size_t			GetNumSamples();

	double			GetGraphGain()				{ return mGain;			}
	double			GetGraphOffset()			{ return mOffset;		}
	
	///				Note: there is a difference between beeing connected and enabled.
	///				You can have a enabled channel without beeing connected.
	void			SetEnabled(bool enabled)	{ mEnabled = enabled; }
	bool			GetEnabled()				{ return mEnabled; }

	bool Validate();
	void CopyFrom(Channel* pChannel);

	Channel();
	virtual ~Channel();

private:
	bool			mEnabled;
	ChannelCoupling	mVerticalCoupling;
	double			mVerticalRange;
	double			mVerticalOffset;
	double			mProbeAttenuation;

	double			mGain;
	double			mOffset;

	tBinGraph		mBinGraph;

	// for safekeeping until we remove it
	tGraph			GetScaledGraph(size_t len);
};

#endif
