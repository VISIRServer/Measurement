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

#ifndef __NETLIST2_H__
#define __NETLIST2_H__

#include <vector>
#include <string>

#include "listcomponent.h"

namespace NamedNodes
{
	typedef const std::string NodeStringType;
	NodeStringType FGenA		= "VFGENA";
	NodeStringType FGenB		= "VFGENB";
	NodeStringType DCPlus25		= "VDC+25V";
	NodeStringType DCMinus25	= "VDC-25V";
	NodeStringType DCPlus6		= "VDC+6V";
	NodeStringType DC_COM		= "VDCCOM";

	NodeStringType Dmm			= "DMM";
	NodeStringType DmmIProbe	= "IPROBE";

	NodeStringType OscProbe		= "PROBE";
	NodeStringType OscProbe1	= "PROBE1";
	NodeStringType OscProbe2	= "PROBE2";
	NodeStringType OscProbe3	= "PROBE3";
	NodeStringType OscProbe4	= "PROBE4";

	// component nodes
	NodeStringType Shortcut		= "SHORTCUT";

	// these shouldn't be used anymore
	NodeStringType DC_A			= "VDCA";
	NodeStringType DC_B			= "VDCB";
	NodeStringType DC_5VA		= "5VA";
	NodeStringType DC_5VB		= "5VB";

};

class NetList2
{
public:
	typedef		ListComponent::tComponentList	tNodeList; //lista de objetos componentlist

	void		SetNodeList(const tNodeList& nodes);

	///			Returns the netlist as a string
	std::string	GetNetListAsString() const;
	
	const tNodeList&	GetNodeList() const;

	///			Parses the netlist
	//bool		Parse(const std::string& strnetlist);

	///			Tests if the netlist is a subset of another.
	bool		IsSubsetOf(const NetList2& netlist) const;

	///			Tests if the netlist is a subset of another, and returns the node it failed on.
	bool		IsSubsetOfWithFailed(const NetList2& superset, tNodeList& failed) const;

	// XXX: remove this, only one user in nodeinterpreter
	tNodeList	GetNodesOfType(const std::string& type) const;

	///			Get all switch nodes in the netlist
	// XXX: remove this, only one user in nodeinterpreter
	tNodeList	GetSwitches() const;

	static		void TransformToComparableList(tNodeList& nodelist);

	// ctor/dtor
	NetList2();
	NetList2(const tNodeList& nodes);
	~NetList2();
private:
	tNodeList	mNodeList;

};

#endif
