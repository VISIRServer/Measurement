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

#ifndef __TYPES_H__
#define __TYPES_H__

/// \todo use boost tokenizer instead?
// XXX: use substr?

#include <string>
#include <stdlib.h>

#ifdef _WIN32 
#pragma warning( disable : 4996 )
#endif

inline std::string ToString(int x)
{
	char buffer[128];
	sprintf(buffer, "%i", x);
	return buffer;
}

inline std::string ToString(unsigned int x)
{
	char buffer[128];
	sprintf(buffer, "%u", x);
	return buffer;
}

inline std::string ToString(double x)
{
	char buffer[128];
	sprintf(buffer, "%f", x);
	return buffer;
}

inline std::string ToStringScientific(double x)
{
	char buffer[128];
	sprintf(buffer, "%e", x);
	return buffer;
}

inline double ToDouble(std::string instring)
{
	return atof(instring.c_str());
}

inline int	ToInt(std::string instring)
{
	return atoi(instring.c_str());
}

inline std::string ToStringbitfield(unsigned int val, int len)
{
	std::string tempstring;
	// assume MSB -> LSB
	unsigned int testbit = 1 << (len-1);
	for(int i=0;i<len;i++)
	{
		tempstring += (val & testbit) ? "1" : "0";
		testbit >>= 1;
	}

	return tempstring;
}

inline std::string Token(std::string instring, int tokennum, std::string tokenchars)
{
	// skip all empty chars in beginning
	size_t pos = 0;
	std::string outstring;

	for(int i=0;i<=tokennum;i++)
	{		
		// eat whitespaces and tabs
		if (i != 0) pos = instring.find_first_not_of(tokenchars, pos);
		if (pos == std::string::npos) return "";

		// get token!
		size_t endpos = instring.find_first_of(tokenchars, pos);
		// return the whole string.. but all tokens after should be ""
		if (std::string::npos == endpos) outstring = std::string(instring, pos);
		else outstring = std::string(instring, pos, endpos - pos);
		pos = endpos;
	}

	return outstring;
}

// standard tokenizer.. on tab and space
inline std::string Token(std::string instring, int tokennum)
{
	return Token(instring, tokennum, "\t ");
}

// removes everything from start to token tokennum
inline std::string RemoveToken(std::string instring, int tokennum, std::string tokenchars = "\t ")
{
	size_t pos = 0;
	std::string outstring;

	for(int i=0;i<=tokennum;i++)
	{
		// eat whitespaces and tabs
		pos = instring.find_first_not_of(tokenchars, pos);

		// get token!
		size_t endpos = instring.find_first_of(tokenchars, pos);
		// strip trailing tokenchars
		endpos = instring.find_first_not_of(tokenchars, endpos);
		if (endpos != std::string::npos)
		{
			outstring = std::string(instring, endpos);
		}
		else
		{
			outstring = "";
		}
		pos = endpos;
	}

	return outstring;
}

template<typename T>
void Tokenize(const std::string& inString, T& outContainer, std::string separators, bool removeDuplicateSeparators = false)
{
	if (inString.empty()) return;
	size_t curPos = 0;

	while(curPos != std::string::npos)
	{
		if (removeDuplicateSeparators) curPos = inString.find_first_not_of(separators, curPos);
		size_t endPos = inString.find_first_of(separators, curPos);
		if (endPos == std::string::npos)
		{
			if (curPos != std::string::npos)
			{
				outContainer.push_back(std::string(inString, curPos));
			}

			curPos = std::string::npos;
		}
		else
		{
			outContainer.push_back(std::string(inString, curPos, endPos - curPos));
			curPos = endPos+1;
		}
	}
}

inline std::string ToLower(std::string instring)
{
	std::string outstring;
	for(std::string::const_iterator i = instring.begin(); i != instring.end(); i++)
	{
		outstring += (char)tolower(*i);
	}

	return outstring;
}

inline std::string ToUpper(std::string instring)
{
	std::string outstring;
	for(std::string::const_iterator i = instring.begin(); i != instring.end(); i++)
	{
		outstring += (char)toupper(*i);
	}

	return outstring;
}

inline bool GetLine(std::string& instring, std::string& outstring)
{
	if (instring.empty()) return false;

	size_t pos = instring.find_first_of("\n\r");
	if (pos != std::string::npos)
	{
		outstring = std::string(instring, 0, pos);
		size_t endpos = instring.find_first_not_of("\n\r",pos);
		if (endpos == std::string::npos)
		{
			instring = "";
		}
		else
		{
			instring = std::string(instring, endpos);
		}
	}
	else
	{
		outstring = instring;
		instring = "";
	}

	return true;
}

inline std::string CleanWhitespaces(std::string instring)
{
	size_t first	= instring.find_first_not_of(" \t");
	size_t last		= instring.find_last_not_of(" \t");

	if (first	== std::string::npos) return "";
	if (last	== std::string::npos) return "";

	return std::string(instring, first, last - first + 1);
}

inline std::string ToCRLF(std::string instring)
{
	std::string outstring;
	for(std::string::iterator i=instring.begin(); i != instring.end(); i++)
	{
		if (*i == '\n')
		{
			outstring += "\r\n";
		}
		else outstring += *i;
	}

	return outstring;
}

inline std::string AddWhitespace(std::string instring, size_t length)
{
	std::string outstring = instring;
	while(outstring.size() < length) outstring += " ";
	return outstring;
}

#endif
