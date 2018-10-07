#include "servermain.h"

#include "service.h"
#include "requestqueue.h"
#include "clientmanager.h"
#include "authentication.h"
#include "session.h"
#include "version.h"
#include "moduleregistry.h"

#include "transactioncontrol.h"
#include "systemtransactions.h"

#include "protocolservice.h"

#include <network/socket.h>
#include <network/multiplexer.h>

#include <config.h>
#include <syslog.h>

#include <httpserver/httpserver.h>
#include <xmlserver/xmlserver.h>
#include <scgiserver/scgiserver.h>

using namespace std;

#define SAFE_DELETE(x) {delete x; x = NULL;}

ServerMain::ServerMain()
{
	mpConfig = NULL;
	mpSessionRegistry = NULL;
	
	mpAuthentication = NULL;
	mpTransactionControl = NULL;
	mpMultiplexer = NULL;
	mpService = NULL;
	mpClientManager = NULL;
	mpSystemTransactionHandler = NULL;
	mpRequestQueue = NULL;
	mpServerProtocolService = NULL;
	mpModuleRegistry = NULL;

	mpHTTPServer = NULL;
	mpXMLServer = NULL;
	mpXMLPolicyServer = NULL;
	mpSCGIServer = NULL;

	mState = eInit;
}

ServerMain::~ServerMain()
{
	SAFE_DELETE(mpHTTPServer)
	SAFE_DELETE(mpXMLServer)
	SAFE_DELETE(mpXMLPolicyServer)
	SAFE_DELETE(mpSCGIServer)

	SAFE_DELETE(mpModuleRegistry)
	SAFE_DELETE(mpService)

	SAFE_DELETE(mpServerProtocolService)
	SAFE_DELETE(mpRequestQueue)
	SAFE_DELETE(mpSystemTransactionHandler)

	SAFE_DELETE(mpClientManager)
	SAFE_DELETE(mpAuthentication)

	SAFE_DELETE(mpSessionRegistry)
		
	SAFE_DELETE(mpTransactionControl)
	SAFE_DELETE(mpMultiplexer)

	SAFE_DELETE(mpConfig)
}

bool ServerMain::InitLog()
{
	int logging = mpConfig->GetInt("Log", 1);
	string logdir = mpConfig->GetString("LogDir", "logs");
	InitLogging(logging, logdir.c_str());

	int loglevel = mpConfig->GetInt("LogLevel", 1);
	SetLogLevel(loglevel);

	return true;
}

int ServerMain::StartUp(int argc, char** argv)
{
	srand((unsigned int)time(0)); // Genera un num aleatorio (no es real) al pasarle como semilla el mismo numero, siempre genera el mimo num
								  // por eso se le pasa la hora en formato int como semilla para que la semilla vaya variando y por consiguiente el num aleatorio
	Net::Socket::Init(); // inicializamos socket

	mpConfig = new Config(); // creamos objeto clase mpConfig, este ultimo es una clase que maneja un dict con pares clave-valor con parametros de conf

	// should we have command line parsing?
	if (argc > 1) mpConfig->ParseParams(argc-1, &argv[1]); // chequeamos si le han pasado parametros por consola y si es asi los parseamos

	// maybe config file should be configurable
	string confFile = "conf/measureserver.conf"; // este es el archivo que contiene todas las configuraciones del sistema
	if (!mpConfig->ParseFile(confFile)) // Leemos y parseamos el archivo de config, si es correcto almacenamos par clave valor de lo contrario mensag error
	{
		// we can't use syserr before initlogging
		cerr << "*** Failed to read config file! (" << confFile << ")" << endl;
		return 0;
	}

	// init logging
	InitLog(); // Inicializamos e logging

	sysout << timestamp << "*** Starting server, version " << VersionString() << " " << BUILDTYPE " ***" << endl; // Indicamos hora y que se inializo server y la version

	size_t maxSessions = mpConfig->GetInt("MaxSessions", 50); // del dict de conf leemos el maximo de sesiones, si no se carga valor por defecto 50
	double sessionTimeout = mpConfig->GetInt("SessionTimeout", 60*10); // Leemos el tiempo de tiemout del dict de conf si novalor defecto 600

	mpSessionRegistry = new SessionRegistry(maxSessions, sessionTimeout); // creamos un objeto de registro de sesiones, esta clase esta en sesion.h
	mpTransactionControl = new TransactionControl();

	int allowKeepAlive = mpConfig->GetInt("AllowKeepAlive", 1); // leemos la configuracion de permitir mantener se vivo, por defecto a 1, es necesario http en especial
	int bypassAuth = mpConfig->GetInt("BypassAuth", 0); // leemos la conf si hacemos by pass a la autentificacion, por defecto 0
	mpAuthentication = new Authentication(mpSessionRegistry, allowKeepAlive != 0, bypassAuth != 0); // creamos objeto autentificacion, le pasamos el objeto de registro de sesion true si permitimos keepalive y true si autentificacion

	mpMultiplexer = new Net::Multiplexer(); // Clase que multiplexa la conexion socket para permitir multiples clientes, esto, en nuestro caso lo hace directamente flask

	mpService = new Service(mpConfig);

	// init information service
	if (!mpService->Init()) // lee archivos de .max, politicas y definicion de componentes
	{
		syserr << "*** Failed to initialize services.." << endl;
		syserr << "*** Check your configuration files, databases and external equipment" << endl;
		return 0;
	}
	
	mpRequestQueue = new RequestQueue(); // creamos la clase que gestiona la cola de respuestas, igual flask ya despone de esto

	int maxClients = mpConfig->GetInt("MaxClients", 16); 
	mpClientManager = new ClientManager(maxClients);
	
	mpSystemTransactionHandler = new SystemTransactionHandler(mpAuthentication);
	mpTransactionControl->RegisterHandler(mpSystemTransactionHandler);
	
	double transactiontimeout = (double) mpConfig->GetInt("Timeout", 10);
	mpServerProtocolService = new ServerProtocolService(mpRequestQueue, mpTransactionControl, mpService, mpSessionRegistry, transactiontimeout);

	// register the modules as late as possible
	mpModuleRegistry = new ModuleRegistry(mpMultiplexer, mpAuthentication, mpConfig, mpTransactionControl, mpService);
	if (!mpModuleRegistry->LoadModules())
	{
		syserr << "*** Failed to load modules" << endl;
		return 0;
	}

	if (!mpModuleRegistry->InitModules())
	{
		syserr << "*** Failed to initialize modules" << endl;
		return 0;
	}

	/*while(!mpModuleRegistry->IsInitDone()) {}

	if (mpModuleRegistry->HasInitFailed())
	{
		syserr << "*** Failed to initialize modules" << endl;
		return 0;
	}

	// authentication must be done after module registration
	if (!mpAuthentication->Init())
	{
		syserr << "*** Failed to initialize authentication module" << endl;
		return 0;
	}*/

	// start the server!
	//return StartServers();
	return 1;
}

