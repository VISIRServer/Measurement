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
 * Copyright (c) 2008 Johan Zackrisson
 * All Rights Reserved.
 */

#include "gpib_module.h"
#include "gpib_service.h"
#include "gpib_transactions.h"
#include "gpib_log.h"

#include <config.h>

#include <iostream>
#include <fstream>

using namespace gpib;
using namespace std;

fstream		fgpiblog;

GPIBModule::GPIBModule()
{
	mpGPIBService = NULL;
}

GPIBModule::~GPIBModule()
{
	delete mpGPIBService;
}

int GPIBModule::RegisterModule(ModuleServices* pServices)
{
	SetupLogging(pServices);
	gpiblog.Out() << "RegisterModule" << endl;

	mpServices = pServices;
	mpGPIBService = new GPIBService(pServices->GetConfig());
	mpGPIBService->Init();
	mpServices->RegisterTransactionHandler(mpGPIBService->GetTransactionHandler());

	return 1;
}

int GPIBModule::UnregisterModule()
{
	mpServices->UnregisterTransactionHandler(mpGPIBService->GetTransactionHandler());

	gpiblog.Out() << "UnregisterModule" << endl;
	mpServices = NULL;
	return 1;
}

void GPIBModule::Tick()
{
	mpGPIBService->Tick();
}

void GPIBModule::SetupLogging(ModuleServices* pServices)
{
	Config* cfg = pServices->GetConfig();
	int loglevel = 0;
	int logging = cfg->GetInt("Log", 1);
	if (logging > 0) loglevel = cfg->GetInt("LogLevel", 1);

	string logdir = cfg->GetString("LogDir", "logs");

	if (loglevel > 0)
	{
		fgpiblog.open((logdir + DirSeparator() + "gpibcontrol.log").c_str(), ios_base::binary | ios_base::out | ios_base::app);
		if (fgpiblog.is_open()) gpiblog.AddFileStream(&fgpiblog);
		gpiblog.AddScreenStream(&cout);
		gpiblog.AddErrorStream(&cerr);
		gpiblog.SetLogLevel(loglevel);
	}
}