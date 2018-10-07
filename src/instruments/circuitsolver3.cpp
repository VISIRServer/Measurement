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

#include "circuitsolver3.h"
#include "listalgorithm.h"
#include "netlist2.h" // for named nodes
#include "circuitsolverinternal.h"

#include <logmodule.h>
#include <sstream>

using namespace std;

#define ARR_SIZE(a) ( sizeof(a) / sizeof(a[0]) )

const char* instrumentNodes[] = { "VFGENA", "DMM", "PROBE", "PROBE1", "PROBE2", "PROBE3", "PROBE4", "VDC+25V", "VDC-25V", "VDC+6V", "VDCCOM" };
const size_t numInstrumentNodes = ARR_SIZE(instrumentNodes);

//const char* generatorNodes[] = { "VFGENA", "VDC+25V", "VDC-25V", "VDC+6V", "VDCCOM" };
//const size_t numGeneratorNodes = ARR_SIZE(generatorNodes);

static bool isInstrumentNode(const std::string& type)
{
	for(int i=0; i< numInstrumentNodes; i++)
	{
		//if (type == instrumentNodes[i]) return true;
		if (strcmp(type.c_str(), instrumentNodes[i]) == 0) return true;
	}
	return false;
}

/*static bool isGeneratorNode(const std::string& type)
{
	for(int i=0; i< numGeneratorNodes; i++)
	{
		if (type == generatorNodes[i]) return true;
	}
	return false;
}*/

/*
	TODO: it may be faster to use indices instead of data comparation when comparing connection iterators
	or maybe we can compare iterators instead?
*/

LogModule circuitLog("circuitlog", 5);
fstream		flog;
int			sLogging = false;

void InitLogging()
{
	static bool sInitialized = false;
	if (!sInitialized)
	{
		sInitialized = true;
		circuitLog.AddScreenStream(&cout);
		circuitLog.AddErrorStream(&cerr);
		flog.open("circuit.log", ios_base::binary | ios_base::out | ios_base::app);
		if (flog.is_open()) circuitLog.AddFileStream(&flog);
	}
}

CircuitSolver3::CircuitSolver3()
{
}

CircuitSolver3::~CircuitSolver3()
{
}

bool CircuitSolver3::Solve(const tCircuit& incircuit, const tCandidates& candidates)
{
	mCircuit = incircuit;

	mSolution.clear();
	
	SymbolTable::ResetLookup();

#ifdef DEBUG_OUT
	OUTSTREAM << "input candidate dump" << endl;
	DumpCandidateList(candidates);
#endif

	tCandidates circuit(mCircuit.size());
	copy(mCircuit.begin(), mCircuit.end(), circuit.begin());
	CircuitFixup(circuit);

#ifdef DEBUG_OUT
	OUTSTREAM << "input circuit dump" << endl;
	DumpCandidateList(circuit);
#endif

	InstrumentFixup(circuit);

	tCandidates cleancirlist;

#ifdef DEBUG_OUT
	OUTSTREAM << "***** Tier one begin *****" << endl;
#endif
	
	tUsedInstrumentSymbols instrSymbols;
	// this should be more than enough space, but it depends on how many instruments are sent (and if we use the instrumentfixup for backwards compatibility)
	instrSymbols.reserve(16);
	
	tUsedIndices used(circuit.size());
	tOrderedIndices ordered;
	ordered.reserve(circuit.size());
	
	if (!TierOne(circuit, used, ordered, instrSymbols))
	{
		OUT(cerr << "Tier one failed" << endl);
		return false;
	}

#ifdef DEBUG_OUT
	OUTSTREAM << "***** Tier one end *****" << endl;
	OUTSTREAM << "Tier one dump" << endl;
	DumpCandidateList(cleancirlist);
	OUTSTREAM << "Instrument symbols: " << instrSymbols.size() << endl;

	OUTSTREAM << "***** Tier two begin *****" << endl;
#endif
	
	//cout << DumpCandidateList(cleancirlist) << endl;

	// this needs the original circuit for the wires..
	// and the cleancirlist for adding components and removing wires when done
	if (!TierTwo(circuit, used, mSymbols, instrSymbols))
	{
		OUT(cerr << "Tier two failed" << endl);
		return false;
	}
	
	for(size_t i=0, size=ordered.size(); i<size; i++)
	{
		if (used[ordered[i]]) cleancirlist.push_back(circuit[ordered[i]]);
	}

#ifdef DEBUG_OUT
	OUTSTREAM << "***** Tier two end *****" << endl;

	OUTSTREAM << "Tier two dump" << endl;
	DumpCandidateList(cleancirlist);
	mSymbols.Dump();

	OUTSTREAM << "***** Tier three begin *****" << endl;
	OUTSTREAM << "Tier three start dump" << endl;
	DumpCandidateList(cleancirlist);
#endif

	if (!TierThree(cleancirlist, candidates, mSymbols))
	{
		OUT(OUTSTREAM << "Tier three failed" << endl);
		return false;
	}

#ifdef DEBUG_OUT
	OUTSTREAM << "***** Tier three end *****" << endl;
	mSolutionSymbols.Dump();
#endif

	AddInstrumentNodes(mSolution);

	return true;
}

