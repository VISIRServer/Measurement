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

#include "xmlparser.h"
#include <util/basic_exception.h>

#include <expat.h>

#include <sstream>
#include <string>
using namespace XMLUtil;

static void XMLCALL startElement(void *userData, const char *name, const char **attr)
{
	XMLParserState* pState = (XMLParserState*) userData;
	pState->StartElement(name, attr);
}

static void XMLCALL endElement(void *userData, const char *name)
{
	XMLParserState* pState = (XMLParserState*) userData;
	pState->EndElement(name);
}

static void XMLCALL characterData(void *userData, const char *s, int len)
{
	XMLParserState* pState = (XMLParserState*) userData;
	pState->CharacterData(s, len);
}

static void XMLCALL startNS(void *userData, const XML_Char *prefix, const XML_Char *uri)
{
	XMLParserState* pState = (XMLParserState*) userData;
	pState->StartNS(prefix, uri);
}

static void XMLCALL endNS(void *userData, const XML_Char *prefix)
{
	XMLParserState* pState = (XMLParserState*) userData;
	pState->EndNS(prefix);
}

XMLParser::XMLParser()
{
}

XMLParser::~XMLParser()
{
}

int XMLParser::Parse(const std::string& in, XMLElementParser* rootparser)
{
	XML_Parser parser = XML_ParserCreate(NULL);
	//XML_Parser parser = XML_ParserCreateNS("UTF-8", '#');
	
	if (parser == NULL) throw BasicException("unable to create parser");

	XMLParserState state(rootparser);

	XML_SetUserData(parser, &state);

	XML_SetElementHandler(parser, startElement, endElement);
	XML_SetCharacterDataHandler(parser, characterData);
	XML_SetNamespaceDeclHandler(parser, startNS, endNS);
	XML_SetStartNamespaceDeclHandler(parser, startNS);
	XML_SetEndNamespaceDeclHandler(parser, endNS);

	try
	{
		if (XML_Parse(parser, in.c_str(), in.size(), true) == XML_STATUS_ERROR)
		{
			std::stringstream sstream;
			sstream << "XML Parse error at line " << 
				XML_GetCurrentLineNumber(parser) << " " << 
				XML_ErrorString(XML_GetErrorCode(parser)) << std::endl;

			XML_ParserFree(parser);
			parser = NULL;
			throw BasicException(sstream.str().c_str());
		}
		else
		{
			XML_ParserFree(parser);
			parser = NULL;
		}
	}
	catch(std::exception& /*e*/)
	{
		if (parser) XML_ParserFree(parser);
		parser = NULL;
		throw;
	}

	return 1;
}

int XMLParser::ParseNS(const std::string& in, XMLElementParser* rootparser)
{
	//XML_Parser parser = XML_ParserCreate(NULL);
	XML_Parser parser = XML_ParserCreateNS("UTF-8", '#');
	
	if (parser == NULL) throw BasicException("unable to create parser");

	XMLParserState state(rootparser);

	XML_SetUserData(parser, &state);

	XML_SetElementHandler(parser, startElement, endElement);
	XML_SetCharacterDataHandler(parser, characterData);
	XML_SetNamespaceDeclHandler(parser, startNS, endNS);
	XML_SetStartNamespaceDeclHandler(parser, startNS);
	XML_SetEndNamespaceDeclHandler(parser, endNS);

	try
	{
		if (XML_Parse(parser, in.c_str(), in.size(), true) == XML_STATUS_ERROR)
		{
			std::stringstream sstream;
			sstream << "XML Parse error at line " << 
				XML_GetCurrentLineNumber(parser) << " " << 
				XML_ErrorString(XML_GetErrorCode(parser)) << std::endl;

			XML_ParserFree(parser);
			parser = NULL;
			throw BasicException(sstream.str().c_str());
		}
		else
		{
			XML_ParserFree(parser);
			parser = NULL;
		}
	}
	catch(std::exception& /*e*/)
	{
		if (parser) XML_ParserFree(parser);
		parser = NULL;
		throw;
	}

	return 1;
}