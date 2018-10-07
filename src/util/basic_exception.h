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
#ifndef __BASIC_EXCEPTION_H__
#define __BASIC_EXCEPTION_H__

#include <string>
#include <exception>

/// Base class for all exceptions thrown in the system.
/// Holds a errno and description of the error
/// \todo errno is not used as it is. There should be some way of "allocating" static and dynamic errors.

class BasicException : public std::exception
{
public:
	BasicException(std::string error, int errornr = 0)
	{
		mErrorString	= error;
		mErrorNum		= errornr;
	}
	
	virtual ~BasicException() throw () {}
	
	virtual const char* what() const throw()
	{		
		return mErrorString.c_str();
	}

	virtual const int errornr() const
	{
		return mErrorNum;
	}
	
private:
	int			mErrorNum;
	std::string	mErrorString;
};

class ValidationException : public std::exception
{
public:
	ValidationException(std::string msg)
	{
		mException = msg;
	}
	virtual ~ValidationException() throw () {}

	virtual const char* what() const throw()
	{		
		return mException.c_str();
	}
private:
	std::string	mException;

};

#endif
