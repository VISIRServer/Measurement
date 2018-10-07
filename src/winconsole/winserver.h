#pragma once
#ifndef __WINSERVER_H__
#define __WINSERVER_H__

#include <string>

class ServerMain;

class WinServer
{
public:
	WinServer();
	virtual ~WinServer();

	int		Run();
	int		Shutdown();
	bool	IsRunning();

	void	FatalShutdown();

	int StartThreaded();

	std::string GetInfoText();

	
private:
	int Tick();
	int StartUp();	

	
	
	enum eState
	{
		eStart,
		eRun,
		eShutdown,
		eDead
	} mState;

	ServerMain* mpServerMain;
	HANDLE mpThread;
};

#endif
