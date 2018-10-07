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

#include "circuitsymbols2.h"
#include "circuitsolverinternal.h"

#include <sstream>
#include <algorithm>

#include <assert.h>
#include <basic_exception.h>

#define NEW_SYMBOLS
#define NEW_NODES

#define NodeToIdxSize ( sizeof(mNodeToIdx) / sizeof(mNodeToIdx[0]) )

const char* cSymbolReverseLookup = "0ABCDEFGHI";

SymbolTable::SymbolTable()
{
	for(size_t i = 0;i < NodeToIdxSize; i++) mNodeToIdx[i] = -1;
	mSymbolCounter = 0;
}

/// add a reference between sym1 and sym2
bool SymbolTable::Ref(const std::string& sym1, const std::string& sym2)
{
	// get both symbols
	// if both exist, merge and update index
	const int idx1 = GetIndexOf(sym1);
	const int idx2 = GetIndexOf(sym2);

	if ( (idx1 != -1) && (idx1 == idx2)) // already the same
	{
		return true;
	}
	if (idx1 >= 0 && idx2 >= 0)
	{
		if (idx1 < idx2)
		{
			if (!MergeNodes(idx1, idx2, sym2)) return false;
		}
		else
		{
			if (!MergeNodes(idx2, idx1, sym1)) return false;
		}
	}
	else if (idx1 >= 0)
	{
		GetSymbolRef(sym2) = idx1;
	}
	else if (idx2 >= 0)
	{
		GetSymbolRef(sym1) = idx2;
	}
	else
	{
		size_t newindex = CreateNode();
		GetSymbolRef(sym1) = newindex;
		GetSymbolRef(sym2) = newindex;
	}

	return true;
}

/// finds the symbol and adds the node to it
/// if the node exists elsewhere, it will return false
/// Notice: Can't just return false if the node is defined elsewhere as the function is used with or without verification
bool SymbolTable::Insert(const std::string& sym, const std::string& node)
{
	ValidateNodeName(node);
	
	bool rv = true;
	const int idx = GetIndexOf(sym);
	if (idx < 0)
	{
		int newindex = CreateNode(node[0]);
		if (newindex < 0) return false;
		GetSymbolRef(sym) = newindex;
	}
	else
	{
		return (AddNodeToIdx(idx, node[0]) >= 0);
	}
	return rv;
}

void SymbolTable::Remove(const std::string& sym)
{
	EraseNodeForSymbol(sym);
	EraseSymbol(sym);
}

bool SymbolTable::ContainsSymbolOrEmpty(const std::string& sym, const std::string& node) const
{
	ValidateNodeName(node);
	
	const int idx = GetIndexOf(sym);
	if (idx < 0) return true;
	if (NodesUsedForIdx(idx) == 0) return true;
	return IdxUsesNode(idx, node[0]);
}

bool SymbolTable::ContainsSymbol(const std::string& sym, const std::string& node) const
{
	ValidateNodeName(node);
	const int idx = GetIndexOf(sym);
	if (idx < 0) return false;
	return IdxUsesNode(idx, node[0]);
}

bool SymbolTable::IsUsed(const std::string& sym) const
{
	const int idx = GetIndexOf(sym);
	if (idx >= 0)
	{
		return (NodesUsedForIdx(idx) > 0);
	}
	return false;
}

bool SymbolTable::RefersSameSymbol(const std::string& sym1, const std::string& sym2) const
{
	const int idx1 = GetIndexOf(sym1);
	const int idx2 = GetIndexOf(sym2);
	if ( (idx1 >= 0) && (idx2 >= 0) && (idx1 == idx2)) return true;
	return false;
}

bool SymbolTable::RefersSameNode(const std::string& node1, const std::string& node2) const
{
	ValidateNodeName(node1);
	ValidateNodeName(node2);

	const int idx1 = FindNodeIdx(node1[0]);
	const int idx2 = FindNodeIdx(node2[0]);
	if ( (idx1 >= 0) && (idx2 >= 0) && (idx1 == idx2)) return true;
	return false;
}

