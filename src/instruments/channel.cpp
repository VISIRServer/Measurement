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

#include "channel.h"
#include "oscilloscope.h"
#include "instrumentblock.h"

#include <stringop.h>
#include <math.h>

Channel::Channel()
{
	mEnabled			= false;
	mVerticalCoupling	= DC;
	mVerticalRange		= 2.0;
	mVerticalOffset		= 0.0;
	mProbeAttenuation	= 1.0;
	mGain = 0.0;
}

Channel::~Channel()
{
}

void Channel::SetGraph(const char* buffer, size_t len, double gain, double offset)
{
	mGain = gain;
	mOffset = offset;
	mBinGraph.clear();
	for(size_t i=0;i<len;i++)
	{
		mBinGraph.push_back(buffer[i]);
	}
}

void InterpolateBuffer(char* in, size_t inlen, char* out, size_t outlen)
{
	for(size_t i=0;i<outlen;i++)
	{
		double pos = ((double)i / (double)outlen) * (double)inlen;
		double lookup = floor(pos);
		double t = pos - lookup;

		size_t pos1 = (size_t)lookup;
		size_t pos2 = (size_t)(lookup+1);
		if (pos2 > inlen) pos2 = inlen;
        
		out[i] = (char)((double)in[pos1] * (1.0-t) + (double)in[pos2] * t);
	}
}

void Channel::GetCharGraph(char* outbuffer, size_t len)
{
	char* tempbuffer = new char[mBinGraph.size()];
	copy(mBinGraph.begin(), mBinGraph.end(), tempbuffer);
	InterpolateBuffer(tempbuffer, mBinGraph.size(), outbuffer, len);
	delete [] tempbuffer;
}

Channel::tGraph Channel::GetScaledGraph(size_t len)
{
	tGraph outGraph;
	char* tempbuffer = new char[len];
	GetCharGraph(tempbuffer, len);

	for(size_t i=0; i<len; ++i)
	{
		double out = (double) tempbuffer[i];
		out *= mGain;
		out += mOffset;
		outGraph.push_back(out);
	}	

	delete [] tempbuffer;
	return outGraph;
}

char* Channel::GetRawGraph()
{
	return &(*mBinGraph.begin());
}

size_t Channel::GetNumSamples()
{
	return mBinGraph.size();
}

bool Channel::Validate()
{
	if (mVerticalRange < 0.00025 || mVerticalRange > (25.0*8.0))
		throw ValidationException("Vertical range must be between 0.00025 and 25.0");
	//if (mVerticalOffset < (-mVerticalRange * 4.0) || mVerticalOffset > (mVerticalRange * 4.0))
	//	throw ValidationException(0,"Vertical range must be between +/- half of vertical range");

	return true;
}

void Channel::CopyFrom(Channel* pChannel)
{
	mEnabled			= pChannel->mEnabled;
	mVerticalCoupling	= pChannel->mVerticalCoupling;
	mVerticalRange		= pChannel->mVerticalRange;
	mVerticalOffset		= pChannel->mVerticalOffset;
	mProbeAttenuation	= pChannel->mProbeAttenuation;
	mGain				= pChannel->mGain;
	mBinGraph			= pChannel->mBinGraph;
	// scaled graph is not copied
}

static const char* sVerticalCoupling[] = { "ac", "dc", NULL };
IMPL_SET_GET_STR(Channel, VerticalCouplingStr, sVerticalCoupling, mVerticalCoupling, ChannelCoupling)
