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
 * Copyright (c) 2008 André van Schoubroeck
 * Copyright (c) 2008-2009 Johan Zackrisson
 * All Rights Reserved.
 */

#include "ivimodule.h"
#include "ivitransactions.h"
#include "ividrivers.h"

#include "experiment.h"
#include "log.h"

#include <config.h>
#include <util/basic_exception.h>
#include <util/syslog.h>
#include <iostream>

using namespace IVIControl;
using namespace std;

IVIModule::IVIModule()
{
	mpTransactionHandler = new IVITransactionHandler();
	mpDrivers = NULL;
	mpListParser = NULL;
}

IVIModule::~IVIModule()
{
	if (mpTransactionHandler) delete mpTransactionHandler;
	if (mpDrivers) delete mpDrivers;
	if (mpListParser) delete mpListParser;
}

int IVIModule::RegisterModule(ModuleServices* pServices)
{
	cout << "RegisterModule" << endl;
	mpServices = pServices;
	mpServices->RegisterTransactionHandler(mpTransactionHandler);
	return 1;
}

int	IVIModule::Init()
{
	int logging = mpServices->GetConfig()->GetInt("Log", 1);
	std::string logdir = mpServices->GetConfig()->GetString("LogDir", "logs");
	InitIviLog(logging, logdir.c_str());
	int loglevel = mpServices->GetConfig()->GetInt("LogLevel", 1);

	mpDrivers = new Drivers();

	if (mpDrivers->Init(mpServices->GetConfig()) == 0)
	{
		delete mpDrivers;
		mpDrivers = NULL;
		return 0;
	}

	std::string CompConfig = "conf/" + mpServices->GetConfig()->GetString("CompConfig", "components.list");		
	mpListParser = new ListParser(*mpServices->GetComponentDefinitions());
	if (!mpListParser->ParseFile(CompConfig))
	{
		return 0;
	}
	mpTransactionHandler->Init(mpDrivers, mpListParser, mpServices);

	return 1;
}

int IVIModule::UnregisterModule()
{
	mpServices->UnregisterTransactionHandler(mpTransactionHandler);

	ivilog.Log(1) << "UnregisterModule DirectIVI" << endl; 

	mpServices = NULL;

	if (mpDrivers) delete mpDrivers;
	mpDrivers = NULL;

	if (mpListParser) delete mpListParser;
	mpListParser = NULL;

	return 1;
}
