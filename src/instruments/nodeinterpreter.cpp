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

#include "nodeinterpreter.h"
#include "digitalmultimeter.h"
#include "tripledc.h"

#include <stringop.h>
#include <syslog.h>

#include "listalgorithm.h"

NodeInterpreter::NodeInterpreter(int instrumentID) : Instrument(Instrument::TYPE_NodeInterpreter,instrumentID)
{
}

NodeInterpreter::~NodeInterpreter()
{
}

void NodeInterpreter::SetNetList(const NetList2& netlist)
{
	mNetList = netlist;
}

void NodeInterpreter::Accept(InstrumentVisitor& visitor)
{
	visitor.Visit(*this);
}

bool NodeInterpreter::Validate()
{
	NetList2::tNodeList iprobes = mNetList.GetNodesOfType(NamedNodes::DmmIProbe);
	NetList2::tNodeList dmms = mNetList.GetNodesOfType(NamedNodes::Dmm);

	for(NetList2::tNodeList::const_iterator probeit = iprobes.begin(); probeit != iprobes.end(); ++probeit)
	{
		for(NetList2::tNodeList::const_iterator dmmit = dmms.begin(); dmmit != dmms.end(); ++dmmit)
		{
			if (probeit->GetName() == dmmit->GetName())
			{
				Error("Can't measure voltage and current at the same time");
				return false;
			}
		}
	}

	//if (!iprobes.empty() && !dmms.empty()) Error("Can't measure voltage and current at the same time");
	return true;
}

void NodeInterpreter::CopyFrom(Instrument* pInstrument)
{
	Instrument::CopyFrom(pInstrument);
	NodeInterpreter* pNodeIntr = (NodeInterpreter*) pInstrument;

	mNetList		= pNodeIntr->mNetList;
}

bool NodeInterpreter::(Instrument::InstrumentType type)
{
	typedef ListComponent::tComponentList tComp;
	tComp instrComps;

	GetInstrumentNodes(type, instrComps);
	if (!instrComps.empty()) return true;
	return false;
}

void NodeInterpreter::GetInstrumentNodes(Instrument::InstrumentType type, ListComponent::tComponentList& out) const
{
	const ListComponent::tComponentList& components = mNetList.GetNodeList();
	switch(type)
	{
	case TYPE_Oscilloscope:
		{
			ListAlgorithm::PushNodesOfType(NamedNodes::OscProbe1,	components, out);
			ListAlgorithm::PushNodesOfType(NamedNodes::OscProbe2,	components, out);
			ListAlgorithm::PushNodesOfType(NamedNodes::OscProbe3,	components, out);
			ListAlgorithm::PushNodesOfType(NamedNodes::OscProbe4,	components, out);
		}
		
		break;
	case TYPE_FunctionGenerator:
		{
			ListAlgorithm::PushNodesOfType(NamedNodes::FGenA		, components, out);
		}
		break;
	case TYPE_DigitalMultimeter:
		{
			ListAlgorithm::PushNodesOfType(NamedNodes::Dmm,			components, out);
			ListAlgorithm::PushNodesOfType(NamedNodes::DmmIProbe,	components, out);
		}
		break;
	case TYPE_TripleDC:
		{
			ListAlgorithm::PushNodesOfType(NamedNodes::DCPlus25		, components, out);
			ListAlgorithm::PushNodesOfType(NamedNodes::DCMinus25	, components, out);
			ListAlgorithm::PushNodesOfType(NamedNodes::DCPlus6		, components, out);
		}
		break;
	default: break;
			
	}
}

ConnectionPoint NodeInterpreter::DigitalMultimeterConnection(DigitalMultimeter& dmm, size_t pointnr)
{
	if (dmm.MeasuresCurrent())
	{
		// XXX: We could just look up iprobe nodes with a specific name..
		NetList2::tNodeList nodes = mNetList.GetNodesOfType(NamedNodes::DmmIProbe);
		if (nodes.empty()) return ConnectionPoint();

		for(NetList2::tNodeList::const_iterator it = nodes.begin(); it != nodes.end(); ++it)
		{
			if (it->GetName() == ToString(dmm.GetID()))
			{
				const ListComponent::tConnections& cons = it->GetCConnections();
				if (pointnr >= cons.size()) return ConnectionPoint();

				if (nodes.front().GetSpecial() == "1") // the circuit solver thinks we should flip the iprobe
				{
					return ConnectionPoint(cons[1-pointnr]);
				}

				return ConnectionPoint(cons[pointnr]);
			}
		}

		return ConnectionPoint();

		/*
		ListComponent::tConnections cons = nodes.front().GetCConnections();
		if (pointnr >= cons.size()) return ConnectionPoint();

		if (nodes.front().GetSpecial() == "1") // the circuit solver thinks we should flip the iprobe
		{
			return ConnectionPoint(cons[1-pointnr]);
		}
		
		return ConnectionPoint(cons[pointnr]);
		*/
	}
	else
	{
		// XXX: We could just look up dmm nodes with a specific name..
		NetList2::tNodeList nodes = mNetList.GetNodesOfType(NamedNodes::Dmm);
		if (nodes.empty()) return ConnectionPoint();

		//if (dmm.GetID() > 1)
		{
			for(NetList2::tNodeList::const_iterator it = nodes.begin(); it != nodes.end(); ++it)
			{
				if (it->GetName() == ToString(dmm.GetID()))
				{
					const ListComponent::tConnections& cons = it->GetCConnections();
					if (pointnr >= cons.size()) return ConnectionPoint();
					return ConnectionPoint(cons[pointnr]);
				}
			}
		}
		/*else
		{
			const ListComponent::tConnections& cons = nodes.front().GetCConnections();
			if (pointnr >= cons.size()) return ConnectionPoint();

			return ConnectionPoint(cons[pointnr]);
		}*/

		/*const ListComponent::tConnections& cons = nodes.front().GetCConnections();
		if (pointnr >= cons.size()) return ConnectionPoint();

        return ConnectionPoint(cons[pointnr]);
		*/
	}

	// never reached..
	return ConnectionPoint();
}

ConnectionPoint NodeInterpreter::OscilloscopeChannelConnection(Oscilloscope& osc, size_t ch)
{
	std::string probename = NamedNodes::OscProbe;
	probename += ToString((int)ch+1);

	NetList2::tNodeList nodes = mNetList.GetNodesOfType(probename);
	if (nodes.empty()) return ConnectionPoint();
	
	return ConnectionPoint(nodes.front().GetCConnections().front());
}

ConnectionPoint NodeInterpreter::TripleDCConnection(TripleDC& tripledc, size_t ch)
{
	std::string nodeName = "";
	switch(ch)
	{
		case TRIPLEDC_25PLUS:	nodeName = NamedNodes::DCPlus25;	break;
		case TRIPLEDC_25MINUS:	nodeName = NamedNodes::DCMinus25;	break;
		case TRIPLEDC_6:		nodeName = NamedNodes::DCPlus6;		break;
		default: return ConnectionPoint();
	}

	NetList2::tNodeList nodes = mNetList.GetNodesOfType(nodeName);
	if (nodes.empty()) return ConnectionPoint();
	return ConnectionPoint(nodes.front().GetCConnections().front());
}

bool NodeInterpreter::ContainsSwitches() const
{
	return !mNetList.GetSwitches().empty();
}