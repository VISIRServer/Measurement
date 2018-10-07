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

#include "xmltools.h"
#include <basic_exception.h>
#include <stringop.h>

using namespace xmlprotocol;
using namespace std;

string xmlprotocol::GetAttr(string name, const char** attr, bool doThrow)
{
	for(int i=0;attr[i]; i+=2)
	{
		if (attr[i] == name) return attr[i+1];
	}

	if (doThrow) throw BasicException(string("xmlprotocol::GetAttr could not find token: ") + name);
	else return "";
}

string xmlprotocol::GetAttrValueStr(const char** attr)
{
	return GetAttr("value", attr);
}

double xmlprotocol::GetAttrValueDbl(const char** attr)
{
	string tmp = GetAttr("value", attr);
	return ToDouble(tmp); // make this trow?
}

int xmlprotocol::GetAttrValueInt(const char** attr)
{
	string tmp = GetAttr("value", attr);
	return ToInt(tmp); // make this trow?
}
