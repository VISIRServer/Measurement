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
 * Copyright (c) 2008-2009 Johan Zackrisson
 * All Rights Reserved.
 */

#include "header.h"
#include "symbols.h"

#include <serializer.h>

using namespace EqSrv;
using namespace std;

#ifdef _WIN32
#pragma warning( disable : 4996 )
#endif

string Header::ToLenString(size_t len, int outlen)
{
	char format[32];
	char buffer[32];
	sprintf(format,"%%.%ii",outlen);
	sprintf(buffer, format, len);
	return buffer;
}

const char* Header::ToChar(HeaderType type)
{
	switch(type)
	{
	case Data:			return "d"; break;
	case Information:	return "i"; break;
	case Queue:			return "q"; break;
	case Error:			return "e"; break;
	default:			return "X"; break;
	}
}

Header::HeaderType Header::ToType(char c)
{
	switch(c)
	{
	case 'd':			return Data; break;
	case 'i':			return Information; break;
	case 'q':			return Queue; break;
	case 'e':			return Error; break;
	}

	return INVALID;
}

void Header::WriteHeader(HeaderType type, Serializer& ser)
{
	Serializer addheader;

	addheader << endtoken << ToStringType(type) << endtoken;
	ser.InsertFirst(addheader);

	string lenstring;
	lenstring = ToLenString(ser.GetStream().size(), 6);

	ser.InsertFirst(lenstring);
}

/*
void Header::ReadHeader(HeaderType& type, int& length, Serializer& ser)
{
	Serializer out;
	char lenbuf[10];

	int len = 7;
	ser.GetBuffer(lenbuf, len);
	lenbuf[len] = '\0';
	int packetlen = atoi(lenbuf);

	string typestr;
	ser.GetString(typestr, "\n");

	string buf = ser.ReadToEOS();

	out << buf;	
	type	= ToHeaderType(typestr);
	length	= packetlen;
	ser		= out;
}
*/

string Header::ToStringType(HeaderType type)
{
	switch(type)
	{
	case Data:			return "data"; break;
	case Information:	return "info"; break;
	case Queue:			return "queue"; break;
	case Error:			return "error"; break;
	default:			return "Unknown"; break;
	}
}

/*
Header::HeaderType Header::ToHeaderType(string typestr)
{
	if		(typestr == "data")		return Data;
	else if (typestr == "info")		return Information;
	else if (typestr == "queue")	return Queue;
	else if (typestr == "error")	return Error;
	else return INVALID;
}
*/