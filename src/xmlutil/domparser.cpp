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
 * Copyright (c) 2009 Johan Zackrisson
 * All Rights Reserved.
 */

#include "domparser.h"

#include <util/basic_exception.h>

#include <string>

using namespace XMLUtil;

DOMNode::DOMNode()
{
	CharData = "";
}

DOMNode::~DOMNode()
{
	for(tChildren::iterator it = Children.begin(); it != Children.end(); it++)
	{
		delete *it;
	}
	Children.clear();
}

DOMNode::DOMNode(const DOMNode& other)
{
	Name = other.Name;
	Attribs = other.Attribs;
	CharData = other.CharData;

	for(tChildren::const_iterator it = other.Children.begin(); it != other.Children.end(); it++)
	{
		DOMNode* pNode = new DOMNode(**it);
		Children.push_back(pNode);
	}
}

DOMNode& DOMNode::operator=(const DOMNode& other)
{
	Name = other.Name;
	Attribs = other.Attribs;
	CharData = other.CharData;

	for(tChildren::const_iterator it = other.Children.begin(); it != other.Children.end(); it++)
	{
		DOMNode* pNode = new DOMNode(**it);
		Children.push_back(pNode);
	}

	return *this;
}

DOMNode* DOMNode::NewChild(const char* name, const char **attr)
{
	DOMNode* pNew = new DOMNode();
	pNew->Name = name;
	pNew->SetAttr(attr);
	Children.push_back(pNew);
	return pNew;
}

void DOMNode::SetAttr(const char** attr)
{
	const char** cur = attr;
	while( *cur != NULL)
	{
		Attribs.push_back(tAttrPair(cur[0], cur[1]));
		cur+=2;
	}
}

std::string	DOMNode::GetAttr(std::string attr, bool throws) const
{
	for(tAttribs::const_iterator it = Attribs.begin(); it != Attribs.end(); it++)
	{
		if (it->first == attr) return it->second;
	}
	//if (throws) throw NodeNotFoundException(string("Attrib not found: ") + attr);
	// use this kind of exception for now.. we don't have any safeguards for the new type
	if (throws)
		throw BasicException(std::string("Attrib not found: ") + attr);
	return "";
}

const DOMNode* DOMNode::GetChild(std::string child, bool throws) const
{
	for(tChildren::const_iterator it = Children.begin(); it != Children.end(); it++)
	{
		if ((*it)->Name == child) return *it;
	}
	//if (throws) throw NodeNotFoundException(string("Child not found: ") + child);
	// use this kind of exception for now.. we don't have any safeguards for the new type
	if (throws) throw BasicException(std::string("Child not found: ") + child);
	return NULL;
}

///

XMLElementParser* DOMParser::StartElement(const char *name, const char **attr)
{
	if (mpNestedParser == NULL) mpNestedParser = new DOMParser();
	mpNestedParser->SetNode(mpDOMNode->NewChild(name, attr));
	return mpNestedParser;
}

void DOMParser::CharacterData(const char *s, int len)
{
	mpDOMNode->CharData += std::string(s, len);
}

int DOMParser::Parse(const std::string& in, DOMNode* out)
{
	mpDOMNode = out;

	try
	{
		XMLParser parser;
		return parser.Parse(in, this);
	}
	catch(std::exception& /*e*/) // XXX: catch all is not that great
	{
		return false;
	}
}
