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

#ifndef __LIST_COMPONENT_H__
#define __LIST_COMPONENT_H__

#include <vector>
#include <string>

/// General list component/entry/node.
class ListComponent
{	
public:
	/// utility definition for vectors of components
	typedef std::vector<ListComponent>	tComponentList;
	typedef std::vector<std::string>	tConnections;

	ListComponent();
	~ListComponent();

	ListComponent(std::string type, std::string name);
	ListComponent(std::string type, std::string name, std::string con1);

	/// ctor for standard 2 connection components..
	ListComponent(std::string type, std::string name, std::string con1, std::string con2);

	bool	Equals(const ListComponent& other) const;

	// only name can differ
	bool EqualsWithConnection(const ListComponent& other) const;
	bool operator==(const ListComponent& other) const;
	bool operator<(const ListComponent& other) const;

	// XXX: this info should come from the definition and not be hard coded
	bool CanTurn() const;

	inline const tConnections&	GetCConnections() const		{ return mConnections; }
	inline const std::string&	GetName() const				{ return mName; }
	inline const std::string&	GetType() const				{ return mType; }
	inline const std::string&	GetValue() const			{ return mValue; }
	inline const std::string&	GetSpecial() const			{ return mSpecial; }

	void	AddConnection(std::string con)	{ mConnections.push_back(con);	}
	void	SetValue(std::string value)		{ mValue = value;		}
	void	SetType(std::string newtype)	{ mType = newtype;		}
	void	SetSpecial(std::string special)	{ mSpecial = special;	}
	void	SetName(std::string name)		{ mName = name;	}
	
	bool	IsInGroup() const { return mGroupID != 0; }
	size_t	GetGroup() const { return mGroupID; }
	void	SetGroup(size_t groupid) { mGroupID = groupid; }

	std::string Dump() const;
	std::string GetSpecialToken(const std::string& name) const;

private:
	std::string		mType;
	std::string		mName;
	std::string		mValue;
	std::string		mSpecial;
	tConnections	mConnections;
	size_t			mGroupID;
};

#endif
