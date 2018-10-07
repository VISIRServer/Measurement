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

#pragma once
#ifndef __SYSTEM_LOG_H__
#define __SYSTEM_LOG_H__

#include <time.h>
#include <iostream>
#include <fstream>

#ifdef _WIN32 
#pragma warning( disable : 4996 )
#endif

/// Multiplexing output stream, with many endpoints
/// Used to redirect to both standard out and log
class MultiOStream
{
public:
	MultiOStream() : Os1(0) , Os2(0) {}
	MultiOStream(std::ostream& os1) : Os1(&os1) , Os2(0) {}
	MultiOStream(std::ostream& os1,std::ostream& os2) : Os1(&os1), Os2(&os2) {}

	void SetStreams(std::ostream& os1)
	{
		Os1 = &os1;
		Os2 = 0;
	}

	void SetStreams(std::ostream& os1, std::ostream& os2)
	{
		Os1 = &os1;
		Os2 = &os2;
	}

	MultiOStream& operator<<(std::ostream& (*_Pfn)(std::ostream&))
	{
		if (Os1) *Os1 << _Pfn;
		if (Os2) *Os2 << _Pfn;
		return *this;
	}
public:
	std::ostream* Os1,*Os2;
};

/// Output template operator for MultiOStream
template< class T >
MultiOStream& operator<< (MultiOStream& t,T thing)
{
	if (t.Os1) *t.Os1 << thing;
	if (t.Os2) *t.Os2 << thing; 
	return t;
}

std::string GenerateTimestamp();

std::string DirSeparator();

// helpers
inline std::ostream& timestamp(std::ostream& in)
{
	in << "[" << GenerateTimestamp().c_str() << "] ";
	return in;
}

/// Initialize logging
void InitLogging(bool enablelog, const char* dir = 0);

/// Set log level
void SetLogLevel(int level);
int GetLogLevel();

/// Log only if loglevel is highat or equal to argument
std::ostream& LogLevel(std::ostream& ostream, int level);

MultiOStream& LogLevel(MultiOStream& ostream, int level);

#define LOG_WHERE __FILE__ << ":" << __LINE__

extern std::fstream	dlog;
extern std::fstream	foutlog;
extern std::fstream	ferrlog;

extern std::fstream	timerlog;

extern MultiOStream	sysout;
extern MultiOStream	syserr;

extern MultiOStream	syslog;

#endif