int ServerMain::StartServers()
{
	sysout << "[+] Initialization complete, staring to listen for incoming connections" << endl;

	mpHTTPServer = new HTTPServer(mpMultiplexer, mpServerProtocolService, mpClientManager);
	int httpport = mpConfig->GetInt("HTTPPort", 0);
	if (httpport != 0)
	{
		if (!mpHTTPServer->Init(httpport, mpConfig))
		{
			syserr << "HTTP Server failed to start" << endl;
			return 0;
		}
		else
		{
			sysout << "[+] Started HTTP server on port " << httpport << endl;
		}
	}

	int port = mpConfig->GetInt("Port", 0);
	mpXMLServer = new XMLServer(mpMultiplexer, mpServerProtocolService, mpClientManager);
	if (port != 0)
	{
		if (!mpXMLServer->Init(port, mpConfig))
		{
			syserr << "XML Server failed to start" << endl;
			return 0;
		}
		else
		{
			sysout << "[+] Started XML server on port " << port << endl;
		}
	}

	int scgiport = mpConfig->GetInt("SCGIPort", 0);
	if (scgiport != 0)
	{
		mpSCGIServer = new SCGIServer(mpMultiplexer, mpServerProtocolService, mpClientManager);
		if (!mpSCGIServer->Init(scgiport, mpConfig))
		{
			syserr << "SCGI Server failed to start" << endl;
			return 0;
		}
		else
		{
			sysout << "[+] Started SCGI server on port " << scgiport << endl;
		}
	}

	int noPolicyServer = mpConfig->GetInt("NoPolicyServer", 0);
	
	if (!noPolicyServer)
	{
		// just reuse the xml server for the policy file handling..
		mpXMLPolicyServer = new XMLServer(mpMultiplexer, mpServerProtocolService, mpClientManager);
		if (!mpXMLPolicyServer->Init(843, mpConfig))
		{
			syserr << "XML Policy Server failed to start" << endl;
			return 0;
		}
		else
		{
			sysout << "[+] Started XML policy server on port " << 843 << endl;
		}
	}

	return 1;
}

int ServerMain::Tick(int timeout)
{
	switch(mState)
	{
	case eInit:
		{
			if (mpModuleRegistry->IsInitDone())
			{
				if (mpModuleRegistry->HasInitFailed())
				{
					syserr << "*** Failed to initialize modules" << endl;
					return -1;
				}
				// authentication must be done after module registration
				if (!mpAuthentication->Init())
				{
					syserr << "*** Failed to initialize authentication module" << endl;
					return -1;
				}

				if (StartServers() == 0) return -1;				

				mState = eRunning;
			}
		}
		break;
	case eRunning:
		{
			if (!mpMultiplexer->WaitForEvent(timeout))
			{
				// we need recovery from failed select!
				cerr << "Server error.. shutting down.." << endl;
				return 0;
			}

			mpRequestQueue->ProcessQueue();

			// housekeeping
			mpMultiplexer->HouseKeeping();

			mpAuthentication->Tick();
			mpSessionRegistry->Tick();
		}
		break;
	}

	return 1;
}

int ServerMain::Shutdown()
{
	// shutdown the network services
	SAFE_DELETE(mpHTTPServer)
	SAFE_DELETE(mpXMLServer)
	SAFE_DELETE(mpXMLPolicyServer)

	if (mpModuleRegistry) mpModuleRegistry->UnloadModules();
	if (mpTransactionControl) mpTransactionControl->UnregisterHandler(mpSystemTransactionHandler);

	return 1;
}

int ServerMain::NumHandledRequests()
{
	if (mpRequestQueue) return mpRequestQueue->NumHandledRequests();
	return 0;
}

int ServerMain::TotalNumClients()
{
	if (mpClientManager) return mpClientManager->TotalNumClients();
	return 0;
}

int ServerMain::NumActiveSessions()
{
	if (mpSessionRegistry) return mpSessionRegistry->NumActiveSessions();
	return 0;
}

int ServerMain::NumCurClients()
{
	if (mpClientManager) return mpClientManager->NumCurClients();
	return 0;
}