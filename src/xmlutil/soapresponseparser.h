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

#ifndef __SOAP_RESPONSE_PARSER_H__
#define __SOAP_RESPONSE_PARSER_H__

#include "xmlparser.h"

#include "soapresponse.h"

#include <string>

namespace XMLUtil
{

class ParameterParser : public XMLElementParser
{
public:
	void	SetParam(SoapParameter* out) { mpParameter = out; }

	virtual XMLElementParser* StartElement(const char *name, const char **attr);
	virtual void CharacterData(const char *s, int len);

	ParameterParser()
	{
		mpParameterParser = NULL;
	}

	~ParameterParser()
	{
		if (mpParameterParser) delete mpParameterParser;
	}
private:
	SoapParameter*		mpParameter;
	ParameterParser*	mpParameterParser;
};

class MethodParser : public XMLElementParser
{
public:
	void SetMethod(SoapMethod* out) { mpMethod = out; }

	virtual XMLElementParser* StartElement(const char *name, const char **attr);
private:
	ParameterParser	mParameterParser;
	SoapMethod*		mpMethod;
};

class BodyParser : public XMLElementParser
{
public:
	void SetResponse(SoapResponse* out) { mpResponse = out; }

	virtual XMLElementParser* StartElement(const char *name, const char **attr);
private:
	MethodParser	mMethodParser;
	SoapResponse*	mpResponse;
};

class EnvelopeParser : public XMLElementParser
{
public:
	void SetResponse(SoapResponse* out) { mpResponse = out; }

	virtual XMLElementParser* StartElement(const char *name, const char **attr);
private:
	BodyParser		mBodyParser;
	SoapResponse*	mpResponse;
};

class SoapResponseParser : public XMLElementParser
{
public:
	int Parse(const std::string& in, SoapResponse* out);

	virtual XMLElementParser* StartElement(const char *name, const char **attr);
private:
	EnvelopeParser mEnvelopeParser;
	SoapResponse* mpResponse;
};

} // end of namespace

#endif