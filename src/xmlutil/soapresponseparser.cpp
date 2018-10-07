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

#include "soapresponseparser.h"

#include <string>

using namespace XMLUtil;

#define SOAP_ENVELOPE	"http://www.w3.org/2003/05/soap-envelope#Envelope"
#define SOAP_BODY		"http://www.w3.org/2003/05/soap-envelope#Body"

XMLElementParser* ParameterParser::StartElement(const char *name, const char **attr)
{
	if (mpParameterParser == NULL) mpParameterParser = new ParameterParser();
	mpParameterParser->SetParam(&mpParameter->GetParam(name));
	return mpParameterParser;
}

void ParameterParser::CharacterData(const char *s, int len)
{
	mpParameter->SetValue(std::string(s, len));
}

XMLElementParser* MethodParser::StartElement(const char *name, const char **attr)
{
	mParameterParser.SetParam(&mpMethod->GetParam(name));
	return &mParameterParser;
}

XMLElementParser* BodyParser::StartElement(const char *name, const char **attr)
{
	mpResponse->GetMethod().SetName(name);
	mMethodParser.SetMethod(&mpResponse->GetMethod());
	return &mMethodParser;
}

XMLElementParser* EnvelopeParser::StartElement(const char *name, const char **attr)
{
	if (std::string(SOAP_BODY) == name)
	{
		mBodyParser.SetResponse(mpResponse);
		return &mBodyParser;
	}
	return NULL;
}

XMLElementParser* SoapResponseParser::StartElement(const char *name, const char **attr)
{
	if (std::string(SOAP_ENVELOPE) == name)
	{
		mEnvelopeParser.SetResponse(mpResponse);
		return &mEnvelopeParser;
	}
	return NULL;
}

int SoapResponseParser::Parse(const std::string& in, SoapResponse* out)
{
	mpResponse = out;

	XMLParser parser;
	return parser.ParseNS(in, this);
}
