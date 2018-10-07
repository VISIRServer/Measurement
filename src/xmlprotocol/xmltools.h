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
 * Copyright (c) 2007-2009 Johan Zackrisson
 * All Rights Reserved.
 */

#ifndef __XML_TOOLS_H__
#define __XML_TOOLS_H__

#include <string>
#include <ostream>
#include <sstream>

namespace xmlprotocol
{

class XmlContainer
{
public:
	XmlContainer(const std::string& name) { mName = name; }
	~XmlContainer() {}

	void AddProperty(const std::string& name, const std::string& prop, const std::string& value) {
		mLocalSer << "<" << name << " " << prop << "=\"" << value << "\"/>\n";
	}

	void AddProperty(const std::string& name, const std::string& prop, int value) {
		mLocalSer << "<" << name << " " << prop << "=\"" << value << "\"/>\n";
	}

	void AddProperty(const std::string& name, const std::string& prop, double value) {
		mLocalSer << "<" << name << " " << prop << "=\"" << std::scientific << value << "\"/>\n";
	}

	void AddValue(const std::string& name, int value) {
		mValues << " " << name << "=\"" << value << "\"";
	}
	
	void AddValue(const std::string& name, const std::string& value) {
		mValues << " " << name << "=\"" << value << "\"";
	}

	void AddData(const std::string& data) { mLocalSer << data; }

	// outputs the container
	void AddContainer(XmlContainer& cont) {
		cont.Write(mLocalSer);
	}

	void Write(std::ostream& out) {
		out << "<" << mName << mValues.str() << ">\n" << mLocalSer.str() << "</" << mName << ">\n";
	}

private:
	std::string mName;
	std::stringstream mLocalSer;
	std::stringstream mValues;
};

std::string GetAttr(std::string name, const char** attr, bool doThrow = true);
std::string GetAttrValueStr(const char** attr);
double GetAttrValueDbl(const char** attr);
int GetAttrValueInt(const char** attr);

} // end of namespace

#endif
