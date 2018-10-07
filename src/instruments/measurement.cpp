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

#include "measurement.h"
#include "oscilloscope.h"

Measurement::Measurement()
{
	// set default values
	mChannel		= Chan0;
	mSelection		= None;
	mMeasureValue	= 0.0;
}

Measurement::~Measurement()
{
}

bool Measurement::Validate()
{
	return true; 
}

double Measurement::GetMeasureResult()
{
	return mMeasureValue; 
}

void Measurement::SetMeasureResult(double result)
{
	mMeasureValue = result;
}

void Measurement::CopyFrom(Measurement* pMeasurement)
{
	mChannel		= pMeasurement->mChannel;
	mSelection		= pMeasurement->mSelection;
	mMeasureValue	= pMeasurement->mMeasureValue;
}

static const char* sChannel[] = { "channel 1", "channel 2", NULL };
IMPL_SET_GET_STR(Measurement, ChannelStr, sChannel, mChannel, MeasurementChannel)

static const struct sSelectionT
{
	const char* str;
	int			value;
} sSelection[] = { 
{ "acestimate",			1012 } ,
{ "area",				1003 } ,
{ "averagefrequency",	1016 } ,
{ "averageperiod",		1015 } ,
{ "cyclearea",			1004 } ,
{ "dcestimate",			1013 } ,
{ "falltime",			1 } ,
{ "fallingslewrate",	1011 } ,
{ "fftamplitude",		1009 } ,
{ "fftfrequency",		1008 } ,
{ "frequency",			2 } ,
{ "integral",			1005 } ,
{ "negativedutycycle",	13 } ,
{ "negativewidth",		11 } ,
{ "none",				4000 } ,
{ "overshoot",			18 } ,
{ "period",				3 } ,
{ "phasedelay",			1018 } ,
{ "positivedutycycle",	14 } ,
{ "positivewidth",		12 } ,
{ "preshoot",			19 } ,
{ "risetime",			0 } ,
{ "risingslewrate",		1010 } ,
{ "timedelay",			1014 } ,
{ "voltageamplitude",	15 } ,
{ "voltageaverage",		10 } ,
{ "voltagebase",		1006 } ,
{ "voltagebasetotop",	1017 } ,
{ "voltagecycleaverage",17 } ,
{ "voltagecyclerms",	16 } ,
{ "voltagehigh",		8 } ,
{ "voltagelow",			9 } ,
{ "voltagemax",			6 } ,
{ "voltagemin",			7 } ,
{ "voltagepeaktopeak",	5 } ,
{ "voltagerms",			4 } ,
{ "voltagetop",			1007 } ,
{ NULL, 0}
};


std::string Measurement::GetSelectionStr() const
{
	for(int i=0;sSelection[i].str; i++)
	{
		if (mSelection == sSelection[i].value)
		{
			return sSelection[i].str;
		}
	}

	throw ValidationException("INTERNAL ERROR: measurement selection is invalid");
}

void Measurement::SetSelectionStr(const std::string& str)
{
	for(int i=0;sSelection[i].str; i++)
	{
		if (str == sSelection[i].str)
		{
			mSelection = (MeasurementSelection)sSelection[i].value;
			return;
		}
	}

	throw ValidationException("Measurement::SetSelectionStr unable to convert");
}