/*
	Insert instrument nodes for old "magic" node names
*/
void CircuitSolver3::InstrumentFixup(tCandidates& cand)
{
	/*
	FGEN_A => FGEN_1 FGEN_A
	DMM_VHI, DMM_VLO => DMM_1 DMM_VHI DMM_VLO
	OSC_1 => PROBE_1_1 OSC_1
	OSC_2 => PROBE_1_2 OSC_2
	OSC_3 => PROBE_1_3 OSC_3
	OSC_4 => PROBE_1_4 OSC_4
	DC_+25V => VDC+25V_1 DC_+25V
	DC_-25V => VDC-25V_1 DC_-25V
	DC_+6V => VDC+6V_1 DC_+6V
	DC_COM => VDCCOM_1 DC_COM
	*/
	cand.push_back(ListComponent("VFGENA",  "1", "FGEN_A"));
	cand.push_back(ListComponent("DMM",	  "1", "DMM_VHI", "DMM_VLO"));
	cand.push_back(ListComponent("PROBE1", "1", "OSC_1"));
	cand.push_back(ListComponent("PROBE2", "1", "OSC_2"));
	cand.push_back(ListComponent("PROBE3", "1", "OSC_3"));
	cand.push_back(ListComponent("PROBE4", "1", "OSC_4"));
	cand.push_back(ListComponent("VDC+25V", "1", "DC_+25V"));
	cand.push_back(ListComponent("VDC-25V", "1", "DC_-25V"));
	cand.push_back(ListComponent("VDC+6V", "1", "DC_+6V"));
	cand.push_back(ListComponent("VDCCOM", "1", "DC_COM"));

	//
	SetSymbol("FGEN_A", "A");
	SetSymbol("0", "0");
}

/*
	Add (old) hidden instrument components to the circuit
*/

void CircuitSolver3::CircuitFixup(tCandidates& circuit)
{
	circuit.push_back(ListComponent(NamedNodes::DmmIProbe, "1", "DMM_AHI", "DMM_ALO"));
}

bool CircuitSolver3::TierOne(const tCandidates& cand, tUsedIndices& used, tOrderedIndices& ordered, tUsedInstrumentSymbols& instrSymbols) const
{
	typedef list<string> tToVisit;
	tToVisit toVisit;
	
	// Add all instrument nodes as nodes to visit
	// and keep track of the instruments
	// all connection names are symbolic
	for(tCandidates::const_iterator it = cand.begin(); it != cand.end(); it++)
	{
		if (isInstrumentNode(it->GetType()))
		{
			typedef ListComponent::tConnections tCons;
			const tCons& cons = it->GetCConnections();
			
			for(tCons::const_iterator conit = cons.begin(); conit != cons.end(); ++conit)
			{
				toVisit.push_back(*conit);
				instrSymbols.push_back(*conit);
			}
		}
	}

	const char* nodesToVisit[] = { "0" };
	size_t numNodes = sizeof(nodesToVisit) / sizeof(nodesToVisit[0]);
	toVisit.insert(toVisit.end(), nodesToVisit, nodesToVisit + numNodes);
	
	while(!toVisit.empty())
	{
		const string current = toVisit.front();
		toVisit.pop_front();
		
		//cout << current << endl;

		OUT( OUTSTREAM << "VISIT: " << current << endl);
		
		for(size_t candidx=0, size=cand.size(); candidx<size; candidx++)
		{
			bool found = false;
			if (used[candidx]) continue;
			
			typedef ListComponent::tConnections tCons;
			const tCons& cons = cand[candidx].GetCConnections();
						
			// check each component for any connection matching current symbol
			for(size_t conidx = 0, consize=cons.size(); conidx < consize; conidx++)
			{
				if (cons[conidx] == current) {
					if (consize == 1) {
						found = true;
					}
					// add all other connection points to the list of symbols to visit
					for(size_t i=0;i<consize; i++)
					{
						if (i != conidx) {
							// odd/old behaviour, count components directly connected to itself as not connected
							if (cons[i] != cons[conidx]) {
								toVisit.push_back(cons[i]);
								found = true;
							}
						}
					}
				}
			}
			
			if (found) {
				used[candidx] = true;
				ordered.push_back(candidx);
			}
		}
	}
		
	return true;
}

