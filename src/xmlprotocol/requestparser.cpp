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

#include "requestparser.h"
#include "xmlversions.h"
#include "xmlcommands.h"

#include <xmlutil/domparser.h>
#include <basic_exception.h>

#include <protocol/auth.h>
#include <protocol/basic_types.h>

#include <stringop.h>

#include <syslog.h>

#include <iostream>
using namespace xmlprotocol;
using namespace std;
using namespace XMLUtil;

bool RequestParser::ParsePacket(const char* pData, size_t length, tTransactions& outTransactions)
{
	try
	{
		XMLUtil::DOMParser	parser;
		XMLUtil::DOMNode	rootnode;
		if (!parser.Parse(string(pData, length), &rootnode)) return false;

		return ParseTree(&rootnode, outTransactions);
	}
	catch(XMLUtil::NodeNotFoundException e)
	{
		throw BasicException("XML Missing expected tags");
	}

	return true;
}

bool RequestParser::ParseTree(const XMLUtil::DOMNode* pNode, tTransactions& outTransactions)
{
	const DOMNode::tChildren& rootChildren = pNode->GetChildren();
	if (rootChildren.size() == 0) throw BasicException("No children in root node");
	if (rootChildren.size() > 1) throw BasicException("More than one root node");

	string firstname = rootChildren.front()->GetName();
	if (firstname == "protocol")
	{
		return ParseProtocolNode( pNode->GetChild("protocol", true), outTransactions);
	}
	else if(firstname == "policy-file-request")
	{
		protocol::Transaction* pTransaction = new protocol::Transaction();
		pTransaction->AddRequest( new protocol::DomainPolicyRequest() );
		outTransactions.push_back(pTransaction);
		return true;
	}
	else
	{
		throw BasicException("Unknown root node");
		return false;
	}

	return true;
}

bool RequestParser::ParseProtocolNode(const XMLUtil::DOMNode* pNode, tTransactions& outTransactions)
{
	double version = ToDouble(pNode->GetAttr("version"));
	if (version < XML_MIN_VERSION)
		throw BasicException(string("Protocol version lower than ") + ToString(XML_MIN_VERSION) + " is not supported");

	if (version > XML_MAX_VERSION)
		throw BasicException(string("Protocol version newer than ") + ToString(XML_MAX_VERSION) + " is not supported");

	const DOMNode::tChildren& protoch = pNode->GetChildren();
	if (protoch.size() == 0) throw BasicException("No children in protocol node");
	if (protoch.size() > 1) throw BasicException("More than one protocol node");

	const XMLUtil::DOMNode* pReqNode = protoch.front();
	string reqname = pReqNode->GetName();

	if (reqname == "request")
	{
		return ParseRequestNode(pReqNode, outTransactions);
	}
	else if (reqname == "login")
	{
		string cookie = pReqNode->GetAttr("cookie", false);
		string keepalivestr = pReqNode->GetAttr("keepalive", false);
		bool keepalive = false;

		if (keepalivestr == "1" || keepalivestr == "true") keepalive = true;

		protocol::Transaction* pTransaction = new protocol::Transaction();
		pTransaction->AddRequest( new protocol::AuthRequest(cookie, keepalive) );
		outTransactions.push_back(pTransaction);
		return true;
	}
	else if (reqname == "heartbeat")
	{
		protocol::Transaction* pTransaction = new protocol::Transaction();
		pTransaction->AddRequest( new protocol::HeartbeatRequest() );
		outTransactions.push_back(pTransaction);
		return true;
	}
	else
	{
		throw BasicException("Unknown request type in protocol node");
	}

	// not reached
	return false;
}

bool RequestParser::ParseRequestNode( const XMLUtil::DOMNode* pNode, tTransactions& outTransactions)
{
	string sessionKey = pNode->GetAttr("sessionkey", false);
	protocol::MeasureRequest* pMeasure = new protocol::MeasureRequest(sessionKey);

	const DOMNode::tChildren& children = pNode->GetChildren();
	DOMNode::tChildren::const_iterator it = children.begin();
	while(it != children.end())
	{
		const string& name = (*it)->GetName();

		XmlInstrumentCommand* pCommand = XmlInstrumentCommandFactory::CreateFromDom(*it);

		if (pCommand)
		{
			pCommand->SetData(*it);
			pMeasure->AddInstrumentCommand(pCommand);
		}
		else
		{
			delete pMeasure;
			throw BasicException(string("unable to handle instrument request type: ") + name );
		}
		it++;
	}

	protocol::Transaction* pTransaction = new protocol::Transaction();
	pTransaction->AddRequest( pMeasure );
	outTransactions.push_back(pTransaction);
	return true;
}
