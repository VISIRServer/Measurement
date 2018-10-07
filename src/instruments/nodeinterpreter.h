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
#ifndef __NODE_INTERPRETER_H__
#define __NODE_INTERPRETER_H__

#include "instrument.h"
#include "netlist2.h"
#include "connectionpoint.h"

/// NetList handler, used for designing the curcuit
/// todo: clean this up!
class NodeInterpreter : public Instrument
{
public:
	typedef			std::vector<std::string> tConnections;

	///				Set the current netlist.
	void			SetNetList(const NetList2& netlist);
	
	///				Get the current netlist
	inline const NetList2&	GetNetList() const { return mNetList; }

	///				Check if a instrument of the type is in the netlist.
	bool			(Instrument::InstrumentType type);

	ConnectionPoint	DigitalMultimeterConnection(DigitalMultimeter& dmm, size_t ch);
	ConnectionPoint OscilloscopeChannelConnection(Oscilloscope& osc, size_t ch);
	ConnectionPoint TripleDCConnection(TripleDC& tripledc, size_t ch);

	///				Symbolic circuit data from the client
	void			SetCircuitList(std::string in) { mCircuitList = in; }
	std::string		GetCircuitList() const { return mCircuitList; }

	bool			ContainsSwitches() const;

	virtual void	CopyFrom(Instrument* pInstrument);

	///				part of the visitor pattern
	virtual void	Accept(InstrumentVisitor& visitor);

	virtual bool	Validate();

	///				ctor
					NodeInterpreter(int instrumentID);
	///				dtor
	virtual			~NodeInterpreter();
private:
	void			GetInstrumentNodes(Instrument::InstrumentType type, ListComponent::tComponentList& out) const;

	NetList2		mNetList;
	std::string		mCircuitList;
};

#endif
