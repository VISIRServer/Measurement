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

#ifndef __CIRCUIT_LIST_H__
#define __CIRCUIT_LIST_H__

#include "listcomponent.h"

#include <vector>

/*
	Special symbols:
	DMM_VHI - measurement nodes
	DMM_VLO
	DMM_AHI
	DMM_ALO

	FGEN_A	- is always symbol "A"
	5V_A	-					"A"
	5V_B	-					"D"

	OSC_1	- measurement nodes
	OSC_2
	OSC_3
	OSC_4

	GND		- is always symbol "0"
*/

// forward decl.
class CircuitSolver3;

class CircuitList
{
public:
	typedef std::vector<ListComponent>	tCircuitList;

	bool Solve(const tCircuitList& circuitlist, const tCircuitList& maxlist);
	const ListComponent::tComponentList& GetSolution() const;

	void	EnableLogging();

	CircuitList();
	virtual ~CircuitList();
private:
	//void	AddInstrumentNodes(CircuitSolver3& solver, tCircuitList& list);

	std::string	mCircuitList;
	std::string	mMaxList;
	ListComponent::tComponentList mSolution;
	bool	mLogging;
};

#endif
