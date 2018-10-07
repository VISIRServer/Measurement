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
 * Copyright (c) 2008 André van Schoubroeck
 * All Rights Reserved.
 */

#include "tripledc.h"
#include "instrumentblock.h"

#include "listalgorithm.h"
#include "netlist2.h"

#include <stringop.h>

TripleDCChannel::TripleDCChannel()
{
	mVoltage = 0.0;
	mCurrent = 0.5;
	mMin = mMax = 0.0;
	mActualVoltage = 0;
	mActualCurrent = 0;
	mOutputEnabled = 1;
	mOutputLimited = 0;
}

bool TripleDCChannel::Validate()
{
	if (mVoltage < mMin) return false;
	if (mVoltage > mMax) return false;
	return true;
}

//////////////////////

TripleDC::TripleDC(int instrumentID) : Instrument(Instrument::TYPE_TripleDC, instrumentID)
{
	mChannels[TRIPLEDC_25PLUS].SetMinMax(0.0, 25.0);
	mChannels[TRIPLEDC_25MINUS].SetMinMax(-25.0, 0.0);
	mChannels[TRIPLEDC_6].SetMinMax(0.0, 6.0);
}

TripleDC::~TripleDC()
{
}

void TripleDC::Accept(InstrumentVisitor& visitor)
{
	visitor.Visit(*this);
}

TripleDCChannel* TripleDC::GetChannel(int channelnr)
{
	if (channelnr < 0 || channelnr >= 3) return NULL;
	return &mChannels[channelnr];
}

bool TripleDC::Validate()
{
	for(int i=0;i<3;i++)
	{
		if (!mChannels[i].Validate()) return false;
	}

	return true;
}

void TripleDC::CopyFrom(Instrument* pInstrument)
{
	Instrument::CopyFrom(pInstrument);
	TripleDC* pDC = (TripleDC*) pInstrument;

	// todo: verify that this does what it should..
	for(int i=0;i<3; i++)
	{
		mChannels[i] = pDC->mChannels[i];
	}
}

bool TripleDC::CheckMaxValues(const NetList2& maxlist)
{
	typedef ListComponent::tComponentList tComps;
	const tComps& comps = maxlist.GetNodeList();

	for(tComps::const_iterator it = comps.begin(); it != comps.end(); ++it)
	{
		std::string type = it->GetType();
		if (type == "VDC+25V" || type == "VDC-25V" || type == "VDC+6V")
		{
			if (it->GetSpecial() != "")
			{
				std::string vmaxstr = it->GetSpecialToken("VMAX:");

				// work around problem where max is written instead of vmax in maxlists
				std::string maxstr = it->GetSpecialToken("MAX:");
				if (vmaxstr.empty() && !maxstr.empty()) vmaxstr = maxstr;
				
				std::string imaxstr = it->GetSpecialToken("IMAX:");

				if (!vmaxstr.empty())
				{
					double vmax = ToDouble(vmaxstr);
					if (type == "VDC+25V" && Ch25plus().GetVoltage() > vmax)	return false;
					if (type == "VDC-25V" && Ch25minus().GetVoltage() < vmax)	return false;
					if (type == "VDC+6V"  && Ch6().GetVoltage() > vmax)			return false;
				}

				if (!imaxstr.empty())
				{
					double imax = ToDouble(imaxstr);
					if (type == "VDC+25V" && Ch25plus().GetCurrent() > imax)	return false;
					if (type == "VDC-25V" && Ch25minus().GetCurrent() > imax)	return false; // NTS: positive current
					if (type == "VDC+6V"  && Ch6().GetCurrent() > imax)			return false;
				}
			}
		}
	}

	return true;
}
