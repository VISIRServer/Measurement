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

#include "eqmodule.h"
#include "eqlog.h"
#include "control.h"

#include <config.h>

using namespace EqSrv;
using namespace std;

#include <iostream>
#include <fstream>

fstream		feqlog;

EQModule::EQModule()
{
	mpEQSrvControl = NULL;
}

EQModule::~EQModule()
{
}

int EQModule::RegisterModule(ModuleServices* pServices)
{
	mpServices = pServices;
	SetupLogging(pServices);
	//mpServices->RegisterTransactionHandler(mpTransactionHandler);
	eqlog.Out() << "Registering EQCOM Module" << endl;
	return 1;
}

int	EQModule::Init()
{
	mpEQSrvControl = new EquipmentServerControl(mpServices->GetMultiplexer(), mpServices->GetConfig(), mpServices);
	return mpEQSrvControl->Init();
}

int	EQModule::IsInitDone()
{
	return mpEQSrvControl->IsInitDone();
}

int	EQModule::HasInitFailed()
{
	return mpEQSrvControl->HasInitFailed();
}

int EQModule::UnregisterModule()
{
 	if (mpEQSrvControl) delete mpEQSrvControl;
	mpEQSrvControl = NULL;

	//mpServices->UnregisterTransactionHandler(mpTransactionHandler);

	//ivilog.Log(1) << "UnregisterModule DirectIVI" << endl; 

	mpServices = NULL;

	return 1;
}

void EQModule::SetupLogging(ModuleServices* pServices)
{
	Config* cfg = pServices->GetConfig();
	int loglevel = 0;
	int logging = cfg->GetInt("Log", 1);
	if (logging > 0) loglevel = cfg->GetInt("LogLevel", 1);

	string logdir = cfg->GetString("LogDir", "logs");

	if (loglevel > 0)
	{
		feqlog.open((logdir + DirSeparator() + "eq.log").c_str(), ios_base::binary | ios_base::out | ios_base::app);
		if (feqlog.is_open()) eqlog.AddFileStream(&feqlog);
		eqlog.AddScreenStream(&cout);
		eqlog.AddErrorStream(&cerr);
		eqlog.SetLogLevel(loglevel);
	}
}