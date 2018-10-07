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

#include "dbmysqlmodule.h"
#include "dbmysql.h"
#include "dblog.h"

#include <util/config.h>

#include <iostream>

#include <iostream>
#include <fstream>

using namespace DBMysql;
using namespace std;

fstream		fdblog;

DBMySQLModule::DBMySQLModule()
{
	mpDatabase = new MysqlDatabase();
}

DBMySQLModule::~DBMySQLModule()
{
	delete mpDatabase;
}

int DBMySQLModule::RegisterModule(ModuleServices* pServices)
{
	dblog.Out() << "Registering DBMySQL Module" << endl;
	cout << "Register DBMySQL module" << endl;
	mpServices = pServices;

	mpDatabase->SetService(mpServices);
	mpServices->RegisterAuthenticator(mpDatabase);

	SetupLogging(pServices);

	return 1;
}

int DBMySQLModule::UnregisterModule()
{
	mpServices->UnregisterAuthenticator(mpDatabase);

	cout << "Unregister DBMySQL module" << endl;
	mpServices = NULL;
	return 1;
}

void DBMySQLModule::SetupLogging(ModuleServices* pServices)
{
	Config* cfg = pServices->GetConfig();
	int loglevel = 0;
	int logging = cfg->GetInt("Log", 1);
	if (logging > 0) loglevel = cfg->GetInt("LogLevel", 1);

	string logdir = cfg->GetString("LogDir", "logs");

	if (loglevel > 0)
	{
		fdblog.open((logdir + DirSeparator() + "dbmysql.log").c_str(), ios_base::binary | ios_base::out | ios_base::app);
		if (fdblog.is_open()) dblog.AddFileStream(&fdblog);
		dblog.AddScreenStream(&cout);
		dblog.AddErrorStream(&cerr);
		dblog.SetLogLevel(loglevel);
	}
}
