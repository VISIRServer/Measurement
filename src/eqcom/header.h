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

#pragma once
#ifndef __EQ_HEADER_H__
#define __EQ_HEADER_H__

#include <string>

class Serializer;

namespace EqSrv
{

/// Writes protocol header
class Header
{
public:
	enum HeaderType
	{
		Data			= 'd',
		Information		= 'i',
		Queue			= 'q',
		Error			= 'e',

		INVALID			= 'X',
	};

	// writes a header first before the data in the serializer
	static void WriteHeader(HeaderType type, Serializer& ser);

	// reads the header and removes the data from the serializer
	//static void ReadHeader(HeaderType& type, int& length, Serializer& ser);
private:
	static const char*		ToChar(HeaderType type);
	static HeaderType		ToType(char c);
	static std::string		ToLenString(size_t len, int outlen);
	static std::string		ToStringType(HeaderType type);
	//static HeaderType		ToHeaderType(std::string typestr);
};

} // end of namespace

#endif
