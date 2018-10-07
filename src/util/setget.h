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
#ifndef __SET_GET_H__
#define __SET_GET_H__

#include <basic_exception.h>

#define SET_GET(type, name, variable) \
	inline void Set##name( type val ) { variable = val; } \
	inline const type Get##name() const { return variable; }

#define SET_GET_STR( name ) \
	void Set##name(const std::string& str); \
	std::string	Get##name() const

// set/get string implementation magic
#define IMPL_SET_GET_STR( cls, name, lookup, var, cast) \
std::string cls::Get##name() const { return lookup[var]; } \
void cls::Set##name(const std::string& str) \
{ \
	for (int i=0;lookup[i]; i++) \
	{ \
		if (str == lookup[i]) \
		{ \
			var = (cast)i; \
			return; \
		} \
	} \
	throw ValidationException(#cls "::Set"#name " unable to convert"); \
} \

#endif
