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
#ifndef __AUTH_ENTRY_H__
#define __AUTH_ENTRY_H__

#include <string>

class IAuthEntry
{
public:
	virtual const char*		GetCookie() = 0;
	virtual const char*		GetIP()		= 0;
	virtual unsigned int	GetTimeout()= 0;
	virtual int				GetPrio()	= 0;

	virtual void			SetCookie(const char* cookie)	= 0;
	virtual void			SetIP(const char* ip)			= 0;
	virtual void			SetTimeout(unsigned int timeout)= 0;
	virtual void			SetPrio(int prio)				= 0;

	virtual ~IAuthEntry() {}
};

class ICredentials
{
public:
	virtual const char*	GetCookie() const = 0;
	virtual ~ICredentials() {}
};

class IAuthenticator
{
public:
	virtual int Init() = 0;
	virtual int FetchAuthEntry(const ICredentials* pInCred, IAuthEntry* pOutAuthEntry, int cacheId) = 0;
	virtual ~IAuthenticator() {}
};

#endif
