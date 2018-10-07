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
 * Copyright (c) 2005-2009 Johan Zackrisson
 * All Rights Reserved.
 */

#include "syslog.h"
#include <string>

#if _WIN32
#include <windows.h>
#endif

std::fstream	dlog;
std::fstream	foutlog;
std::fstream	ferrlog;
std::fstream	timerlog;

MultiOStream	sysout;
MultiOStream	syserr;
MultiOStream	syslog;

MultiOStream	sNullMultiStream;
std::ostream	sNullOStream(0);

static int sLogLevel = 0;

#if _WIN32
#define DIRSEP "\\"
#else
#define DIRSEP "/"
#endif

void InitLogging(bool enablelog, const char* dir)
{
	sLogLevel = 0;
	if (enablelog)
	{
		std::string strDir;
		if (dir) strDir = dir;
		else strDir = "C:";

		std::ios_base::openmode mode = std::ios_base::binary | std::ios_base::out | std::ios_base::app;
		dlog.open(		(strDir + DIRSEP "distlab.dlg").c_str(), mode);
		foutlog.open(	(strDir + DIRSEP "distlab.log").c_str(), mode);
		ferrlog.open(	(strDir + DIRSEP "distlab.err").c_str(), mode);

		timerlog.open(	(strDir + DIRSEP "timer.log").c_str(), mode);
		timerlog.precision(10);

		// maybe we want to disable loggint to standard out?

		sysout.SetStreams(foutlog, std::cout);
		syserr.SetStreams(ferrlog, std::cerr);
		syslog.SetStreams(foutlog);

		dlog.precision(10);
		foutlog.precision(10);
		ferrlog.precision(10);
	}
}

void SetLogLevel(int level)
{
	sLogLevel = level;
}

int GetLogLevel()
{
	return sLogLevel;
}

std::ostream& LogLevel(std::ostream& ostream, int level)
{
	if (level <= sLogLevel)	return ostream;
	else return sNullOStream;
}

MultiOStream& LogLevel(MultiOStream& ostream, int level)
{
	if (level <= sLogLevel)	return ostream;
	else return sNullMultiStream;
}

std::string GenerateTimestamp()
{
#if _WIN32
	char buffer[64];
	SYSTEMTIME systime;
	GetLocalTime(&systime);
	sprintf(buffer, "%d-%02d-%02d %02d:%02d:%02d:%03d",
		systime.wYear,
		systime.wMonth,
		systime.wDay,
		systime.wHour,
		systime.wMinute,
		systime.wSecond,
		systime.wMilliseconds
		);
#else
	time_t now;
	struct tm* timeinfo;
	char buffer[80];
	time(&now);
	timeinfo = localtime(&now);
	strftime(buffer, 80, "%Y-%m-%d %H:%M:%S", timeinfo);
#endif

	return buffer;
}

std::string DirSeparator()
{
	return DIRSEP;
}