bool CircuitSolver3::MarkWiresAsRefs(const tCandidates& candidates, tSymbols& symbols) const
{
	// insert wires from original list. That makes probes work
	for(tCandidates::const_iterator it = candidates.begin(), end = candidates.end(); it != end; ++it)
	{
		if (it->GetType() == "W")
		{
			// add ref and check that no symbol contains more than one nodename
			if (!symbols.Ref(it->GetCConnections()[0], it->GetCConnections()[1]))
			{
				return false;
			}
		}
	}
	return true;
}

bool CircuitSolver3::TierTwo(const tCandidates& list, tUsedIndices& usedcmpnts, tSymbols& symbols, tUsedInstrumentSymbols& usedInstrumentNodes)
{
	if (!MarkWiresAsRefs(list, symbols)) return false;
	
	SymbolTable::tMarked markData;

	for(size_t i=0, size = list.size(); i < size; i++)
	{
		if (!usedcmpnts[i]) continue;
		const ListComponent& cmp = list[i];
		
		if (cmp.GetType() != "W" && !isInstrumentNode(cmp.GetType()))
		{
			const ListComponent::tConnections& cons = cmp.GetCConnections();
			
			for(size_t i = 0; i < cons.size(); ++i)
			{
				symbols.Mark(markData, cons[i]);
			}
		}
	}
	
	usedInstrumentNodes.push_back("0"); // XXX: verify that this actually does the right thing..
	
	for(size_t i=0, size = usedInstrumentNodes.size(); i < size; i++)
	{
		for(size_t j=i+1; j<size; j++)
		{
			if (symbols.RefersSameSymbol(usedInstrumentNodes[i], usedInstrumentNodes[j]))
			{
				OUT(OUTSTREAM << "INSTRUMENTS DIRECTLY LINKED: " << usedInstrumentNodes[i] << " " << usedInstrumentNodes[j] << endl);
				symbols.Mark(markData, usedInstrumentNodes[i]);
			}
		}
	}

	OUT(OUTSTREAM << "Tier two: Output symbol table before cleanup" << endl);
	OUT(symbols.Dump());

	for(size_t i=0, size = list.size(); i<size; i++)
	{
		if (!usedcmpnts[i]) continue;

		const ListComponent& cmp = list[i];
		const std::string& type = cmp.GetType();
		if (type == "W")
		{
			usedcmpnts[i] = false;
		}
		else if (isInstrumentNode(type))
		{
			// for now, just delete the probe and dmm nodes..
			
			bool used = true;
			const ListComponent::tConnections& cons = cmp.GetCConnections();
			for(size_t ci = 0; ci < cons.size(); ++ci)
			{
				if (cons[ci] == "0") continue; //< instruments connected to node "0" usually never connect the node directly
				if (!symbols.IsMarked(markData, cons[ci])) used = false;
			}

			if (!used)
			{
				OUT( OUTSTREAM << cmp.GetType() << " " << cmp.GetName() << " unused, removing" << endl );
				for(size_t ci = 0; ci < cons.size(); ++ci)
				{
					OUT(OUTSTREAM << "removing symbol " << cons[ci] << endl);
					symbols.Remove(cons[ci]);
				}
				
				usedcmpnts[i] = false;
			}
			else
			{
				OUT(OUTSTREAM << type << " " << cmp.GetName() << " in use" << endl);

				// measurement instruments are in use, but we don't want them in the input circuit
				// we still need to keep track of them so we can insert them after the circuit is solved
				if (type == "DMM"
					|| type == "PROBE"
					|| type == "PROBE1"
					|| type == "PROBE2"
					|| type == "PROBE3"
					|| type == "PROBE4"
					)
				{
					mMeasInstrument.push_back(cmp); // unnecessary copy
					usedcmpnts[i] = false;
				}
			}
		}
	}

	OUT(OUTSTREAM << "Tier two: Output symbol table" << endl);
	OUT(symbols.Dump());

	return true;
}

