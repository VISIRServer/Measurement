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

#ifndef __CIRCUIT_SOLVER3_H__
#define __CIRCUIT_SOLVER3_H__

#include <vector>
#include <map>
#include <set>
#include <deque>

//#include "listcomponent.h"
#include "circuitsymbols2.h"

class LogModule;
class ListComponent;

class CircuitSolver3
{
private:
	typedef SymbolTable					tSymbols;
	typedef std::vector<ListComponent>	tCircuit;
	typedef std::deque<std::string>		tVisit;
	typedef std::vector<size_t>			tOrderedIndices;
	typedef std::vector<bool>			tUsedIndices;
	typedef std::vector<std::string>	tUsedInstrumentSymbols;
	typedef std::vector<size_t>			tShorts;
public:
	typedef std::deque<ListComponent>	tCandidates;
	typedef std::vector<size_t>			tUsage;

	typedef std::deque<size_t>			tCompIndices;
	typedef std::vector<ListComponent>	tVectorCircuit;

	bool Solve(const tCircuit& circuit, const tCandidates& candidates);

	//void Add(ListComponent& comp);
	void SetSymbol(const std::string& node, const std::string& symbol);
	
	tCircuit	GetSolution() const;

	void		EnableLogging();

	/// Check if a node is used in the solution
	bool		IsConnected(const std::string& node) const;

	CircuitSolver3();
	virtual ~CircuitSolver3();
private:
	//typedef std::map<std::string, tCompIndices > tTree;
	typedef std::map<std::string, size_t > tTree;

	typedef std::vector< tUsage >	tCandCache;
	tCandCache		mCandCache;

	tVectorCircuit	mIndexCircuit;

	tCircuit	mCircuit;
	tSymbols	mSymbols;

	tVectorCircuit	mCandidates;
	tCompIndices	mShortcuts;

	// These needs to be cleaned up..
	//tCircuit	mInstruments;
	tCircuit	mMeasInstrument;

	tCircuit	mSolution;
	tSymbols	mSolutionSymbols;

	bool	InsertIfValid(const ListComponent& circomp, size_t netcomp, int turn, const tUsage& usage, tSymbols& symbolcopy, tUsage& solution, bool shortcut);

	//		Tier one, search for used components and instruments in the circuit
	bool	TierOne(const tCandidates& cand, tUsedIndices& used, tOrderedIndices& ordered, tUsedInstrumentSymbols& instrSymbols) const;

	//		Tier two, replace wires with symbols
	//		also figure out which measurement instruments are connected and clear them from the candidates
	bool	TierTwo(const tCandidates& list, tUsedIndices& used, tSymbols& symbols, tUsedInstrumentSymbols& usedInstrumentNodes);

	//		Tier three, the actual solver
	bool	TierThree(const tCandidates& circuit, const tCandidates& candidates, tSymbols& symbols);
	bool	TierThreeSolveRecursive(size_t circuitidx, tUsage& usage, tSymbols& symbols);
	bool	TryCandidateList(const ListComponent& current, const tUsage& matched, size_t circuitidx, tUsage& usage, const tSymbols& symbols, bool withShortcuts);
	bool	TierThreeMatchAndInsert();

	bool	BuildCandidateCache();

	bool	SpecialCompare(const ListComponent& c1, const ListComponent& c2) const;

	bool	SearchShortcuts(tSymbols& symbols, const tUsage& usage, const std::string& endnode, const std::string& insertsymbol, tUsage& solution);
	bool	SearchShortcutVisit(const std::string& current, tVisit& tovisit, tShorts& edges, tTree& tree) const;
	bool	SearchShortcutBacktrace(const tTree& tree, const std::string& lastnode, const std::string& startnode, tCompIndices& out) const;

	void	InsertIProbe(ListComponent& component, int turn, const std::string& name);

	void	InstrumentFixup(tCandidates& cand);
	void	CircuitFixup(tCandidates& cand);
	void	AddInstrumentNodes(tCircuit& list);
	
	bool	MarkWiresAsRefs(const tCandidates& list, tSymbols& symbols) const;

	inline bool IsUsed(size_t idx, const tUsage& usage) const;
	inline void UpdateUsageMap(tUsage& usage, const tUsage& update, size_t state);

	/// Get first symbol from node
	std::string		GetFirstSymbol(const std::string& node);

	//void	DumpCandidateList(tCandidates& list);
	//friend void	DumpCandidateList(tCandidates& list);
};

#endif
