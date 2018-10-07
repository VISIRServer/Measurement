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

#ifndef __LIST_PARSER_H__
#define __LIST_PARSER_H__

#include "listcomponent.h"

#include <string>
#include <list>
#include <map>

class ComponentTypeDefinition
{
public:
	ComponentTypeDefinition()
	{
		mType = "Undefined";
		mNumCons = 0;
		mCanTurn = mIgnoreValue = mHasSpecialValue = false;
	}
	ComponentTypeDefinition(std::string type, int cons, bool canTurn = false, bool ignoreValue = false, bool specialValue = false)
	{
		mType		= type;
		mNumCons	= cons;
		mCanTurn	= canTurn;
		mIgnoreValue		= ignoreValue;
		mHasSpecialValue	= specialValue;
	}
	const std::string& Type() const		{ return mType; }
	const int	NumConnections() const	{ return mNumCons; }
	const bool	IgnoreValue() const		{ return mIgnoreValue; }
	const bool	HasSpecialValue() const	{ return mHasSpecialValue; }
private:
	std::string	mType;
	int			mNumCons;
	bool		mCanTurn;
	bool		mIgnoreValue;
	bool		mHasSpecialValue;
};

/// General list parser.
class ListParser
{
public:
	typedef std::list<ComponentTypeDefinition> tComponentDefinitions;
	
	/// Parses a netlist and stores it in a nodelist.
	/// returns false if Parser fails.
	bool	Parse(std::string aList);
	bool	ParseFile(const std::string& filename);
	const ListComponent::tComponentList&	GetList() const;

	ListParser(const tComponentDefinitions& definitions);
private:
	/// Create a component from a given list line
	ListComponent*	CreateComponent(const std::string& instring);
	bool			IsComment(const std::string& instring);

	ListComponent::tComponentList	mComponentList;

	typedef std::map<std::string,ComponentTypeDefinition> tCompDefMap;
	tCompDefMap			mCompDefMap;

	const ComponentTypeDefinition* GetTypeDefinition(const std::string& type) const;
};

#endif