bool CircuitSolver3::BuildCandidateCache()
{
	mCandCache.resize(mIndexCircuit.size());
	for(size_t i=0,isize=mIndexCircuit.size(); i<isize; i++)
	{
		const ListComponent& c = mIndexCircuit[i];
		bool found = false;
		for(size_t j=0, candsize=mCandidates.size(); j<candsize; j++)
		{
			if (SpecialCompare(c, mCandidates[j]))
			{
				mCandCache[i].push_back(j);
				found = true;
			}
		}
		if (!found) return false;
	}
	
	for(size_t i=0,size = mCandidates.size(); i<size; i++)
	{
		const ListComponent& c = mCandidates[i];
		if (c.GetType() == "SHORTCUT" ) //|| c.GetType() == NamedNodes::DmmIProbe)
		{
			mShortcuts.push_back(i);
		}
	}

	return true;
}

bool CircuitSolver3::TierThree(const tCandidates& circuit, const tCandidates& candidates, tSymbols& symbols)
{
	mIndexCircuit.resize(circuit.size());
	copy(circuit.begin(), circuit.end(), mIndexCircuit.begin());

	mCandidates.resize(candidates.size());
	copy(candidates.begin(), candidates.end(), mCandidates.begin());
	
	size_t maxgroup = 0;
	for(size_t i=0,candsize=mCandidates.size(); i<candsize; i++)
	{
		const ListComponent& c = mCandidates[i];
		maxgroup = max(maxgroup, c.GetGroup());
	}
	
	tUsage usage(candidates.size() + maxgroup + 1, 0);

	if (!BuildCandidateCache()) return false;

	// the actual solver
	// walk the list, sorted in connection order, and try to find a component matching and insertable
	return TierThreeSolveRecursive(0, usage, symbols);
}

bool CircuitSolver3::TierThreeSolveRecursive(size_t circuitidx, tUsage& usage, tSymbols& symbols)
{
	if (circuitidx >= mIndexCircuit.size())
	{
		mSolutionSymbols = symbols;
		return true; // endcase
	}

	const ListComponent& current = mIndexCircuit[circuitidx];

#ifdef DEBUG_OUT
	OUTSTREAM << endl << "Recurse - solving component: " << current.Dump() << endl << "size: " << circuitidx << endl;
#endif

	// make a list of usable candidates, filtered from the candidate cache
	tUsage matching;
	const tUsage& potential = mCandCache[circuitidx];
	for(size_t i=0,potsize=potential.size(); i<potsize; i++)
	{
		//size_t idx = potential[i];
		//if (usage[potential[i]] == 0)
		if (!IsUsed(potential[i], usage))
		{
			matching.push_back(potential[i]);
		}
	}
	
	if (matching.empty()) return false;

	circuitidx++;

	if (TryCandidateList(current, matching, circuitidx, usage, symbols, false)) return true;
#ifdef DEBUG_OUT
	OUTSTREAM << "Unable to find a solution, trying with shortcuts" << endl;
#endif
	if (TryCandidateList(current, matching, circuitidx, usage, symbols, true)) return true;
	return false;
}

