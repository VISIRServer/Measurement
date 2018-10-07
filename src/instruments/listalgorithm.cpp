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

#include "listalgorithm.h"

#include <algorithm>

bool ListAlgorithm::MatchNode(const ListComponent& comp, const tComponentList& list, ListComponent& out)
{
	for(tComponentList::const_iterator it = list.begin(); it != list.end(); it++)
	{
		if (comp.EqualsWithConnection(*it))
		{
			out = *it;
			return true;
		}
	}

	return false;
}

bool ListAlgorithm::IsSubsetOf(const tComponentList& subset, const tComponentList& superset)
{
	tComponentList matchlist;
	return Match(subset, superset, matchlist);
}

bool ListAlgorithm::IsSubsetOfWithFailed(const tComponentList& subset, const tComponentList& superset, tComponentList& failed)
{
	tComponentList matchlist;
	//return Match(subset, superset, matchlist);
	
	bool rv = true;

	for(tComponentList::const_iterator it = subset.begin(); it != subset.end(); it++)
	{
		if (!TestSubNode(matchlist, superset, *it))
		{
			failed.push_back(*it);
			rv = false;
		}
	}
	return rv;
}

bool ListAlgorithm::Match(const tComponentList& subset, const tComponentList& superset, tComponentList& out)
{
	for(tComponentList::const_iterator it = subset.begin(); it != subset.end(); it++)
	{
		if (!TestSubNode(out, superset, *it))
		{
			return false;
		}
	}
	return true;
}

bool ListAlgorithm::TestSubNode(tComponentList& matchlist, const tComponentList& superset, const ListComponent& comp)
{
	for(tComponentList::const_iterator it = superset.begin(); it != superset.end(); it++)
	{
		// check if the nodes match
		if (comp.EqualsWithConnection(*it))
		{
			// and the node is not already used
			if (find(matchlist.begin(), matchlist.end(),*it) == matchlist.end())
			{
				matchlist.push_back(*it);
				return true;
			}
		}
	}
	return false;
}

bool ListAlgorithm::MatchIndex(const tComponentList& subset, const tComponentList& superset, tIndexPairs& out)
{
	std::vector<size_t> used(superset.size(), 0);

	for(size_t subi = 0; subi<subset.size(); ++subi)
	{
		bool matched = false;
		for(size_t superi = 0; !matched && superi<superset.size(); ++superi)
		{
			if ( (used[superi] == 0) && subset[subi].EqualsWithConnection(superset[superi]))
			{
				matched = true;
				used[superi] = 1;				
				out.push_back( tIndexPair(superi, subi) );
			}
		}

		if (!matched)
		{
			out = tIndexPairs();
			return false;
		}
	}

	return true;
}

void ListAlgorithm::Replace(const ListComponent& comp, const tComponentList& list, const tComponentList& replace, tComponentList& out)
{
	for(tComponentList::const_iterator it = list.begin(); it != list.end(); it++)
	{
		if (comp.Equals(*it))
		{
			for(tComponentList::const_iterator repit = replace.begin(); repit != replace.end(); repit++)
			{
				out.push_back(*repit);
			}
		}
		else
		{
			out.push_back(*it);
		}
	}
}

void ListAlgorithm::ReplaceNamed(const std::string& name, tComponentList& list, const ListComponent& comp)
{
	for(tComponentList::iterator it = list.begin(); it != list.end(); it++)
	{
		if (name == it->GetName())
		{
			*it = comp;
		}
	}
}

void ListAlgorithm::ReplaceTypeWithList(const std::string& name, tComponentList& list, const tComponentList& replace)
{
	tComponentList out;
	for(tComponentList::const_iterator it = list.begin(); it != list.end(); it++)
	{
		if (it->GetType() == name)
		{
			for(tComponentList::const_iterator repit = replace.begin(); repit != replace.end(); repit++)
			{
				out.push_back(*repit);
			}
		}
		else
		{
			out.push_back(*it);
		}
	}

	list = out;
}

void ListAlgorithm::ReplaceType(const std::string& type, tComponentList& list, std::string newtype)
{
	for(tComponentList::iterator it = list.begin(); it != list.end(); it++)
	{
		if (type == it->GetType())
		{			
			it->SetType(newtype);
		}
	}
}

void ListAlgorithm::PushNodesOfType(const std::string& type, const tComponentList& list, tComponentList& out)
{
	for(tComponentList::const_iterator it = list.begin(); it != list.end(); it++)
	{
		if (type == it->GetType())
		{
			out.push_back(*it);
		}
	}
}

void ListAlgorithm::RemoveOfType(const std::string& type, tComponentList& out)
{
	tComponentList temp;
	for(tComponentList::const_iterator it = out.begin(); it != out.end(); it++)
	{
		if (type != it->GetType()) temp.push_back(*it);
	}

	out = temp;
}

/*
void ListAlgorithm::GetOfType(string type, tComponentList& out)
{
	tComponentList temp;
	for(tComponentList::const_iterator it = out.begin(); it != out.end(); it++)
	{
		if (type == it->GetType()) temp.push_back(*it);
	}

	out = temp;
}
*/

bool ListAlgorithm::CircuitSetupMatch(const tComponentList& inCircuit, const tComponentList& compList, tComponentList& out)
{
	tComponentList nodes = inCircuit; // copy

	// Clean up netlist for matching
	// Remove instruments and convert/remove switches
	RemoveOfType("DMM", nodes);
	RemoveOfType("IPROBE", nodes);
	RemoveOfType("PROBE1", nodes);
	RemoveOfType("PROBE2", nodes);
	RemoveOfType("PROBE3", nodes);
	RemoveOfType("PROBE4", nodes);

	RemoveOfType("XSWITCHCLOSE", nodes);
	ReplaceType( "XSWITCHOPEN",  nodes, "SHORTCUT");

	// check if we have all nodes in hw
	if (Match(nodes, compList, out))
	{
		return true;
	}

	return false;
}