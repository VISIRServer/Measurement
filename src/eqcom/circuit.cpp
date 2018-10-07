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
 * Copyright (c) 2008-2009 Johan Zackrisson
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

#include "symbols.h"
#include "eqlog.h"

#include <sstream>

using namespace EqSrv;
using namespace std;

bool Circuit::BuildCircuitSetup(std::ostream& out, InstrumentBlock* pBlock, NetList2& lookup)
{
	NetList2 netlist = pBlock->GetNodeInterpreter()->GetNetList();
	NetList2::tNodeList nodes = netlist.GetNodeList(); // copy

	// clean up netlist for matching
	// we have switches to be removed.. instruments to be cleared

	ListAlgorithm::RemoveOfType("DMM", nodes);
	ListAlgorithm::RemoveOfType("IPROBE", nodes);
	ListAlgorithm::RemoveOfType("PROBE1", nodes);
	ListAlgorithm::RemoveOfType("PROBE2", nodes);

	ListAlgorithm::RemoveOfType("XSWITCHCLOSE", nodes);
	ListAlgorithm::ReplaceType( "XSWITCHOPEN",  nodes, "SHORTCUT");

	//LogLevel(sysout, 5) << "Connected components:" << endl << ListProducer::Produce(nodes) << endl;

	ListAlgorithm::tIndexPairs matchedNodes; // <compidx, maxidx>

	NetList2::tNodeList componentList = lookup.GetNodeList();

	// check if we have all nodes in hw
	if (ListAlgorithm::MatchIndex(nodes, componentList, matchedNodes))
	{
		if (matchedNodes.empty()) return true;

		out << InstrumentHeadType(V3_CircuitBuilder) << "3 ";

		bool first = true;
		std::stringstream logMessage;
		logMessage << "component numbers:";
		for(ListAlgorithm::tIndexPairs::iterator it = matchedNodes.begin(); it != matchedNodes.end(); ++it)
		{
			if (!first)
			{
				out << "?";
			}
			else first = false;
			out << componentList[it->first].GetName();
			logMessage << " " << componentList[it->first].GetName();
		}

		eqlog.Log(5) << logMessage.str() << endl;
		out << "\n";

		// output any potentiometer values
		for(ListAlgorithm::tIndexPairs::iterator it = matchedNodes.begin(); it != matchedNodes.end(); ++it)
		{
			if (componentList[it->first].GetType() == "POT")
			{
				if (!nodes[it->second].GetSpecial().empty()) {
					out << InstrumentHeadType(V3_CircuitBuilder) << "7 "
						<< componentList[it->first].GetName() << " " << nodes[it->second].GetSpecial() << "\n";
					eqlog.Out(5) << "Potentiometer " << componentList[it->first].GetName()
						<< " set to " << nodes[it->second].GetSpecial() << endl;
				}
				else syserr << "Potentiometer is missing its setting" << endl;
			}
		}
	}
	else
	{
		eqlog.Error()
			<< "Matched checklist is not a subset of the component list. Contact the administrator" << endl
			<< "Dumping netlist:" << endl
			<< netlist.GetNetListAsString() << endl;
		throw BasicException("Matched checklist is not a subset of the component list. Contact the administrator");
	}

	return true;
}

bool Circuit::BuildCircuitSwitch(std::ostream& out, InstrumentBlock* pBlock, NetList2& lookup)
{
	const NetList2& netlist = pBlock->GetNodeInterpreter()->GetNetList();

	NetList2::tNodeList switches;
	
	ListAlgorithm::PushNodesOfType("XSWITCHOPEN", netlist.GetNodeList(), switches);
	ListAlgorithm::PushNodesOfType("XSWITCHCLOSE", netlist.GetNodeList(), switches); // check that we append and not clear!

	if (switches.size() > 0)
	{
		// temporary only allow one switch per circuit
		if (switches.size() > 1) throw BasicException("Only one switch per circuit");

		out << InstrumentHeadType(V3_CircuitBuilder) << "5 ";

		bool first = true;
		for(NetList2::tNodeList::iterator it = switches.begin(); it != switches.end(); it++)
		{
			if (!first)
			{
				out << "?";
			}
			else first = false;

			ListComponent matched;

			it->SetType("SHORTCUT");
			
			if (!ListAlgorithm::MatchNode(*it, lookup.GetNodeList(), matched))
			{
				throw BasicException("No switch found in component list. Contact administrator");
			}

			out << matched.GetName();
			cerr << "switchnr: " << matched.GetName() << endl;
		}

		out << "\n";
	}

	return true;
}