bool CircuitSolver3::TryCandidateList(const ListComponent& current, const tUsage& matched, size_t circuitidx, tUsage& usage, const tSymbols& symbols, bool withShortcuts)
{
	for(size_t i=0,msize=matched.size(); i<msize; i++)
	{
		bool isIprobe = false;
		if (current.GetType() == NamedNodes::DmmIProbe) {
			isIprobe = true; // hack..
			//if (withShortcuts) return false;
		}
		
		for(int turn=0;turn<2;++turn)
		{
			tSymbols	symbolcopy = symbols;
			tUsage		out;

			if (!InsertIfValid(current, matched[i], turn, usage, symbolcopy, out, withShortcuts)) {
				OUT(OUTSTREAM << "isn't valid" << endl);
				continue;
			}
			
			UpdateUsageMap(usage, out, 1);

			if (!TierThreeSolveRecursive(circuitidx, usage, symbolcopy)) {
				// no solution found, mark components as free again and continue
				UpdateUsageMap(usage, out, 0);
				continue;
			}

			// we found a solution, trace back and add the components to the solution
			
			OUT(OUTSTREAM << "leaving.." << endl);
			for(tUsage::const_iterator solveit = out.begin(),outend=out.end(); solveit != outend; ++solveit)
			{
				tUsage::const_iterator nextit = solveit;
				nextit++;
				// dirty hack!  we should probably mark the briding shortcuts and make sure only the original is matched
				if (isIprobe && nextit == out.end())
				{
					ListComponent iprobe = mCandidates[*solveit];
					InsertIProbe(iprobe, turn, current.GetName());
					mSolution.push_back(iprobe);
				}
				else
				{
					ListComponent comp = mCandidates[*solveit];
					if (current.GetSpecial() != "")
					{
						comp.SetSpecial(current.GetSpecial());
					}
					mSolution.push_back(comp);
				}
			}
			return true;
		}
	}

	return false;
}

bool CircuitSolver3::SpecialCompare(const ListComponent& c1, const ListComponent& c2) const
{
	if (c1.GetType() == NamedNodes::DmmIProbe && c2.GetType() == "SHORTCUT")
	{
		OUT(OUTSTREAM << "compare equal (iprobe/shortcut): " << c1.Dump() << " " << c2.Dump() << endl;);
		return true;
	}
	bool rv = c1.Equals(c2);
	return rv;
}

/*
bool CircuitSolver3::CheckTwoLeadComponent(const tSymbols& symbols,
										   const string& symbol1,
										   const string& symbol2,
										   const string& cmpnode1,
										   const string& cmpnode2)
{
#ifdef DEBUG_OUT
	OUTSTREAM << "try match: " << symbol1 << " " << symbol2 << " " << cmpnode1 << " " << cmpnode2 << endl;
#endif
	
	if (!symbols.ContainsSymbolOrEmpty(symbol1, cmpnode1))
	{
		// check if we can insert a shortcut to help us
		if (shortcut)
		{
			if (!SearchShortcuts(symbols, usage, node1, cmpnode1, out)) return false;
		}
		else return false;
	}
	
	if (!symbolcopy.ContainsSymbolOrEmpty(symbol2, cmpnode2))
	{
		// check if we can insert a shortcut to help us
		if (shortcut)
		{
			if (!SearchShortcuts(symbols, usage, node2, cmpnode2, out)) return false;
		}
		else return false;
	}
	
	if (symbolcopy.ContainsSymbol(symbol1, netcons[turn ? 0 : 1]))
	{
		OUT(OUTSTREAM << "SAME (1)" << endl);
		return false;
	}
	
	if (symbolcopy.ContainsSymbol(symbol2, netcons[turn ? 1 : 0]))
	{
		OUT(OUTSTREAM << "SAME (2)" << endl);
		return false;
	}

}

*/

