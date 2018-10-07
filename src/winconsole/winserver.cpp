#include "stdafx.h"
#include "winserver.h"

#include <measureserver/servermain.h>
#include <contrib/crashinfo.h>

#include <sstream>
#include <fstream>
#include <iostream>

#include <syslog.h>

using namespace std;

WinServer::WinServer()
{
	mState = eStart;
	mpThread = NULL;
	mpServerMain = new ServerMain();
}

WinServer::~WinServer()
{
	delete mpServerMain;
}

int WinServer::StartUp()
{
	int rv = mpServerMain->StartUp(0, NULL);
	mState = (rv > 0) ? eRun : eDead;
	return rv;
}

DWORD WINAPI realThreadFunc(LPVOID lpParam)
{
	WinServer* pServer = (WinServer*) lpParam;
	while(pServer->IsRunning())
	{
		int rv = pServer->Run();
		if (rv <= 0)
		{
			return rv;
		}
	}

	return 1;
}

void save_buffer(const std::string & buffer) {
	std::ofstream ofs("crashlog.txt");
	if (ofs) ofs << buffer;
	std::cerr << buffer;
	syserr << buffer;
}

int seh_helper(LPVOID lpParam, std::string & buffer)
{
	WinServer* pServer = (WinServer*) lpParam;
	__try {
		return realThreadFunc(lpParam);
	} __except (seh_filter(GetExceptionInformation(), buffer)) {
		if (!buffer.empty()) {
			save_buffer(buffer);
			pServer->FatalShutdown();
			MessageBoxA(0, buffer.c_str(), "Abnormal Termination", MB_OK);
		}
		return -1;
	}
} 

DWORD WINAPI ThreadFunc(LPVOID lpParam)
{
	//SymInit sym;
	std::string buffer;
	return seh_helper(lpParam, buffer);
}

int WinServer::StartThreaded()
{
	DWORD id = 0;
	mpThread = CreateThread(NULL, 0, ThreadFunc, this, 0, &id);

	return 1;
}

int WinServer::Run()
{
	switch(mState)
	{
	case eStart:
		return StartUp();
		break;
	case eRun:
		return Tick();
		break;
	case eShutdown:
		mState = eDead;
		mpServerMain->Shutdown();
		break;
	case eDead:
		break;
	}

	return 1;
}

int WinServer::Tick()
{
	int rv = mpServerMain->Tick(100);
	if (rv <= 0)
	{
		mState = eShutdown;
		return 1;
	}
	return rv;
}

int WinServer::Shutdown()
{
	mState = eDead;
	if (mpThread) WaitForSingleObject(mpThread, INFINITE);
	return mpServerMain->Shutdown();
}

bool WinServer::IsRunning()
{
	if (mState == eDead) return false;
	else return true;
}

std::string WinServer::GetInfoText()
{
	stringstream stats;
	stats	<< "Handled:" << (int)mpServerMain->NumHandledRequests()
		<< " Total clients: " << (int)mpServerMain->TotalNumClients()
		<< " Active: " << (int)mpServerMain->NumActiveSessions()
		<< " Current: " << (int)mpServerMain->NumCurClients();

	if (mState == eShutdown) stats << " - Server is shutting down";
	else if (mState == eDead) stats << " - Server not running";

	return stats.str();

}

void WinServer::FatalShutdown()
{
	mState = eDead;
}