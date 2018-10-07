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

#ifndef __CIRCUIT_SYMBOLS2_H__
#define __CIRCUIT_SYMBOLS2_H__

//#include <algorithm>
#include <string>
#include <map>
//#include <list>
#include <set>
#include <vector>

/*class SymNode
{
private:
	typedef char tNodeName;
	typedef std::set<tNodeName> tNodes;
public:
	SymNode();
	
	void AddNodesFrom(const SymNode& other);
	void AddNode(const tNodeName& node);
	bool HasNode(const tNodeName& node) const;
	size_t NumNodes() const;
	
	inline tNodeName GetFirstNode() const
	{
		if (!mNodes.empty()) return *mNodes.begin();
		return tNodeName();
	}

	std::string Dump() const;

	void CopyTo(tNodes& to) const;

private:
	tNodes		mNodes;
};
 */

class SymbolTable
{
public:
	SymbolTable();
	
	///		Add a reference between sym1 and sym2
	///		Returns false if more than one node is defined in the resulting symbol
	bool	Ref(const std::string& sym1, const std::string& sym2);

	///		Finds the symbol and adds the node to it
	///		If the node exists elsewhere, it will return false and no node is added
	bool	Insert(const std::string& sym, const std::string& node);

	void	Remove(const std::string& sym);

	bool	ContainsSymbolOrEmpty(const std::string& sym, const std::string& node) const;
	bool	ContainsSymbol(const std::string& sym, const std::string& node) const;
	
	bool	IsUsed(const std::string& sym) const;
	bool	RefersSameSymbol(const std::string& sym1, const std::string& sym2) const;
	bool	RefersSameNode(const std::string& sym1, const std::string& sym2) const;

	///		Utility function used by instruments to get the first used node name
	///		or a spare node if none is defined
	///		returns "NOSPARE" if no spares are available
	std::string	GetFirstNodeOrSpare(const std::string& sym);

	typedef std::set<int>	tMarked;
	void	Mark(tMarked& markdata, const std::string& sym);
	bool	IsMarked(const tMarked& markdata, const std::string& sym) const;

	void	Dump() const;
	void	DumpSymbol(const std::string& sym);
	
	static void	ResetLookup();
private:
	typedef char tNodeName;

	// returns the index of the symbol if it exist
	int GetIndexOf(const std::string& sym) const;

	// Throws if invalid node name is found
	void	ValidateNodeName(const std::string& node) const;
	
	/// look if perticular node is used somewhere and return the index
	int		FindNodeIdx(tNodeName node) const;
	bool	MergeNodes(int dst, int src, const std::string& srcsym);
	int		CreateNode();
	int		CreateNode(tNodeName node);
	int		AddNodeToIdx(int idx, tNodeName node);

	std::string DumpSymbols() const;
	std::string	DumpNodes() const;
	
	void UpdateRef(int ref, int newref);
	
	int		LookupSymbol(const std::string& symbol) const;
	int&	GetSymbolRef(const std::string& symbol);
	
	void	EraseSymbol(const std::string& symbol);

	struct tSymbolName
	{
		char	_name[16];
	};
	
	void ToSymbolName(const std::string& symbol, tSymbolName& out);

	typedef std::vector<tSymbolName>	tSymbolLookup;
	static tSymbolLookup sSymbolLookup;
	
	typedef std::vector<int>	tSymbolToNodeIdx;
	tSymbolToNodeIdx mSymbolToNodeIdx;
	
	size_t		LookupNode(tNodeName node) const;
	tNodeName	ReverseLookupNode(size_t i) const;
	void		EraseNodeForSymbol(const std::string& sym);
	size_t		NodesUsedForIdx(int idx) const;
	bool		IdxUsesNode(int idx, tNodeName node) const;
	tNodeName	GetFirstNodeForIdx(int idx) const;

	typedef		int tNodeToIdx[10];
	tNodeToIdx		mNodeToIdx;
	
	size_t		mSymbolCounter;
	
	
	//tMap	mMap;
	//tVec	mVec;
};

#endif