bool CircuitSolver3::InsertIfValid(const ListComponent& circomp, size_t netcomp_idx, int turn, const tUsage& usage, tSymbols& symbolcopy, tUsage& out, bool shortcut)
{
	// we assume that that the compared components have equal number of connections..
	const ListComponent& netcomp = mCandidates[netcomp_idx];

	OUT(OUTSTREAM << "InsertIfValid: " << shortcut << " " << circomp.Dump() << " ?? " << netcomp.Dump() << endl;);

	if (netcomp.CanTurn())
	{
		// turning case
		const string& node1 = circomp.GetCConnections()[0];
		const string& node2 = circomp.GetCConnections()[1];

		const ListComponent::tConnections netcons = netcomp.GetCConnections();

#ifdef DEBUG_OUT
		const string& debug1 = netcomp.GetCConnections()[turn ? 1 : 0];
		const string& debug2 = netcomp.GetCConnections()[turn ? 0 : 1];
		OUTSTREAM << "try match: " << node1 << " " << node2 << " " << debug1 << " " << debug2 << endl;
#endif

		if (!symbolcopy.ContainsSymbolOrEmpty(node1, netcons[turn ? 1 : 0]))
		{
			// check if we can insert a shortcut to help us
			if (shortcut)
			{
				if (!SearchShortcuts(symbolcopy, usage, node1, netcons[turn ? 1 : 0], out)) return false;
			}
			else return false;
		}

		if (!symbolcopy.ContainsSymbolOrEmpty(node2, netcons[turn ? 0 : 1]))
		{
			// check if we can insert a shortcut to help us
			if (shortcut)
			{
				if (!SearchShortcuts(symbolcopy, usage, node2, netcons[turn ? 0 : 1], out)) return false;
			}
			else return false;
		}

		if (symbolcopy.ContainsSymbol(node1, netcons[turn ? 0 : 1]))
		{
			OUT(OUTSTREAM << "SAME (1)" << endl);
			return false;
		}

		if (symbolcopy.ContainsSymbol(node2, netcons[turn ? 1 : 0]))
		{
			OUT(OUTSTREAM << "SAME (2)" << endl);
			return false;
		}

		// these are the only ones modifying the symbol table
		if (!symbolcopy.Insert(node1, netcons[turn ? 1 : 0])) return false;
		if (!symbolcopy.Insert(node2, netcons[turn ? 0 : 1])) return false;
	}
	else
	{
		// safeguard! we should really not be here.. but the algo is written that way.. for now..
		if (turn == 1) return false;

		const size_t numCon = circomp.GetCConnections().size();
		for(size_t i = 0; i < numCon; ++i)
		{
			// NC* nodes are only allowed on multi legged components, ignore them as not existing
			const string& netnode = netcomp.GetCConnections()[i];
			if (netnode.size() > 2 && netnode[0] == 'N' && netnode[1] == 'C') continue; // skip NC nodes
			
			string symbol = circomp.GetCConnections()[i];
			if (!symbolcopy.ContainsSymbolOrEmpty(symbol, netcomp.GetCConnections()[i]))
			{
				// check if we can insert a shortcut to help us
				/// XXX: Why am i not only doing this when shortcut search is on?
				if (!SearchShortcuts(symbolcopy, usage, symbol, netcomp.GetCConnections()[i], out)) return false;
			}
			
			// Notice: if we failed on some iteration the symbolcopy will be invalid
			if (!symbolcopy.Insert(symbol, netnode)) return false;
		}
	}

#ifdef DEBUG_OUT
	OUTSTREAM << "INSERT COMPONENT - DUMPING SYMBOLS" << endl;
	OUTSTREAM << "MATCH: " << circomp.Dump() << " " << netcomp.Dump() << endl;
	symbolcopy.Dump();
	OUTSTREAM << endl;
#endif

	out.push_back(netcomp_idx);

	return true;
}