std::string	SymbolTable::GetFirstNodeOrSpare(const std::string& sym)
{
	tNodeName out = tNodeName();
	const int idx = GetIndexOf(sym);
	if (idx >= 0) out = GetFirstNodeForIdx(idx);

	if (out != tNodeName()) return std::string(1, out);

	//const char* allowedNodes[] = { "A", "B", "C", "D", "E", "F", "G", "H", "I" };
	const char* allowedNodes = "ABCDEFGHI";
	typedef std::set<tNodeName> tSetType;
	//tSetType allowed(allowedNodes, allowedNodes + (sizeof(allowedNodes) / sizeof(allowedNodes[0])));
	tSetType allowed(allowedNodes, allowedNodes + strlen(allowedNodes));

	tSetType used;
#ifdef NEW_NODES
	for(size_t i=0;i<NodeToIdxSize; ++i)
	{
		if (mNodeToIdx[i] > -1) used.insert(cSymbolReverseLookup[i]);
	}
#else
	for(tVec::const_iterator vec_it = mVec.begin(), end = mVec.end(); vec_it != end; ++vec_it)
	{
		vec_it->CopyTo(used);
	}
#endif

	tSetType diff;
	set_difference(allowed.begin(), allowed.end(), used.begin(), used.end(), inserter(diff, diff.begin()));

	if (diff.empty()) return "NOSPARE";
	out = *diff.begin();
	std::string strout(1, out);
	Insert(sym, strout);
	return strout;
}

void SymbolTable::Mark(tMarked& markdata, const std::string& sym)
{
	// should this create the node if it doesn't exist?
	const int idx = GetIndexOf(sym);
	if (idx >= 0)
	{
		markdata.insert(idx);
	}
	else
	{
		size_t newindex = CreateNode();
		markdata.insert(newindex);
		GetSymbolRef(sym) = newindex;
	}
}

bool SymbolTable::IsMarked(const tMarked& markdata, const std::string& sym) const
{
	const int idx = GetIndexOf(sym);
	if (idx >= 0)
	{
		tMarked::const_iterator f = markdata.find(idx);
		return (f != markdata.end());
	}
	return false;
}

void SymbolTable::Dump() const
{
#ifdef DEBUG_OUT
	OUTSTREAM << DumpSymbols() << "- " << DumpNodes() << std::endl;
#endif
}

void SymbolTable::DumpSymbol(const std::string& sym)
{
#ifdef DEBUG_OUT
	int idx = GetIndexOf(sym);
	if (idx >= 0) mVec[idx].Dump();
#endif
}

///////////////////////////////////////////////

SymbolTable::tSymbolLookup SymbolTable::sSymbolLookup;

void SymbolTable::ToSymbolName(const std::string& symbol, tSymbolName& out)
//SymbolTable::tSymbolName SymbolTable::ToSymbolName(const std::string& symbol)
{
	if (symbol.size() >= 16) {
		//std::cerr << "Symbol name longer than 15 chars";
		const char* invalid = "<INVALID>";
		strcpy(out._name, invalid);
	} else {
		strcpy(out._name, symbol.c_str());
	}
}

int SymbolTable::LookupSymbol(const std::string& symbol) const
{
	for(size_t i = 0, size = sSymbolLookup.size(); i < size; ++i)
	{
		if (symbol == sSymbolLookup[i]._name) return i;
	}
	return -1;
}

/*int SymbolTable::GetSymbolIdx(const std::string& symbol) const
{
#ifdef NEW_SYMBOLS
	int idx = LookupSymbol(symbol);
	if (idx < 0) return -1;
	return mSymbolToNodeIdx[idx];
#else
	return -1;
#endif
}*/

int& SymbolTable::GetSymbolRef(const std::string& symbol)
{
#ifdef NEW_SYMBOLS
	int idx = LookupSymbol(symbol);
	if (idx >= 0) return mSymbolToNodeIdx[idx];

	tSymbolName symname;
	ToSymbolName(symbol, symname);
	
	size_t newidx = sSymbolLookup.size();
	sSymbolLookup.push_back(symname);
	if (mSymbolToNodeIdx.size() <= newidx) {
		mSymbolToNodeIdx.resize((newidx+1)*2);
	}
	mSymbolToNodeIdx[newidx] = -1;
	return mSymbolToNodeIdx[newidx];
#else
	return mMap[symbol];
#endif
}

