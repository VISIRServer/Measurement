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
 * Copyright (c) 2007-2009 Johan Zackrisson
 * All Rights Reserved.
 */

#include "authentication.h"

#include "client.h"
#include "clientmanager.h"
#include "authentry.h"
#include "session.h"

#include <instruments/instrumentblock.h>

#include <protocol/protocol.h>
#include <protocol/auth.h>

#include <basic_exception.h>
#include <syslog.h>

#include <assert.h>

class AuthEntry : public IAuthEntry
{
public:
	virtual const char*		GetCookie()	{ return mCookie.c_str(); }
	virtual const char*		GetIP()		{ return mIP.c_str(); }
	virtual unsigned int	GetTimeout(){ return mTimeout; }
	virtual int				GetPrio()	{ return mPrio; }

	virtual void			SetCookie(const char* cookie)	{ mCookie = cookie; }
	virtual void			SetIP(const char* ip)			{ mIP = ip; }
	virtual void			SetTimeout(unsigned int timeout){ mTimeout = timeout; }
	virtual void			SetPrio(int prio)				{ mPrio = prio; }

	virtual ~AuthEntry() {}

	AuthEntry()
	{
		mCookie = "";
		mIP = "";
		mTimeout = UINT_MAX;
		mPrio = 0;
	}

	AuthEntry(std::string cookie, std::string ip, unsigned int timeout, int prio)
	{
		mCookie = cookie;
		mIP = ip;
		mTimeout = timeout;
		mPrio = prio;
	}

private:
	std::string		mCookie;
	std::string		mIP;
	unsigned int	mTimeout;
	int				mPrio;
};

class Credentials : public ICredentials
{
public:
	virtual const char*		GetCookie() const { return mCookie.c_str(); }
	virtual ~Credentials() {}

	Credentials(std::string cookie) { mCookie = cookie; }
private:
	std::string mCookie;
};


Authentication::Authentication(SessionRegistry* pSessionReg, bool allowKeepAlive, bool bypassAuth)
{
	mBypassAuth		= bypassAuth;
	mAllowKeepAlive	= allowKeepAlive;
	mpSessionReg	= pSessionReg;
	mCacheId = 1;
}

Authentication::~Authentication()
{
	if (!mAuthenticators.empty())
	{
		syserr << "Warning: One or more authentication modules was not unregistered" << std::endl;
	}
}

int Authentication::RegisterAuthenticator(IAuthenticator* pAuth)
{
	mAuthenticators.push_back(pAuth);
	return 1;
}

int Authentication::UnregisterAuthenticator(IAuthenticator* pAuth)
{
	mAuthenticators.remove(pAuth);
	return 1;
}

bool Authentication::Init()
{
	if (mBypassAuth) return true;

	for(tAuthenticators::const_iterator it = mAuthenticators.begin(); it != mAuthenticators.end(); it++)
	{
		if (!(*it)->Init())
		{
			syserr << "Failed to init authenticator" << std::endl;
			return false;
		}
	}

	return true;
}

bool Authentication::Tick()
{
	++mCacheId;
	return true;
}

bool Authentication::Authenticate(Client* pClient, const std::string& cookie, bool keepalive)
{
	if (keepalive && !mAllowKeepAlive)
	{
		syserr << timestamp << "Client is trying to use keepalive when it is not allowed" << std::endl;
		return false;
	}

	if (mBypassAuth)
	{
		AuthEntry entry(cookie, "<unknown>", 0xffffffff, 5);
		return CreateSession(pClient, cookie, keepalive, &entry);
	}

	// check if there is a session using the cookie
	Session* pSession = mpSessionReg->GetSessionFromCookie(cookie);
	if (pSession)
	{
		// session cookie is already in use, check if it can be locked by this client
		if (!pSession->Lock(pClient))
		{
			syserr << timestamp << "Cookie already in use by other client" << std::endl;
			return false;
		}

		sysout << timestamp << "Session re-bound to new client" << std::endl;
		pClient->BindToSession(pSession);

		return true;
	}

	Credentials aCred(cookie);
	AuthEntry	aAuthEntry;

	for(tAuthenticators::const_iterator it = mAuthenticators.begin(); it != mAuthenticators.end(); it++)
	{
		if ((*it)->FetchAuthEntry(&aCred, &aAuthEntry, mCacheId))
		{
			return CreateSession(pClient, cookie, keepalive, &aAuthEntry);
		}
	}

	// no authenticator passed us, return false
	return false;
}

bool Authentication::CreateSession(Client* pClient, const std::string& cookie, bool keepalive, IAuthEntry* pEntry)
{
	// XXX: this need to handle the failed case
	Session* pSession = mpSessionReg->CreateSession(cookie, keepalive, pEntry);
	if (pSession)
	{
		if (!pSession->Lock(pClient)) return false;
		//pClient->BindToSession(pSession);
		return true;
	}
	else return false;
}