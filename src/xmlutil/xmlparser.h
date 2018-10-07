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

#ifndef __XML_PARSER_H__
#define __XML_PARSER_H__

#include <string>
#include <stack>

namespace XMLUtil
{

class XMLElementParser
{
public:
	virtual XMLElementParser* StartElement(const char *name, const char **attr) { return NULL; }
	virtual void EndElement(const char *name) {}
	virtual void CharacterData(const char *s, int len) {}
	virtual void StartNS(const char *prefix, const char *uri) {}
	virtual void EndNS(const char *prefix) {}
	virtual ~XMLElementParser() {}
};

class XMLParserState
{
public:
	virtual ~XMLParserState() {}
	
	virtual void StartElement(const char *name, const char **attr)
	{
		XMLElementParser* pParser = Top();
		Push( (pParser != NULL) ? pParser->StartElement(name, attr) : NULL );
	}

	virtual void EndElement(const char *name)
	{
		XMLElementParser* pParser = Top();
		if (pParser) pParser->EndElement(name);
		mStack.pop();
	}

	virtual void CharacterData(const char *s, int len)
	{
		XMLElementParser* pParser = Top();
		if (pParser) pParser->CharacterData(s, len);
	}

	virtual void StartNS(const char *prefix, const char *uri)
	{
		XMLElementParser* pParser = Top();
		if (pParser) pParser->StartNS(prefix, uri);
	}

	virtual void EndNS(const char *prefix)
	{
		XMLElementParser* pParser = Top();
		if (pParser) pParser->EndNS(prefix);
	}

	XMLParserState(XMLElementParser* pRootParser)
	{
		Push(pRootParser);
	}

private:
	XMLElementParser*	Top() { return mStack.top(); }
	void	Push(XMLElementParser* pParser)
	{
		mStack.push(pParser);
	}

	typedef std::stack< XMLElementParser* > tParserStack;
	tParserStack mStack;
};

class XMLParser
{
public:
	int Parse(const std::string& in, XMLElementParser* rootparser);
	// may throw
	int ParseNS(const std::string& in, XMLElementParser* rootparser);

	XMLParser();
	virtual ~XMLParser();
};

} // end of namespace

#endif