/// This will modify the candidates and symbol table
bool CircuitSolver3::SearchShortcuts(tSymbols& symbols2, const tUsage& usage, const string& endnode, const string& insertsymbol, tUsage& solution)
{
	// find the shortest shortcut path between start and end
	// end can be many symbols. XXX: ?

#ifdef DEBUG_OUT
	OUTSTREAM << "SEARCHING FOR SHORTCUT BRIDGE: " << endnode << " " << insertsymbol << endl << "ENDSYMBOL: " << endl;
	symbols2.DumpSymbol(endnode);

	OUTSTREAM << "Symbols to work with" << endl;
	symbols2.Dump();
#endif

	// first make a list of all candidate shortcuts
	tShorts usableshorts;
	usableshorts.reserve(mShortcuts.size());
	
	for(size_t i=0, size = mShortcuts.size(); i < size; i++)
	{
		size_t shrtidx = mShortcuts[i];
		//if (!usage[shrtidx])
		if (!IsUsed(shrtidx, usage))
		{
			const ListComponent::tConnections cons = mCandidates[shrtidx].GetCConnections();
			if (!symbols2.RefersSameNode(cons[0], cons[1]))
			{
				usableshorts.push_back(shrtidx);
			}
		}
	}

	if (usableshorts.empty()) return false;

#ifdef DEBUG_OUT
	OUTSTREAM << "USABLE SHORTCUTS" << endl; 
	DumpIndicesList(shortcuts, mCandidates);
#endif

	tVisit toVisit;
	// make the algorithm start at "startnode"
	toVisit.push_back(insertsymbol);

	bool rv = false;
	tTree tree;
	string lastnode;

#ifdef DEBUG_OUT
	OUTSTREAM << "Tracing shortcuts: " << endl;
#endif
	while(!toVisit.empty() && !rv)
	{
		lastnode = toVisit.front();
		if (symbols2.ContainsSymbol(endnode, lastnode)) rv = true;

		SearchShortcutVisit(toVisit.front(), toVisit, usableshorts, tree);
		toVisit.pop_front();
	}

#ifdef DEBUG_OUT
	OUTSTREAM << endl;
#endif

	tSymbols symbolscopy;
	if (rv)
	{
		symbolscopy = symbols2; // don't copy until we really need it copied
#ifdef DEBUG_OUT
		OUTSTREAM << "FOUND A BRIDGE!!" << endl;
#endif
		tCompIndices out; // just a index array
		//out.reserve(mShortcuts.size()); // might be possible to count tree depth
		SearchShortcutBacktrace(tree, lastnode, insertsymbol, out);

#ifdef DEBUG_OUT
		OUTSTREAM << "Dump Bridge" << endl;
		DumpIndicesList(out, mCandidates);
#endif

		for(tCompIndices::const_iterator it = out.begin(),outend=out.end(); it != outend; ++it)
		{
			const ListComponent& comp = mCandidates[*it];
			solution.push_back(*it);
			
			if (!symbolscopy.Insert(endnode, comp.GetCConnections()[0])) return false;
			if (!symbolscopy.Insert(endnode, comp.GetCConnections()[1])) return false;
		}

		symbols2 = symbolscopy;
		return true;
	}

	return false;
}

bool CircuitSolver3::SearchShortcutVisit(const string& current, tVisit& tovisit, tShorts& edges, tTree& tree) const
{
	// find any shortcut leading from current
	// if we find a bridge, return

#ifdef DEBUG_OUT
	OUTSTREAM << "Shortcut Visit: " << current << endl;
#endif
	tShorts::iterator it = edges.begin();
	while(it != edges.end())
	{
		bool next = true;
		const ListComponent::tConnections cons = mCandidates[*it].GetCConnections();

		if (cons[0] == current)
		{
			if (!tree[cons[1]])
			{
				tree[cons[1]] = *it;
				tovisit.push_back(cons[1]);
			}
			
			next = false;
		}
		else if (cons[1] == current)
		{
			if (!tree[cons[0]])
			{
				tree[cons[0]] = *it;
				tovisit.push_back(cons[0]);
			}
			next = false;
		}

		if (next) ++it;
		else
		{
			it = edges.erase(it);
		}
	}

	return true;
}

bool CircuitSolver3::SearchShortcutBacktrace(const tTree& tree, const string& lastnode, const string& startnode, tCompIndices& out) const
{
	// backtrace the found shortcut bridge

	string current = lastnode;
	bool done = false;

	while(!done)
	{
		tTree::const_iterator finder = tree.find(current);
		OUT(if (finder == tree.end()) OUTSTREAM << "Undefined node found in shortcut backtrace.." << endl);

		const size_t c_idx = finder->second; //.front();
		const ListComponent::tConnections cons = mCandidates[c_idx].GetCConnections();
		
		if (cons[0] == current)
		{
			out.push_back(c_idx);
			current = cons[1];
		}
		else if (cons[1] == current)
		{
			out.push_back(c_idx);
			current = cons[0];
		}

		if (current == startnode)
		{
			done = true;
			return true;
		}
	}

	// never reached..
	return false;
}

// utility functions
void CircuitSolver3::SetSymbol(const string& node, const string& symbol)
{
	mSymbols.Insert(node,symbol);
}

/*void CircuitSolver3::Add(ListComponent& comp)
{
	mCircuit.push_back(comp);
}*/

CircuitSolver3::tCircuit CircuitSolver3::GetSolution() const
{
	return mSolution;
}

bool CircuitSolver3::IsConnected(const string& node) const
{
	return mSolutionSymbols.IsUsed(node);
}