void SymbolTable::ResetLookup()
{
	sSymbolLookup = tSymbolLookup();
}

int SymbolTable::GetIndexOf(const std::string& sym) const
{
#ifdef NEW_SYMBOLS
	int idx = LookupSymbol(sym);
	if (idx < 0) return -1;
	return mSymbolToNodeIdx[idx];
	//return GetSymbolIdx(sym);
#else
	tMap::const_iterator finder = mMap.find(sym);
	if (finder == mMap.end()) return -1;
	else return finder->second;
#endif
}

void SymbolTable::UpdateRef(int ref, int newref)
{
	assert(ref != -1 && "ref can't be -1");
	assert(newref != -1 && "newref can't be -1");
	
#ifdef NEW_SYMBOLS
	for(tSymbolToNodeIdx::iterator it = mSymbolToNodeIdx.begin(), end = mSymbolToNodeIdx.end(); it != end; ++it)
	{
		if (*it == ref) *it = newref;
	}
#else
	for(tMap::iterator it = mMap.begin(); it != mMap.end(); it++)
	{
		if (it->second == ref)
		{
			it->second = newref;
		}
	}
#endif
}

void SymbolTable::EraseSymbol(const std::string& symbol)
{
#ifdef NEW_SYMBOLS
	GetSymbolRef(symbol) = -1;
#else
	mMap.erase(symbol);
#endif
}

std::string SymbolTable::DumpSymbols() const
{
	std::stringstream outbuffer;
#ifdef NEW_SYMBOLS
	for(size_t i = 0; i< sSymbolLookup.size(); ++i)
	{
		outbuffer << "(" << std::string(sSymbolLookup[i]._name) << "-" << mSymbolToNodeIdx[i] << ") ";
	}
#else
	for(tMap::const_iterator map_it = mMap.begin(); map_it != mMap.end(); ++map_it)
	{
		outbuffer << "(" << map_it->first << "-" << map_it->second << ") ";
	}
#endif
	return outbuffer.str();
}

///////////////////////////////////////////////

void SymbolTable::ValidateNodeName(const std::string& node) const
{
	if (node.size() > 1) {
		throw BasicException("Invalid node name found in symboltable");
	}
}

/// look if perticular node is used somewhere and return the index
int SymbolTable::FindNodeIdx(tNodeName node) const
{
#ifdef NEW_NODES
	size_t nodeid = LookupNode(node);
	return mNodeToIdx[nodeid];
#else
	for (size_t idx = 0; idx < mVec.size(); ++idx)
	{
		if (mVec[idx].HasNode(node)) return idx;
	}
	return -1;
#endif
}

// XXX: Might be possible to remove srcsym parameter
bool SymbolTable::MergeNodes(int dst, int src, const std::string& srcsym)
{
#ifdef NEW_NODES
	for(size_t i=0;i<NodeToIdxSize; ++i)
	{
		if (mNodeToIdx[i] == src) {
			mNodeToIdx[i] = dst;
		}
	}
#else
	mVec[dst].AddNodesFrom(mVec[src]);
	mVec[src] = SymNode(); // Remove old Node
#endif

	GetSymbolRef(srcsym) = dst;
	UpdateRef(src, dst);
	if (NodesUsedForIdx(dst) > 1) return false;
	return true;
}

int SymbolTable::CreateNode()
{
#ifdef NEW_NODES
	return mSymbolCounter++;
#else
	size_t newindex = mVec.size();
	mVec.push_back(SymNode());
	return newindex;
#endif
}

