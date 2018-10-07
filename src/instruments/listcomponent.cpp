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

#include "listcomponent.h"

ListComponent::ListComponent() {}
ListComponent::~ListComponent() {}

ListComponent::ListComponent(std::string type, std::string name)
{
	mType	= type;
	mName	= name;
	mGroupID = 0;
}


ListComponent::ListComponent(std::string type, std::string name, std::string con1)
{
	mType	= type;
	mName	= name;
	mGroupID = 0;
	mConnections.push_back(con1);
	mGroupID = 0;
}

ListComponent::ListComponent(std::string type, std::string name, std::string con1, std::string con2)
{
	mType	= type;
	mName	= name;
	mGroupID = 0;
	mConnections.push_back(con1);
	mConnections.push_back(con2);
	mGroupID = 0;
}

bool ListComponent::Equals(const ListComponent& other) const
{
	if (mType == other.mType && mValue == other.mValue) return true;
	return false;
}

bool ListComponent::EqualsWithConnection(const ListComponent& other) const
{
	if (mType != other.mType || mValue != other.mValue) return false;
	if (mConnections != other.mConnections) return false;
	return true;
}

bool ListComponent::operator==(const ListComponent& other) const
{
	if (!Equals(other)) return false;
	if (mName != other.mName) return false;
	if (mConnections != other.mConnections) return false;
	return true;
}

bool ListComponent::operator<(const ListComponent& other) const
{
	// sorting is done by combining the type and name of component..
	std::string c1 = mType + mName + mValue;
	std::string c2 = other.mType + other.mName + other.mValue;
	if (c1 < c2) return true;
	else if (c2 < c1) return false;

	// else both are the same.. compare connections
	return mConnections < other.mConnections;
}

// XXX: this info should come from the definition and not be hard coded
bool ListComponent::CanTurn() const
{
	if (	mType == "R" 
		||	mType == "L"
		||	mType == "C"
		||	mType == "SHORTCUT"
		||	mType == "XSWITCHOPEN"
		||	mType == "XSWITCHCLOSE"
		||	mType == "IPROBE") return true;
	return false;
}

std::string ListComponent::Dump() const
{
	std::string out = "Component: ";
	out += "'" + mType + "' '" + mName + "' '" + mValue + "'";
	for(tConnections::const_iterator it = mConnections.begin(); it != mConnections.end(); ++it)
	{
		out += " " + *it;
	}

	return out;
}

std::string ListComponent::GetSpecialToken(const std::string& name) const
{
	size_t start = mSpecial.find(name);
	if (start == std::string::npos || (start + name.length()) >= mSpecial.length() ) return "";
    size_t end = mSpecial.find_first_of(" ", start);
	size_t len = 0;
	if (end == std::string::npos)
		len = mSpecial.length() - start - name.length();
	else
		len = end - start - name.length();
	if (len <= 0) return "";
	return std::string(mSpecial, start + name.length(), len);
}