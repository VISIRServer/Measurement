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

#include "listproducer.h"

#include "instrumentblock.h"
#include "oscilloscope.h"
#include "digitalmultimeter.h"
#include "nodeinterpreter.h"

#include <stringop.h>

std::string ListProducer::Produce(const tComponentList& list, std::string linesep)
{
	std::string out;
	for(tComponentList::const_iterator it = list.begin(); it != list.end(); it++)
	{
		if (!out.empty()) out += linesep;
		out += ProduceComponent(*it);
	}
	return out;
}

std::string ListProducer::ProduceComponent(const ListComponent& comp)
{
	std::string out;
	out += comp.GetType();
	out += "_";
	out += comp.GetName();
	
	// connections
	typedef ListComponent::tConnections tCons;
	const tCons& cons = comp.GetCConnections();
	for(tCons::const_iterator conit = cons.begin(); conit != cons.end(); conit++)
	{
		out += " ";
		out += *conit;
	}

	if (!comp.GetValue().empty())
	{
		out += " ";
		out += comp.GetValue();
	}

	return out;
}

std::string ListProducer::ProduceInstruments(InstrumentBlock* block, std::string linesep)
{
	std::string out;

	// todo: handle more than one oscilloscope..
	Oscilloscope* pOsc = block->Get<Oscilloscope>(1);
	if (pOsc)
	{
		for(int channel=0; channel<OSC_CHANNELS; channel++)
		{
			Channel chan = pOsc->GetChannel(channel);
			if (chan.GetEnabled())
			{
				if (!out.empty()) out += linesep;

				std::string probe = "PROBE";
				std::string number = ToString(channel+1);
				std::string prefix = "P";

				out += probe + number + "_" + prefix + number + " " + 
					block->GetNodeInterpreter()->OscilloscopeChannelConnection(*pOsc, channel).GetPointString();

			}
		}
	}

	DigitalMultimeter* pDMM = block->Get<DigitalMultimeter>(1);

	if (pDMM)
	{
		if (!out.empty()) out += linesep;

		if (pDMM->MeasuresCurrent())
		{
			out += "IPROBE_1 ";
		}
		else
		{
			out += "DMM_DMM1 ";
		}

		std::string tempstring;
		
		tempstring += block->GetNodeInterpreter()->DigitalMultimeterConnection(*pDMM, 0).GetPointString() + " ";
		tempstring += block->GetNodeInterpreter()->DigitalMultimeterConnection(*pDMM, 1).GetPointString();
		out += tempstring;
	}

	return out;
}