unsigned int Circuit::NodeToAddress(int node, bool hi)
{
	unsigned char magic_shift = 0;

	if (hi)
	{
		switch(node)
		{
		case 0: return 3;
		case 1: return 2;
		case 2: return 4;
		case 3: return 5;
		case 4: return 6;
		case 5: return 7;
		case 6: return 10;
		case 7: return 9;
		case 8: return 8;
		default:
			throw BasicException("measuring on non measurable node");
			return -1;
		}
	}
	else
	{
		switch(node)
		{
		case 0: return 17 + magic_shift;
		case 1: return 16 + magic_shift;
		case 2: return 18 + magic_shift;
		case 3: return 19 + magic_shift;
		case 4: return 20 + magic_shift;
		case 5: return 14 + magic_shift;
		case 6: return 13 + magic_shift;
		case 7: return 12 + magic_shift;
		case 8: return 11 + magic_shift;
		default:
			throw BasicException("measuring on non measurable node");
			return -1;
		}
	}
}

/*
bool Circuit::BuildInstrumentSetup(std::ostream& out, InstrumentBlock* pBlock)
{
	int osccard = 16;
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
			int x = NodeToAddress(c1.ToPointNumber(), true);
			oscflags |= 1 << (x-1);
			eqlog.Log(5) << "Osc1 connected to: " << c1.GetPointString() << endl;

			out << InstrumentHeadType(V3_CircuitBuilder) << "1 " << osccard+1 << " " << x << "\n";
		}

		if (c2.IsConnected())
		{
			int x = NodeToAddress(c2.ToPointNumber(), false);
			oscflags |= 1 << (x-1);
			eqlog.Log(5) << "Osc2 connected to: " << c2.GetPointString() << endl;

			out << InstrumentHeadType(V3_CircuitBuilder) << "1 " << osccard+1 << " " << x << "\n";
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

			//unsigned int chan = pDmm->MeasuresCurrent() ? 16 : 1;

			unsigned int currentmask = 0;
			if (pDmm->MeasuresCurrent()) currentmask = 1 << 20; // this adressing is wierd..

			dmmflags = 1 << (dmm1 - 1) | 1 << (dmm2 - 1) | currentmask;

			eqlog.Log(5) << "DMM Connected to: " << c1.GetPointString() << " " << c2.GetPointString() << endl;

			out << InstrumentHeadType(V3_CircuitBuilder) << "1 " << dmmcard+1 << " " << dmm1 << "\n";
			out << InstrumentHeadType(V3_CircuitBuilder) << "1 " << dmmcard+1 << " " << dmm2 << "\n";
			if (pDmm->MeasuresCurrent()) out << InstrumentHeadType(V3_CircuitBuilder) << "1 " << dmmcard+1 << " " << 15 << "\n";
		}
	}

	return true;
}*/


