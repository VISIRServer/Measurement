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

#include "netlist2.h"
#include "listalgorithm.h"
#include "listproducer.h"

NetList2::NetList2()
{
}

NetList2::NetList2(const tNodeList& nodes)
{
	mNodeList = nodes;
}

NetList2::~NetList2()
{
}

void NetList2::SetNodeList(const tNodeList& nodes)
{
	mNodeList = nodes;
}

std::string NetList2::GetNetListAsString() const
{
	return ListProducer::Produce(mNodeList);
}

const NetList2::tNodeList& NetList2::GetNodeList() const
{
	return mNodeList;
}

bool NetList2::IsSubsetOf(const NetList2& superset) const
{
	tNodeList nodelist = mNodeList; // copy
	TransformToComparableList(nodelist);

	tNodeList superlist = superset.GetNodeList(); // copy
	TransformToComparableList(superlist);

	return ListAlgorithm::IsSubsetOf(nodelist, superlist);
}

bool NetList2::IsSubsetOfWithFailed(const NetList2& superset, tNodeList& failed) const
{
	tNodeList nodelist = mNodeList; // copy
	TransformToComparableList(nodelist);

	tNodeList superlist = superset.GetNodeList(); // copy
	TransformToComparableList(superlist);

	return ListAlgorithm::IsSubsetOfWithFailed(nodelist, superlist, failed);
}

NetList2::tNodeList	NetList2::GetNodesOfType(const std::string& type) const
{
	tNodeList out;
	ListAlgorithm::PushNodesOfType(type, mNodeList, out);
	return out;
}

NetList2::tNodeList NetList2::GetSwitches() const
{
	tNodeList out;
	ListAlgorithm::PushNodesOfType("XSWITCHOPEN", mNodeList, out);
	ListAlgorithm::PushNodesOfType("XSWITCHCLOSE", mNodeList, out);
	return out;
}

void NetList2::TransformToComparableList(tNodeList& nodelist)
{
	// Replace switches and iprobes with shortcuts
	ListAlgorithm::ReplaceType("IPROBE",		nodelist, "SHORTCUT");
	ListAlgorithm::ReplaceType("XSWITCHOPEN",	nodelist, "SHORTCUT");
	ListAlgorithm::ReplaceType("XSWITCHCLOSE",	nodelist, "SHORTCUT");

	// remove instrument nodes
	ListAlgorithm::RemoveOfType("DMM", nodelist);
	ListAlgorithm::RemoveOfType("PROBE1", nodelist);
	ListAlgorithm::RemoveOfType("PROBE2", nodelist);
	ListAlgorithm::RemoveOfType("PROBE3", nodelist);
	ListAlgorithm::RemoveOfType("PROBE4", nodelist);
}