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

#pragma once
#ifndef __SYSTEM_LOG_MODULE_H__
#define __SYSTEM_LOG_MODULE_H__

#include <ostream>
#include <list>
#include <string>

#include "syslog.h"
#define LOG_PREFIX timestamp << mName

// forward decls to get access to the global operator in the class
class LogOutput;
template< class T >
const LogOutput& operator<< (const LogOutput& logoutput, T out);

class LogOutput
{
public:
	typedef std::list< std::ostream* > tOutStreams;
	const tOutStreams& GetStreams() const { return mStreams; }
	void AddStream(std::ostream* stream) { mStreams.push_back(stream); }

	const LogOutput& operator<<(std::ostream& (*_Pfn)(std::ostream&)) const
	{
		::operator<<(*this, _Pfn);
		return *this;
	}
private:
	tOutStreams mStreams;
};

/// Output template operator
template< class T >
const LogOutput& operator<< (const LogOutput& logoutput, T out)
{
	LogOutput::tOutStreams streams = logoutput.GetStreams();
	for(LogOutput::tOutStreams::const_iterator it = streams.begin(); it != streams.end(); it++)
	{
		**it << out;
	}

	return logoutput;
}

class LogModule
{
public:
	LogModule(std::string name, int loglevel = 1);
	virtual ~LogModule();

	LogOutput& Log(int outLevel = 1)
	{
		LogOutput& out = mLogOutput;
		if (outLevel > mLogLevel) return sEmptyOut;
		out << LOG_PREFIX << "(" << outLevel << "): ";
		return out;
	}

	LogOutput& Out(int outLevel = 1)
	{
		LogOutput& out = mScreenOutput;
		if (outLevel > mLogLevel) return sEmptyOut;
		out << LOG_PREFIX << "(" << outLevel << "): ";
		return out;
	}

	LogOutput& Error()
	{
		mErrorOutput << LOG_PREFIX << " Error: ";
		return mErrorOutput;
	}

	void	SetLogLevel(int level) { mLogLevel = level; }
	int		GetLogLevel() { return mLogLevel; }
	
	void	AddFileStream(std::ostream* stream)
	{
		mLogOutput.AddStream(stream);
		mScreenOutput.AddStream(stream);
		mErrorOutput.AddStream(stream);
	}

	void AddScreenStream(std::ostream* stream)
	{
		mScreenOutput.AddStream(stream);
	}

	void AddErrorStream(std::ostream* stream)
	{
		mErrorOutput.AddStream(stream);
	}
private:
	static LogOutput sEmptyOut;
	LogOutput	mLogOutput;
	LogOutput	mScreenOutput;
	LogOutput	mErrorOutput;	
	int mLogLevel;
	std::string mName;
};


#endif