string CircuitSolver3::GetFirstSymbol(const string& sym)
{
	return mSolutionSymbols.GetFirstNodeOrSpare(sym);
}

void CircuitSolver3::InsertIProbe(ListComponent& component, int turn, const std::string& name)
{
	component.SetType(NamedNodes::DmmIProbe);
	component.SetSpecial(turn ? "1" : "0");
	component.SetName(name);
}

void CircuitSolver3::EnableLogging()
{
	InitLogging();
}


void CircuitSolver3::AddInstrumentNodes(tCircuit& list)
{
	for(tCircuit::const_iterator it = mMeasInstrument.begin(),mend=mMeasInstrument.end(); it != mend; ++it)
	{
		OUT(OUTSTREAM << "Adding instrument node for: " << it->GetType() << " " << it->GetName() << endl);

		// The DMM has 2 connections that needs to be matched
		if (it->GetType() == "DMM")
		{
			const string sym1 = GetFirstSymbol(it->GetCConnections()[0]);
			const string sym2 = GetFirstSymbol(it->GetCConnections()[1]);

			if (!sym1.empty() && !sym2.empty())
			{
				ListComponent aComp(it->GetType(), it->GetName(), sym1, sym2);
				list.push_back(aComp);
			}
		}
		else // for now we assume that everything else just have one connection
		{
			const string sym = GetFirstSymbol(it->GetCConnections()[0]);
			
			if (!sym.empty()) {
				ListComponent aComp(it->GetType(), it->GetName(), sym);
				list.push_back(aComp);
			}
		}
	}

	/*if (IsConnected("OSC_1"))
	{
		list.push_back(ListComponent(NamedNodes::OscProbe1, "1", GetFirstSymbol("OSC_1")));
	}

	if (IsConnected("OSC_2"))
	{
		list.push_back(ListComponent(NamedNodes::OscProbe2, "2", GetFirstSymbol("OSC_2")));
	}

	if (IsConnected("OSC_3"))
	{
		list.push_back(ListComponent(NamedNodes::OscProbe3, "3", GetFirstSymbol("OSC_3")));
	}

	if (IsConnected("OSC_4"))
	{
		list.push_back(ListComponent(NamedNodes::OscProbe4, "4", GetFirstSymbol("OSC_4")));
	}

	if (IsConnected("DMM_VHI") && IsConnected("DMM_VLO"))
	{
		list.push_back(ListComponent(NamedNodes::Dmm, "DMM", GetFirstSymbol("DMM_VHI"), GetFirstSymbol("DMM_VLO")));
	}
	*/
}

inline bool CircuitSolver3::IsUsed(size_t idx, const tUsage& usage) const
{
	if (mCandidates[idx].IsInGroup()) {
		return usage[usage.size() - mCandidates[idx].GetGroup()] != 0;
	} else {
		return usage[idx] != 0;
	}
}

inline void CircuitSolver3::UpdateUsageMap(tUsage& usage, const tUsage& update, size_t state)
{
	for(CircuitSolver3::tUsage::const_iterator it = update.begin(), end = update.end(); it != end; it++)
	{
		if (mCandidates[*it].IsInGroup()) {
			usage[usage.size() - mCandidates[*it].GetGroup()] = state;
		} else {
			usage[*it] = state;
		}
	}
}

std::string DumpCandidateList(const CircuitSolver3::tCandidates& list)
{
	stringstream out;
	out << "-CANDIDATE DUMP-" << endl;
	for(CircuitSolver3::tCandidates::const_iterator it = list.begin(); it != list.end(); ++it)
	{
		out << "Dump: " << it->Dump() << endl;
	}
	
#ifdef DEBUG_OUT
	OUTSTREAM << out.str() << endl;
#endif
	return out.str();
}

std::string DumpIndicesList(const CircuitSolver3::tCompIndices& list, const CircuitSolver3::tVectorCircuit& circuit)
{
	stringstream out;
	out << "-INDICES DUMP-" << endl;
	for(CircuitSolver3::tCompIndices::const_iterator it = list.begin(); it != list.end(); ++it)
	{
		out << "Dump: " << circuit[*it].Dump() << endl;
	}
	
#ifdef DEBUG_OUT
	OUTSTREAM << out.str() << endl;
#endif
	
	return out.str();
}
