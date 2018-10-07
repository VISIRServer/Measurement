#pragma once
#ifndef __SERVER_MAIN_H__
#define __SERVER_MAIN_H__

class Config;
class SessionRegistry;
class Authentication;
class TransactionControl;
class Service;
class ClientManager;
class SystemTransactionHandler;
class RequestQueue;
class ServerProtocolService;
class ModuleRegistry;
class HTTPServer;
class XMLServer;
class SCGIServer;

namespace Net {
	class Multiplexer;
}

class ServerMain
{
public:
	ServerMain();
	virtual ~ServerMain();

	int StartUp(int argc, char** argv);
	int Tick(int timeout = 1000);
	int Shutdown();

	int NumHandledRequests();
	int TotalNumClients();
	int NumActiveSessions();
	int NumCurClients();
private:
	enum eState
	{
		eInit,
		eRunning
	} mState;

	bool InitLog();

	int StartServers();

	Config* mpConfig;
	SessionRegistry* mpSessionRegistry;
	Authentication* mpAuthentication;
	TransactionControl* mpTransactionControl;
	Net::Multiplexer* mpMultiplexer;
	Service* mpService;
	ClientManager* mpClientManager;
	SystemTransactionHandler* mpSystemTransactionHandler;
	RequestQueue* mpRequestQueue;
	ServerProtocolService* mpServerProtocolService;
	ModuleRegistry* mpModuleRegistry;
	HTTPServer* mpHTTPServer;
	XMLServer* mpXMLServer;
	XMLServer* mpXMLPolicyServer;
	SCGIServer* mpSCGIServer;
};

#endif
