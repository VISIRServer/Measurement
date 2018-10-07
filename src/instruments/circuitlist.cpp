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

#include "circuitlist.h"
#include "circuitsolver3.h"
#include "listproducer.h"

#include "netlist2.h" // for named nodes

#include <vector>

CircuitList::CircuitList()
{
	mLogging = false;
	//mLogging = true;
}

CircuitList::~CircuitList()
{
}

bool CircuitList::Solve(const tCircuitList& circuitlist, const tCircuitList& maxlist)
{
	typedef std::vector<ListComponent> tCompList;

	tCompList circuit = circuitlist;

	CircuitSolver3 aSolver;
	if (mLogging) aSolver.EnableLogging();
	
	CircuitSolver3::tCandidates candidates(maxlist.size());
	copy(maxlist.begin(), maxlist.end(), candidates.begin()); // use back_inserter..

	bool foundsolution = false;
	typedef std::vector<ListComponent> tComps;
	tComps comps;

	if (aSolver.Solve(circuit, candidates))
	{
		comps = aSolver.GetSolution();
		mSolution = comps;
		foundsolution = true;
	}

	return foundsolution;
}

const CircuitList::tCircuitList& CircuitList::GetSolution() const
{
	return mSolution;
}

void CircuitList::EnableLogging()
{
	mLogging = true;
}