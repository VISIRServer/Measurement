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

#include <measureserver/servermain.h>

#include <iostream>
#include <sstream>
#include <fstream>

#include <sys/param.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>



//#include <conio.h>
using namespace std;

//#define CRASH_HARNESS 1

#ifdef CRASH_HARNESS
#include <contrib/crashinfo.h>
#define ENTRY_POINT harnessed_main
#else
#define ENTRY_POINT main
#endif


/*#include <measureserver/server.h>
#include <measureserver/service.h>
#include <measureserver/requestqueue.h>
#include <measureserver/clientmanager.h>
#include <measureserver/authentication.h>
#include <measureserver/session.h>
#include <measureserver/version.h>
#include <measureserver/moduleregistry.h>

#include <measureserver/transactioncontrol.h>
#include <measureserver/systemtransactions.h>

#include <measureserver/protocolservice.h>

#include <network/socket.h>

#include <serializer.h>
#include <conio.h>
#include <config.h>
#include <syslog.h>

#include <httpserver/httpserver.h>
#include <xmlserver/xmlserver.h>
using namespace std;
*/



/*bool InitLog(Config& aConfig)
{
	int logging = aConfig.GetInt("Log", 1);
	string logdir = aConfig.GetString("LogDir", "logs");
	InitLogging(logging, logdir.c_str());

	int loglevel = aConfig.GetInt("LogLevel", 1);
	SetLogLevel(loglevel);

	return true;
}*/

bool isRunning = true;

void catch_sigint(int sig)
{
	isRunning = false;
}

int DropPrivilege()
{
	struct passwd* pw;
	if ((pw = getpwnam("nobody")) == NULL) {
		cerr << "Failed to drop privileges: no such user: nobody" << endl;
		return 0;
	}

	if (setgroups(1, &pw->pw_gid)
		|| setegid(pw->pw_gid)
		|| setgid(pw->pw_gid)
		|| seteuid(pw->pw_uid)
		|| setuid(pw->pw_uid)) {
		cerr << "can't drop privileges" << endl;
		return 0;
	}

	return 1;
}

int ENTRY_POINT(int argc, char** argv)
{
	ServerMain aServerMain;

	if (aServerMain.StartUp(argc-1, &argv[1]) <= 0)
	{
		cerr << "Failed to start the server" << endl;
		return 0;
	}
	
	signal(SIGINT, catch_sigint);

	static int sCounter = 0;
	while( isRunning)
	{
		if (aServerMain.Tick(1000) <= 0)
		{
			cerr << "Server error.. shutting down.." << endl;
			aServerMain.Shutdown();
			return 0;
		}

		stringstream stats;
		//Serializer stats;
		stats	<< "[" << (int)aServerMain.NumHandledRequests()
				<< "/" << (int)aServerMain.TotalNumClients()
				<< "/" << (int)aServerMain.NumActiveSessions()
				<< "/" << (int)aServerMain.NumCurClients() << "]";

		cout << "\r" << stats.str();

		switch(sCounter % 4) // rotating marker :)
		{
		case 0: cout << " |\r"; break;
		case 1: cout << " /\r"; break;
		case 2: cout << " -\r"; break;
		case 3: cout << " \\\r";break;
		}
		
		flush(cout);
		sCounter++;

		if (1); //kbhit())
		{
			/*char c = getch();
			if (c == 'q')
			{
				isRunning = false;
			}*/
			/*else if (c == 'c')
			{
				throw 1;
			}*/
		}
	}

	aServerMain.Shutdown();

	return 1;
}

#ifdef CRASH_HARNESS

void save_buffer(const std::string & buffer) {
	std::ofstream ofs("crashlog.txt");
	if (ofs) ofs << buffer;
}

int seh_helper(int argc, char ** argv, std::string & buffer) {
	__try {
		return ENTRY_POINT(argc, argv);
	} __except (seh_filter(GetExceptionInformation(), buffer)) {
		if (!buffer.empty()) {
			save_buffer(buffer);
			std::cerr << "Abnormal Termination:" << endl << buffer << endl;
		}
		return -1;
	}
} 

int main(int argc, char ** argv) {
	std::string buffer;
	return seh_helper(argc, argv, buffer);
}

#endif

