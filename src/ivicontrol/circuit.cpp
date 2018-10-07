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
 * Copyright (c) 2008 Johan Zackrisson
 * All Rights Reserved.
 */

#include "circuit.h"

#include <instruments/netlist2.h>
#include <instruments/nodeinterpreter.h>
#include <instruments/instrumentblock.h>
#include <instruments/digitalmultimeter.h>
#include <instruments/oscilloscope.h>

// list operations
#include <instruments/listalgorithm.h>
#include <instruments/listproducer.h> // temp for debug

#include <stringop.h>

using namespace IVIControl;
using namespace std;

void Circuit::PushCardComponent(tCardCompList& outPairs, const std::string& name)
{
	list<string> tokens;
	Tokenize(name, tokens, ":");
	for(list<string>::const_iterator it = tokens.begin(); it != tokens.end(); it++)
	{
		vector<string> cardComp;
		Tokenize(*it, cardComp, "_");
		if (cardComp.size() != 2) throw BasicException("Expected <card>_<component> in componentlist");
		outPairs.push_back( tCardComp(ToInt(cardComp[0]), ToInt(cardComp[1])) );
	}
}

bool Circuit::BuildCircuitSetup(const tComponentList& inCircuit, const tComponentList& componentList, tCardCompList& outPairs)
{
	tComponentList outComp;
	if (ListAlgorithm::CircuitSetupMatch(inCircuit, componentList, outComp))
	{
		for(tComponentList::const_iterator it = outComp.begin(); it != outComp.end(); it++)
		{
			PushCardComponent(outPairs, it->GetName());
		}
	}
	else
	{
		//syserr << "Matched checklist is not a subset of the component list. Contact the administrator" << endl;
		//syserr << "Dumping netlist:" << endl;
		//syserr << netlist.GetNetListAsString() << endl;
		throw BasicException("Matched checklist is not a subset of the component list. Contact the administrator");
	}

	return false;
}

bool Circuit::BuildCircuitSwitch(const tComponentList& inCircuit, const tComponentList& componentList, tCardCompList& outPairs)
{
	tComponentList switches;
	
	ListAlgorithm::PushNodesOfType("XSWITCHOPEN", inCircuit, switches);
	ListAlgorithm::PushNodesOfType("XSWITCHCLOSE", inCircuit, switches); // check that we append and not clear!

	if (switches.size() > 0)
	{
		// only allow one switch per circuit
		if (switches.size() > 1) throw BasicException("Only one switch per circuit");

		for(tComponentList::iterator it = switches.begin(); it != switches.end(); it++)
		{
			ListComponent matched;
			it->SetType("SHORTCUT");
			
			if (!ListAlgorithm::MatchNode(*it, componentList, matched))
			{
				throw BasicException("No switch found in component list. Contact administrator");
			}

			PushCardComponent(outPairs, matched.GetName());
		}
	}

	return true;
}

unsigned int Circuit::NodeToAddress(int node, bool hi)
{
	//                       0   A   B   C   D   E   F   G   H   I
	const int loNodes[] = { 17, 20, 16, 15, 14, 13, 12, 11, 10,  9 };
	const int hiNodes[] = { 18, 19,  1,  2,  3,  4,  5,  6,  7,  8 };

	if (node < 0 || node >= (sizeof(hiNodes) / sizeof(hiNodes[0])))
	{
		throw BasicException("measuring on non measurable node");
		return -1;
	}

	return (hi) ? hiNodes[node] : loNodes[node];
}

bool Circuit::BuildInstrumentSetup(InstrumentBlock* pBlock, tCardCompList& outPairs)
{
	int osccard = 16; // these shouldn't be hardcoded..
	int dmmcard = 17;

	unsigned int oscflags = 0;
	unsigned int dmmflags = 0;

	NodeInterpreter* pNodeIntr = pBlock->GetNodeInterpreter();

	// oscilloscope
	Oscilloscope* pOsc = pBlock->Get<Oscilloscope>(1);
	if (pOsc)
	{
		ConnectionPoint c1 = pNodeIntr->OscilloscopeChannelConnection(*pOsc, 0);
		ConnectionPoint c2 = pNodeIntr->OscilloscopeChannelConnection(*pOsc, 1);

		if (c1.IsConnected())
		{
			//LogLevel(sysout, 5) << "Osc1 connected to: " << c1.GetPointString() << endl;
			int relaynum = NodeToAddress(c1.ToPointNumber(), true);
			outPairs.push_back( tCardComp(osccard, relaynum) );
		}

		if (c2.IsConnected())
		{
			//LogLevel(sysout, 5) << "Osc2 connected to: " << c2.GetPointString() << endl;
			int relaynum = NodeToAddress(c2.ToPointNumber(), false);		
			outPairs.push_back( tCardComp(osccard, relaynum) );
		}
	}

	// dmm
	DigitalMultimeter* pDmm = pBlock->Get<DigitalMultimeter>(1);

	if (pDmm)
	{
		ConnectionPoint c1 = pNodeIntr->DigitalMultimeterConnection(*pDmm, 0);
		ConnectionPoint c2 = pNodeIntr->DigitalMultimeterConnection(*pDmm, 1);

		if (c1.IsConnected() && c2.IsConnected())
		{
			unsigned int dmm1 = NodeToAddress(c1.ToPointNumber(), false);	// dmm1, hi , warning... this translation is wierd
			unsigned int dmm2 = NodeToAddress(c2.ToPointNumber(), true);	// dmm2, lo
			
			outPairs.push_back( tCardComp(dmmcard, dmm1) );
			outPairs.push_back( tCardComp(dmmcard, dmm2) );
			if (pDmm->MeasuresCurrent()) outPairs.push_back( tCardComp(dmmcard, 15) );
		}
	}

	return true;
}


