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

#include "serializer.h"
#include "basic_exception.h"

#include <assert.h>
#include "stringop.h"

using namespace std;

// ugly workaround for old equipment server..
//#define USE_LOCALE
//#define LOC_SWEDISH

void Serializer::Init()
{
#ifdef USE_LOCALE
#ifdef LOC_SWEDISH
	setlocale( LC_ALL, "Swedish" );
#else
	setlocale( LC_ALL, "English" );
#endif
#endif // USE_LOCALE

	mCurrentPos = 0;
}

Serializer::Serializer()
{
	Init();
}

Serializer::Serializer(const char* in)
{
	Init();
	mStream = in;
}

Serializer::Serializer(std::string in)
{
	Init();
	mStream = in;
}

Serializer::~Serializer()
{
}

Serializer& Serializer::operator<<(string val)
{
	mStream.insert(mStream.end(), val.begin(), val.end());
	return (*this);
}

Serializer& Serializer::operator<<(double val)
{
	mStream += ToString(val);
	return (*this);
}

Serializer& Serializer::operator<<(int val)
{
	mStream += ToString(val);
	return (*this);
}

Serializer& Serializer::operator<<(unsigned int val)
{
	mStream += ToString(val);
	return (*this);
}

Serializer& Serializer::operator<<(Serializer& (*pFunc)(Serializer& in))
{
	return (*pFunc)(*this);
}

Serializer& Serializer::operator<<(Serializer& other)
{
	mStream.insert(mStream.end(), other.mStream.begin(), other.mStream.end());
	return (*this);
}

bool Serializer::PutBuffer(const char* buffer, size_t length)
{
	mStream.insert(mStream.end(),buffer, buffer + length);
	return true;
}

Serializer& Serializer::operator>>(string& val)
{
	assert(0);
	return (*this);
}

Serializer& Serializer::operator>>(double& val)
{
	assert(0);
	return (*this);
}

Serializer& Serializer::operator>>(int& val)
{
	assert(0);
	return (*this);
}

void Serializer::Reset()
{
	mStream = "";
	mCurrentPos = 0;
}

string Serializer::GetStream()
{
	return mStream;
}

void Serializer::SetStream(const char* buffer, size_t len)
{
	mStream.insert(mStream.begin(), buffer, buffer + len);
}

string Serializer::GetStringVal(string separator, bool eatseparators)
{
	// if currentpos is at EOS then return..
	if (mCurrentPos == string::npos) return "";

	// find separator or end of string..
	size_t pos = mStream.find_first_of(separator, mCurrentPos);
	string strval;
	if (pos != string::npos)
	{
		strval = string(mStream, mCurrentPos, pos - mCurrentPos);
		// step over the separator('s)
		if (eatseparators) mCurrentPos = mStream.find_first_not_of(separator,pos);
		else mCurrentPos = pos + 1;
	}
	else
	{
		strval = string(mStream, mCurrentPos, mStream.size() - mCurrentPos);
		mCurrentPos = string::npos; // set to npos
	}

	return strval;
}

bool Serializer::GetInteger(int& val, string separator, bool eatseparators)
{
	string strval = GetStringVal(separator, eatseparators);
	strval = ConvertLocale(strval);

	int rv = sscanf(strval.c_str(),"%i",&val);
	if (rv <= 0) return false;
	return true;
}

bool Serializer::GetDouble(double& val, string separator, bool eatseparators)
{
	string strval = GetStringVal(separator, eatseparators);
	strval = ConvertLocale(strval);

	double test = atof(strval.c_str());
	val = test;
	return true;
}

bool Serializer::GetString(string& val, string separator)
{
	val = GetStringVal(separator);
	return true;
}

bool Serializer::GetBuffer(char* buffer, size_t length)
{
	if (mCurrentPos + length >= mStream.size()) return false;
	mStream.copy(buffer,length, mCurrentPos);
	mCurrentPos += length;
	return true;
}

bool Serializer::EndOfStream()
{
	if (mCurrentPos == string::npos) return true;
	else return false;
}

string Serializer::ReadToEOS()
{
	if (EndOfStream()) return "";

	string rv(mStream, mCurrentPos, mStream.size() - mCurrentPos);
	mCurrentPos = string::npos;
	return rv;
}

void Serializer::InsertFirst(string in)
{
	// doens't change mCurrentPos.. watch out
	mStream.insert(mStream.begin(), in.begin(), in.end());
}

void Serializer::InsertFirst(Serializer& in)
{
	// doens't change mCurrentPos.. watch out
	mStream.insert(mStream.begin(), in.mStream.begin(), in.mStream.end());
}

string Serializer::ConvertLocale(string str)
{

#ifdef USE_LOCALE
#ifdef LOC_SWEDISH
	for(size_t i=0;i<str.size();i++)
	{
		if (str[i] == '.') str[i] = ',';
	}
#else
	for(size_t i=0;i<str.size();i++)
	{
		if (str[i] == ',') str[i] = '.';
	}
#endif
#endif // USE_LOCALE

	return str;
}
