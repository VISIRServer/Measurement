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
#ifndef __SERIALIZER_H__
#define __SERIALIZER_H__

#include <string>

/// Stream serializer utility
/// Simple read/write stream.
class Serializer
{
public:
	//			write data
	Serializer& operator<<(std::string val);
	Serializer& operator<<(double val);
	Serializer& operator<<(int val);
	Serializer& operator<<(unsigned int val);
	Serializer& operator<<(Serializer& (*pFunc)(Serializer&));
	Serializer& operator<<(Serializer& other);

	//			read data
	Serializer& operator>>(std::string& val);
	Serializer& operator>>(double& val);
	Serializer& operator>>(int& val);

	//
	bool		PutBuffer(const char* buffer, size_t length);
	
	void		InsertFirst(std::string in);
	void		InsertFirst(Serializer& in);

	//			conveniance methods
	bool		GetInteger(int& val, std::string separator, bool eatseparators = true);
	bool		GetDouble(double& val, std::string separator, bool eatseparators = true);
	bool		GetString(std::string& val, std::string separator);
	bool		GetBuffer(char* buffer, size_t length);

	bool		EndOfStream();

	// stream methods
	void	Reset();
	// get raw stream
	std::string	GetStream();
	inline const std::string& GetCStream() { return mStream; }
	std::string	ReadToEOS();
	void	SetStream(const char* buffer, size_t len);

	// ctor/dtor
	Serializer(const char* in);
	Serializer(std::string in);
	Serializer();
	virtual ~Serializer();
private:
//	private methods
	void Init();
	std::string	GetStringVal(std::string separator, bool eatseparators = true);
	std::string	ConvertLocale(std::string str);

//	private data
	std::string	mStream; // this is not a string. this is a sequence of chars.
	size_t	mCurrentPos;
};

#endif