#if 0

	srand((unsigned int)time(0));
	Net::Socket::Init();

	Config aConfig;

	if (argc > 1) aConfig.ParseParams(argc-1, &argv[1]);

	// maybe config file should be configurable
	string confFile = "conf/measureserver.conf";
	if (!aConfig.ParseFile(confFile))
	{
		// we can't use syserr before initlogging
		cerr << "*** Failed to read config file! (" << confFile << ")" << endl;
		return 0;
	}

	// init logging
	InitLog(aConfig);

	sysout << timestamp << "*** Starting server, version " << VersionString() << " " << BUILDTYPE " ***" << endl;

	size_t maxSessions = aConfig.GetInt("MaxSessions", 50);
	double sessionTimeout = aConfig.GetInt("SessionTimeout", 60*10);
	SessionRegistry aSessionRegistry(maxSessions, sessionTimeout);

	TransactionControl aTransactionControl;

	int allowKeepAlive = aConfig.GetInt("AllowKeepAlive", 1);
	int bypassAuth = aConfig.GetInt("BypassAuth", 0);
	Authentication aAuthentication(&aSessionRegistry, allowKeepAlive != 0, bypassAuth != 0);

	Server aServer;

	Service	aService(&aConfig, &aServer, &aTransactionControl);

	// init information service
	if (!aService.Init())
	{
		syserr << "*** Failed to initialize services.." << endl;
		syserr << "*** Check your configuration files, databases and external equipment" << endl;
		return 0;
	}
	
	int maxClients = aConfig.GetInt("MaxClients", 16);
	ClientManager aClientManager(&aSessionRegistry, maxClients);
	
	SystemTransactionHandler mSystemTransactionHandler(&aAuthentication);
	aTransactionControl.RegisterHandler(&mSystemTransactionHandler);

	RequestQueue aRequestQueue;
	ServerProtocolService aServerProtocolService(&aRequestQueue, &aTransactionControl, &aService);

	// register the modules as late as possible
	ModuleRegistry modules(&aServer, &aAuthentication, &aConfig, &aTransactionControl, &aService);
	if (!modules.LoadModules())
	{
		syserr << "*** Failed to load modules" << endl;
		return 0;
	}

	if (!modules.InitModules())
	{
		syserr << "*** Failed to initialize modules" << endl;
		return 0;
	}

	// authentication must be done after module registration
	if (!aAuthentication.Init())
	{
		syserr << "*** Failed to initialize authentication module" << endl;
		return 0;
	}

	// start the server!
	
	sysout << "[+] Initialization complete, staring to listen for incoming connections" << endl;

	{
		// make sure the servers are shut down before everything else

		HTTPServer httpserver(&aServer, &aServerProtocolService, &aClientManager);
		int httpport = aConfig.GetInt("HTTPPort", 0);
		if (httpport != 0)
		{
			if (!httpserver.Init(httpport))
			{
				syserr << "HTTP Server failed to start" << endl;
				return 0;
			}
			else
			{
				sysout << "[+] Started HTTP server on port " << httpport << endl;
			}
		}

		int port = aConfig.GetInt("Port", 0);
		XMLServer xmlserver(&aServer, &aServerProtocolService, &aClientManager);
		if (port != 0)
		{
			if (!xmlserver.Init(port))
			{
				syserr << "XML Server failed to start" << endl;
				return 0;
			}
			else
			{
				sysout << "[+] Started XML server on port " << port << endl;
			}
		}

		// just reuse the xml server for the policy file handling..
		XMLServer xmlpolicyserver(&aServer, &aServerProtocolService, &aClientManager);
		if (!xmlpolicyserver.Init(843))
		{
			syserr << "XML Policy Server failed to start" << endl;
			return 0;
		}
		else
		{
			sysout << "[+] Started XML policy server on port " << 843 << endl;
		}

		static int sCounter = 0;
		bool isRunning = true;

		while( isRunning)
		{
			if (!aServer.WaitForEvent(1000))
			{
				// we need recovery from failed select!
				cerr << "Server error.. shutting down.." << endl;
				isRunning = false;
				return -1;
			}

			aRequestQueue.ProcessQueue();

			// housekeeping
			aServer.HouseKeeping();

			aService.Tick();
			aAuthentication.Tick();
			aSessionRegistry.Tick();

			Serializer stats;
			stats	<< "[" << (int)aRequestQueue.NumHandledRequests()
					<< "/" << (int)aClientManager.TotalNumClients()
					<< "/" << (int)aSessionRegistry.NumActiveSessions()
					<< "/" << (int)aClientManager.NumCurClients() << "]";

			switch(sCounter % 4) // rotating marker :)
			{
			case 0: cout << "\r" << stats.GetCStream() << " |\r"; break;
			case 1: cout << "\r" << stats.GetCStream() << " /\r"; break;
			case 2: cout << "\r" << stats.GetCStream() << " -\r"; break;
			case 3: cout << "\r" << stats.GetCStream() << " \\\r";break;
			}
			sCounter++;

			if (kbhit())
			{
				char c = getch();
				if (c == 'q')
				{
					isRunning = false;
				}
			}
		}
	} // make sure the servers are shut down before everything else

	sysout << "Server shut down on request.. all is well" << endl;

	modules.UnloadModules();
	aTransactionControl.UnregisterHandler(&mSystemTransactionHandler);
	aService.Shutdown();
}

#endif
