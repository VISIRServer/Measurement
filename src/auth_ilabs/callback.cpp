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

#include "callback.h"
#include "module.h"
#include "sendhttp.h"

#include <config.h>
#include <stringop.h>
#include <xmlutil/soapresponseparser.h>

#include <string>
#include <sstream>
#include <iostream>

using namespace Auth_ILabs;
using namespace std;

class XmlContainer
{
public:
	XmlContainer(const std::string& name) { mName = name; }
	XmlContainer(const std::string& name, const std::string& data) { mName = name; mLocalSer << data; }
	~XmlContainer() {}

	void AddProperty(const std::string& name, const std::string& prop, const std::string& value) {
		mLocalSer << "<" << name << " " << prop << "=\"" << value << "\"/>";
	}
	
	void AddAttribute(const std::string& name, const std::string& value) {
		mValues << " " << name << "=\"" << value << "\"";
	}

	void AddRawAttrib(const std::string& value) {
		mValues << " " << value;
	}

	void AddData(const std::string& data) { mLocalSer << data; }

	// outputs the container
	void Add(XmlContainer& cont) {
		cont.Write(mLocalSer);
	}

	void Write(std::ostream& out) {
		out << "<" << mName << mValues.str() << ">" << mLocalSer.str() << "</" << mName << ">";
	}

private:
	std::string mName;
	std::stringstream mLocalSer;
	std::stringstream mValues;
};

void XMLHeader(std::ostream& out)
{
	out << "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n";
}


Callback::Callback()
{
	mpServices = NULL;
}

Callback::~Callback()
{
}

void Callback::SetService(ModuleServices* pServices)
{
	mpServices = pServices;
}

int Callback::Init()
{
	mURL = mpServices->GetConfig()->GetString("iLabs.URL", "http://localhost/openlabsElectronics/service");

	return 1;
}

const char* soapNS = \
	//"xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\""
	//" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\""
	" xmlns:soap12=\"http://www.w3.org/2003/05/soap-envelope\""
;

// XXX: make this interface async, so this doesn't block everything else
int Callback::FetchAuthEntry(const ICredentials* pInCred, IAuthEntry* pOutAuthEntry, int cacheId)
{
	// the cookie should be on the format passkey:couponId:issuerGuid
	string cookie = pInCred->GetCookie();
	vector<string> tokens;
	Tokenize(cookie, tokens, ":");
	if (tokens.size() != 3)
	{
		// wrong number of arguments
		return 0;
	}

	XmlContainer soapCtnr("soap12:Envelope");
	soapCtnr.AddRawAttrib(soapNS);

	XmlContainer soapBody("soap12:Body");

	XmlContainer call("VerifyCoupon");
	call.AddAttribute("xmlns", "http://ilab.mit.edu/iLabs/Services");

	call.Add(XmlContainer("issuerGuid", tokens[0]));
	call.Add(XmlContainer("couponId", tokens[1]));
	call.Add(XmlContainer("passkey", tokens[2]));

	soapBody.Add(call);
	soapCtnr.Add(soapBody);

	HTTPRequest request;
	request.SetHeaders("Content-Type: text/xml; charset=utf-8\r\nSOAPAction: \"http://ilab.mit.edu/iLabs/Services/VerifyCoupon\"\r\n");

	stringstream data;
	XMLHeader(data);
	soapCtnr.Write(data);

	// XXX: this should be done asyncronous
	request.Post(mURL, data.str());
	//

	XMLUtil::SoapResponseParser parser;
	XMLUtil::SoapResponse soapy;

	try
	{
		parser.Parse(request.Response(), &soapy);
	} catch (std::exception e)
	{
		cout << e.what() << endl;
		return 0;
	}

	//cout << "The result method was: " << soapy.GetMethod().GetName() << endl;

	if (soapy.IsFault())
	{
		//cerr << "There was a fault!" << endl;
		//cerr << soapy.FaultSummary() << endl;
		return 0;
	}
	else
	{
		string result = soapy.GetMethod().GetParam("http://ilab.mit.edu/iLabs/Services#VerifyCouponResult").GetValue();
		if (result == "true" || result == "1") return 1;
	}

	return 0;
}