bool Circuit::BuildInstrumentSetup2(std::ostream& out, InstrumentBlock* pBlock)
{
	NodeInterpreter* pNodeIntr = pBlock->GetNodeInterpreter();

	std::stringstream tempout;
	//int i;

	//i = 1;
	//while(BuildOscilloscope(tempout, pBlock, i)) i++;
	//i = 1;
	BuildOscilloscope(tempout, pBlock);
	if (!tempout.str().empty()) tempout << "?";
	BuildDigitalMultimeters(tempout, pBlock);

	// oscilloscope
	/*Oscilloscope* pOsc = pBlock->Get<Oscilloscope>(1);
	if (pOsc)
	{
		ConnectionPoint c1 = pNodeIntr->OscilloscopeChannelConnection(*pOsc, 0);
		ConnectionPoint c2 = pNodeIntr->OscilloscopeChannelConnection(*pOsc, 1);

		if (c1.IsConnected())
		{
			if (!tempout.str().empty()) tempout << "?";
			tempout << "OSC 1 1 " << c1.GetPointString();
			eqlog.Log(5) << "Osc1 connected to: " << c1.GetPointString() << endl;
		}

		if (c2.IsConnected())
		{
			if (!tempout.str().empty()) tempout << "?";
			tempout << "OSC 1 2 " << c2.GetPointString();
			eqlog.Log(5) << "Osc2 connected to: " << c2.GetPointString() << endl;
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
			if (!tempout.str().empty()) tempout << "?";
			tempout << "DMM 1 "; // dmm number
			tempout << (pDmm->MeasuresCurrent() ? "I" : "V");
			tempout << " " << c1.GetPointString() << " " << c2.GetPointString();

			eqlog.Log(5) << "DMM Connected to: " << c1.GetPointString() << " " << c2.GetPointString() << endl;
		}
	}*/

	if (!tempout.str().empty())
	{
		out << InstrumentHeadType(V3_CircuitBuilder) << "6 " << tempout.str() << "\n";
	}

	return true;
}

bool Circuit::BuildOscilloscope(std::ostream& out, InstrumentBlock* pBlock)
{
	NodeInterpreter* pNodeIntr = pBlock->GetNodeInterpreter();
	std::stringstream tempout;

	typedef	std::vector<Oscilloscope*> tOscs;
	tOscs oscs = pBlock->GetAll<Oscilloscope>();
	for(tOscs::iterator it = oscs.begin(); it != oscs.end(); it++)
	{

		Oscilloscope* pOsc = *it;
		ConnectionPoint c1 = pNodeIntr->OscilloscopeChannelConnection(*pOsc, 0);
		ConnectionPoint c2 = pNodeIntr->OscilloscopeChannelConnection(*pOsc, 1);

		if (c1.IsConnected())
		{
			if (!tempout.str().empty()) tempout << "?";
			tempout << "OSC " << pOsc->GetID() << " 1 " << c1.GetPointString();
			eqlog.Log(5) << "Osc1 connected to: " << c1.GetPointString() << endl;
		}

		if (c2.IsConnected())
		{
			if (!tempout.str().empty()) tempout << "?";
			tempout << "OSC " << pOsc->GetID() << " 2 " << c2.GetPointString();
			eqlog.Log(5) << "Osc2 connected to: " << c2.GetPointString() << endl;
		}
	}

	if (!tempout.str().empty()) out << tempout.str();

	return true;
}

bool Circuit::BuildDigitalMultimeters(std::ostream& out, InstrumentBlock* pBlock)
{
	NodeInterpreter* pNodeIntr = pBlock->GetNodeInterpreter();
	std::stringstream tempout;

	typedef	std::vector<DigitalMultimeter*> tMultimeters;
	tMultimeters multimeters = pBlock->GetAll<DigitalMultimeter>();
	for(tMultimeters::iterator it = multimeters.begin(); it != multimeters.end(); it++)
	{
		DigitalMultimeter* pDmm = *it; //pBlock->Get<DigitalMultimeter>(num);

		if (pDmm)
		{
			ConnectionPoint c1 = pNodeIntr->DigitalMultimeterConnection(*pDmm, 0);
			ConnectionPoint c2 = pNodeIntr->DigitalMultimeterConnection(*pDmm, 1);

			if (c1.IsConnected() && c2.IsConnected())
			{
				if (!tempout.str().empty()) tempout << "?";
				tempout << "DMM " << pDmm->GetID() << " "; // dmm number
				tempout << (pDmm->MeasuresCurrent() ? "I" : "V");
				tempout << " " << c1.GetPointString() << " " << c2.GetPointString();

				eqlog.Log(5) << "DMM " << pDmm->GetID() << " Connected to: " << c1.GetPointString() << " " << c2.GetPointString() << " " << (pDmm->MeasuresCurrent() ? "I" : "V") << endl;
			}
		}
	}

	if (!tempout.str().empty())
	{
		out << tempout.str();
		
	}
	return true;
}
