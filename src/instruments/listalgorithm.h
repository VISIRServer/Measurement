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

#ifndef __LIST_ALGORITHM_H__
#define __LIST_ALGORITHM_H__

#include "listcomponent.h"

#include <vector>
#include <string>
#include <list>

/// Utility class for list operations, such as set operations and node lookup
class ListAlgorithm
{
public:
	typedef	std::vector<ListComponent>	tComponentList;
	typedef std::pair<size_t, size_t> tIndexPair;
	typedef std::list< tIndexPair >	tIndexPairs;

	static bool CircuitSetupMatch(const tComponentList& circuit, const tComponentList& compList, tComponentList& out);

	static bool	MatchNode(const ListComponent& comp, const tComponentList& list, ListComponent& out);
	static bool	IsSubsetOf(const tComponentList& subset, const tComponentList& superset);
	static bool IsSubsetOfWithFailed(const tComponentList& subset, const tComponentList& superset, tComponentList& failed);

	static bool MatchIndex(const tComponentList& subset, const tComponentList& superset, tIndexPairs& out);

	/// Finds component of same type and value and replaces with components in the replace list.
	/// Returns nothing, but fills the out array with the result.
	static void	Replace(const ListComponent& comp, const tComponentList& list, const tComponentList& replace, tComponentList& out);

	/// Finds named component and replaces it with another.
	/// XXX: use outlist or not?
	static void	ReplaceNamed(const std::string& name, tComponentList& list, const ListComponent& comp);

	static void	ReplaceTypeWithList(const std::string& name, tComponentList& list, const tComponentList& replace);

	static void	ReplaceType(const std::string& type, tComponentList& list, std::string newtype);

	///	Seek nodes of a certain type in the list.
	static void	PushNodesOfType(const std::string& type, const tComponentList& list, tComponentList& out);

	static void	RemoveOfType(const std::string& type, tComponentList& out);
private:
	static bool	Match(const tComponentList& subset, const tComponentList& superset, tComponentList& out);
	static bool	TestSubNode(tComponentList& matchlist, const tComponentList& superset, const ListComponent& comp);
};

#endif