int	SymbolTable::CreateNode(tNodeName node)
{
#ifdef NEW_NODES
	size_t nodeid = LookupNode(node);
	if (mNodeToIdx[nodeid] >= 0) return -1; // used elsewhere
	int newSym = mSymbolCounter++;
	mNodeToIdx[nodeid] = newSym;
	return newSym;
#else
	
	int findidx = FindNodeIdx(node);
	if (findidx >= 0) {
#ifdef DEBUG_OUT
		OUTSTREAM << findidx << " is already using the node that is being inserted" << std::endl;
#endif
		return -1;
	}
	
	size_t newindex = mVec.size();
	SymNode symnode;
	symnode.AddNode(node);
	mVec.push_back(symnode);
	return newindex;
#endif
}

int SymbolTable::AddNodeToIdx(int idx, tNodeName node)
{
#ifdef NEW_NODES
	size_t nodeid = LookupNode(node);
	if (idx == mNodeToIdx[nodeid]) return idx; // we're already using the node, ok
	if (mNodeToIdx[nodeid] != -1) return -1; // someone else is using this node
	
	mNodeToIdx[nodeid] = idx;
	return idx;
#else
	// verify that no other SymNode has the same node first before inserting
	int findidx = FindNodeIdx(node);
	if (findidx >= 0) {
		if (idx != findidx)
		{
#ifdef DEBUG_OUT
			OUTSTREAM << findidx << " is already using the node that is being inserted" << std::endl;
#endif
			return -1;
		}
	}
	
	mVec[idx].AddNode(node);
	return idx;
#endif
}

std::string SymbolTable::DumpNodes() const
{
	std::stringstream out;
#ifdef NEW_NODES
	for(size_t i=0;i<NodeToIdxSize; ++i)
	{
		if (mNodeToIdx[i] > -1) out << "(" << mNodeToIdx[i] << ") " << cSymbolReverseLookup[i] << " ";
	}
#else
	size_t i = 0;
	for(tVec::const_iterator vec_it = mVec.begin(); vec_it != mVec.end(); ++vec_it)
	{
		out << "(" << i << ") " << vec_it->Dump() << " ";
		++i;
	}
#endif
	return out.str();
}

SymbolTable::tNodeName SymbolTable::ReverseLookupNode(size_t i) const
{
	if (i >= NodeToIdxSize) throw BasicException("Out of bounds in ReverseLookupNode");
	return cSymbolReverseLookup[i];
}

size_t SymbolTable::LookupNode(tNodeName node) const
{
	if (node >= 'A' && node <= 'I') return node - 'A' + 1;
	if (node == '0') return 0;

	throw BasicException("Invalid node name found in symboltable node lookup");
	return 0; // never reached
}

void SymbolTable::EraseNodeForSymbol(const std::string& sym)
{
#ifdef NEW_NODES
	int idx = GetIndexOf(sym);
	if (idx >= 0)
	{
		for(size_t i=0;i<NodeToIdxSize; ++i)
		{
			if (mNodeToIdx[i] == idx) mNodeToIdx[i] = -1;
		}
	}
#else
	int idx = GetIndexOf(sym);
	if (idx >= 0)
	{
		mVec[idx] = SymNode();
		
	}
#endif
}

size_t SymbolTable::NodesUsedForIdx(int idx) const
{
#ifdef NEW_NODES
	size_t usage = 0;
	for(size_t i=0;i<NodeToIdxSize; ++i)
	{
		if (mNodeToIdx[i] == idx) usage++;
	}
	return usage;
#else
	return mVec[idx].NumNodes();
#endif
}

bool SymbolTable::IdxUsesNode(int idx, tNodeName node) const
{
#ifdef NEW_NODES
	size_t nodeid = LookupNode(node);
	return (mNodeToIdx[nodeid] == idx);
#else
	return mVec[idx].HasNode(node);
#endif
}

SymbolTable::tNodeName SymbolTable::GetFirstNodeForIdx(int idx) const
{
#ifdef NEW_NODES
	for(size_t i=0;i<NodeToIdxSize; ++i)
	{
		if (mNodeToIdx[i] == idx) return ReverseLookupNode(i);
	}
	return tNodeName();
#else
	return mVec[idx].GetFirstNode();
#endif
}
