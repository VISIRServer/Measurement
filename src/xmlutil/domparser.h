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

#ifndef __DOM_PARSER_H__
#define __DOM_PARSER_H__

#include "xmlparser.h"

#include <string>
#include <vector>
#include <list>
#include <exception>

namespace XMLUtil
{

class NodeNotFoundException : public std::exception
{
public:
	NodeNotFoundException(std::string what) : mWhat(what) {}
	virtual ~NodeNotFoundException() throw() {}
	virtual const char* what() const throw() {
		return mWhat.c_str();
	}
	
private:
	std::string mWhat;
};

class DOMNode
{
public:
	typedef std::pair<std::string, std::string> tAttrPair;
	typedef std::list< tAttrPair > tAttribs;
	typedef std::list< DOMNode* > tChildren;

	const tChildren&	GetChildren() const		{ return Children; }
	const tAttribs&		GetAttributes() const	{ return Attribs; }
	const std::string&	GetName() const			{ return Name; }
	const std::string&	GetData() const			{ return CharData; }

	std::string			GetAttr(std::string attr, bool throws = false) const;
	const DOMNode*		GetChild(std::string child, bool throws = false) const;

	DOMNode*	NewChild(const char* name, const char **attr);
	void		SetAttr(const char** attr);

	DOMNode();
	~DOMNode();
	DOMNode(const DOMNode& other);
	DOMNode& operator=(const DOMNode& other);

	// Make private?
	std::string	Name;
	std::string CharData;
	tAttribs	Attribs;
	tChildren	Children;
};

class DOMParser : public XMLElementParser
{
public:
	void	SetNode(DOMNode* out) { mpDOMNode = out; }

	virtual XMLElementParser* StartElement(const char *name, const char **attr);
	virtual void CharacterData(const char *s, int len);

	int Parse(const std::string& in, DOMNode* out);

	DOMParser()
	{
		mpNestedParser = NULL;
	}

	~DOMParser()
	{
		if (mpNestedParser) delete mpNestedParser;
	}
private:
	DOMNode*			mpDOMNode;
	DOMParser*			mpNestedParser;
};

} // end of namespace

#endif