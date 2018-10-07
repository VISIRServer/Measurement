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

#include "service.h"

#include <instruments/validate.h>
#include <instruments/compdefreader.h>

#include <config.h>
#include <basic_exception.h>

#include <syslog.h>

static const char* sDefaultPolicy =
"<?xml version=\"1.0\"?>"
"<!DOCTYPE cross-domain-policy SYSTEM \"http://www.macromedia.com/xml/dtds/cross-domain-policy.dtd\">"
"<cross-domain-policy>"
"<allow-access-from domain=\"*\" to-ports=\"*\"/>"
"</cross-domain-policy>";

Service::Service(Config* pConfig)
{
	mpConfig = pConfig; // copiamos el objeto que contiene el diccionacion con las configuraciones
	mpMaxLists = new MaxLists(); // creamos un objeto de maxlist
	mCompInfo = new ComponentDefinitionReader(); // creamos un objeto de definicion de componentes
}

Service::~Service()
{
	delete mpMaxLists;
	delete mCompInfo;
}

bool Service::Init()
{
	std::string confBaseDir		= mpConfig->GetString("ConfBaseDir", "conf/");
	std::string compTypeConfig	= mpConfig->GetString("CompTypes", "component.types");
	std::string maxListConfig	= mpConfig->GetString("MaxListConfig", "maxlists.conf");
	std::string saveCircuits	= mpConfig->GetString("SaveCircuits", "");

	if (!mCompInfo->ReadFile(confBaseDir + compTypeConfig))
	{
		syserr << "Failed to read component type definitions" << std::endl;
		return false;
	}

	if (!mpMaxLists->Init(confBaseDir, maxListConfig, saveCircuits, mCompInfo->GetDefinitions())) return false;

	std::string policyFile		= mpConfig->GetString("PolicyFile", "");
	if (policyFile == "")
	{
		mPolicyData = sDefaultPolicy;
	}
	else
	{
		std::string filename = confBaseDir + policyFile;
		std::fstream policyStream(filename.c_str());
		if (!policyStream.is_open())
		{
			syserr << "Failed to open policy file: " << filename << std::endl;
			return false;
		}

		std::string line;
		while(!policyStream.eof())
		{
			getline(policyStream, line);
			mPolicyData.append(line);			
		}
	}

	return true;
}

bool Service::ValidateMaxlists(const NetList2& componentlist)
{
	return mpMaxLists->IsSubsetsOfComponentlist(componentlist);
}

bool Service::TranslateCircuitAndValidate(InstrumentBlock* pBlock)
{
	// this will probably throw a lot of exceptions

	try
	{
		if (!mpMaxLists->CircuitToNetlist(pBlock))
		{
			//string msg = "The circuit is not allowed as it may be harmful for the laboratory setup";
			std::string msg = "The circuit cannot be constructed. Either it is unsafe or the current set of rules validating the circuit can't find a suitable solution.";
			syslog << msg << std::endl;
			throw ValidationException(msg);
			return false; // never reached
		}

		// validate instrumentblock before encoding
		if (!Validator::ValidateBlock(pBlock)) // may throw
		{
			std::string msg = "Invalid instrument settings";
			syslog << msg << std::endl;
			throw ValidationException(msg);
			return false;
		}

		// XXX: This check shouldn't be needed, as the CircuitToNetlist call above shouldn't match in that case
		if (!mpMaxLists->CheckAndValidate(pBlock))
		{
			std::string msg = "Circuit is not safe, either its not a subset of a maxlist or a instrument limit is exceeded";
			syslog << msg << std::endl;
			throw ValidationException(msg);
			return false; // never reached
		}
	}
	catch(ValidationException e)
	{
		syserr << "Service::TranslateCircuitAndValidate: Validation exception: " << e.what() << std::endl;
		throw;
	}
	catch(BasicException e)
	{
		syserr << "Service::TranslateCircuitAndValidate: BasicException: " << e.what() << std::endl;
		throw;
	}
	catch(...)
	{
		syserr << "FATAL ERROR: Service::TranslateCircuitAndValidate: Unknown exception" << std::endl;
		return false;
	}

	return true;
}

const ListParser::tComponentDefinitions& Service::GetComponentDefinitions() const
{
	return mCompInfo->GetDefinitions();
}