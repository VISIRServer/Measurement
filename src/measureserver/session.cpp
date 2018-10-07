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

#include "session.h"

#include "client.h"
#include "authentry.h"

#include <stringop.h>
#include <instruments/instrumentblock.h>
#include <time.h>
#include <syslog.h>

#include <contrib/md5.h>

#include <basic_exception.h>

size_t Session::sSessionCounter = 1;

Session::Session(SessionRegistry* pSessionReg, std::string key, std::string cookie, bool keepalive, int prio)
{
	mpSessionReg = pSessionReg;
	mNumber = sSessionCounter++;
	mKey = key;
	mCookie = cookie;
	mpBlock = new InstrumentBlock();
	mpTransaction = NULL;
	
	mKeepAlive = keepalive;
	mpLock = NULL;
	mPrio = prio;

	mCreatedAt = time(0);
	mValidUntil = -1;
	mLastActive = time(0);
}

Session::~Session()
{
	delete mpBlock;
	mpBlock = NULL;
}

void Session::Close()
{
	mpSessionReg->CloseSession(this);
	// no we're dead..
}

void Session::Touch()
{
	mLastActive = time(0);
}

bool Session::Lock(Client* pClient)
{
	if (mpLock && (pClient != mpLock)) return false;
	if (!pClient->BindToSession(this)) return false;

	mpLock = pClient;	
	return true;
}

bool Session::Unlock(Client* pClient)
{
	if (mpLock != pClient) return false;
	mpLock = NULL;
	return true;
}

bool Session::LessPrioThan(Session* pOther)
{
	if (mPrio < pOther->mPrio) return true;
	if (mCreatedAt > pOther->mCreatedAt) return true;
	return false;
}

void Session::SetActiveTransaction(protocol::Transaction* pTransaction)
{
	if (mpTransaction && pTransaction != NULL)
	{
		throw BasicException("Transaction already in progress");
	}
	mpTransaction = pTransaction;
}

bool Session::HasActiveTransaction()
{
	return (mpTransaction != 0);
}

///////////

SessionRegistry::SessionRegistry(size_t maxSessions, double sessionTimeout) // le pasa el maximo de sesiones y el timeout de sesion
{
	mMaxSessions	= maxSessions; // almacena el maximo num de sesiones 
	mSessionTimeout	= sessionTimeout; // almacena el timeout de sesion
}

SessionRegistry::~SessionRegistry()
{
	for(tSessions::iterator it = mSessions.begin(); it != mSessions.end(); it++)
	{
		delete it->second;
	}
}

Session* SessionRegistry::CreateSession(std::string cookie, bool keepalive, IAuthEntry* entry)
{
	int prio = (entry ? entry->GetPrio() : 0);
	if (mSessions.size() >= mMaxSessions)
	{
		// try to find, and destroy, a session with lower priority than this
		// if there is none, you can't create a session
		if (!DestroyLeastPrio(prio)) return NULL;
	}

	std::string sessionkey = GenerateName();
	Session* pNewSession = new Session(this, sessionkey, cookie, keepalive, prio);
	mSessions[sessionkey] = pNewSession;
	return pNewSession;
}

void SessionRegistry::CloseSession(Session* pSession)
{
	for(tSessions::iterator it = mSessions.begin(); it != mSessions.end(); it++)
	{
		if (it->second == pSession)
		{
			DestroySession(pSession);
			mSessions.erase(it);
			return;
		}
	}
}

Session* SessionRegistry::GetSession(std::string sessionkey) const
{
	tSessions::const_iterator finder = mSessions.find(sessionkey);
	if (finder != mSessions.end()) return finder->second;
	return 0;
}

bool SessionRegistry::CheckCookie(Client* pClient, const std::string& cookie) const
{
	for(tSessions::const_iterator it = mSessions.begin(); it != mSessions.end(); it++)
	{
		if (it->second->GetCookie() == cookie) return false;
	}

	return true;
}

Session* SessionRegistry::GetSessionFromCookie(const std::string& cookie) const
{
	for(tSessions::const_iterator it = mSessions.begin(); it != mSessions.end(); it++)
	{
		if (it->second->GetCookie() == cookie) return it->second;
	}

	return NULL;
}

bool SessionRegistry::Tick()
{
	double timeout = mSessionTimeout;
	time_t now = time(0);

	tSessions::iterator it = mSessions.begin();
	while(it != mSessions.end())
	{
		// check for timeout
		if (it->second->LastActive() < (now - timeout))
		{
			tSessions::iterator deleteit = it;
			it++;

			sysout << timestamp << "Session timed out: " << deleteit->second->GetNumber() << std::endl;
			DestroySession(deleteit->second);
			
			mSessions.erase(deleteit);
		}
		else
		{
			it++;
		}
	}
	return true;
}

std::string	SessionRegistry::GenerateName() const
{
	unsigned char md5sum[16];
	std::string input = "openlabs session hash seed";
	input += ToString((int)time(0));
	input += ToString(rand());
	input += ToString((int)Session::sSessionCounter);

	md5((unsigned char*)input.c_str(), input.size(), md5sum);
	char output[33];
	for( int i = 0; i < 16; i++ )
	{
		sprintf(&output[i*2] , "%02x", md5sum[i]);
	}
	output[32] = '\0';
	return output;
}

void SessionRegistry::DestroySession(Session* pSession)
{
	// we better make sure this lock is updated, or this will crash horribly
	if (pSession->GetLock())
	{
		pSession->GetLock()->SessionDestroyed();
	}
	delete pSession;
}

bool SessionRegistry::DestroyLeastPrio(int lowerthan)
{
	tSessions::iterator currentit = mSessions.end();

	for(tSessions::iterator it = mSessions.begin(); it != mSessions.end(); it++)
	{
		if (it->second->GetPriority() < lowerthan)
		{
			if (currentit != mSessions.end())
			{
				if (currentit->second->LastActive() > it->second->LastActive()) currentit = it;
			}
			else
			{
				currentit = it;
			}
		}
	}

	if (currentit != mSessions.end())
	{
		sysout << timestamp << "Destroying low priority session" << std::endl;
		DestroySession(currentit->second);
		mSessions.erase(currentit);
	}
	else
	{
		return false;
	}

	return true; // check if this is correct
}

size_t SessionRegistry::NumActiveSessions() const
{
	return mSessions.size();
}

Session* SessionRegistry::BindToSession(Client* pClient, std::string sessionKey)
{
	Session* pSession = GetSession(sessionKey);
	if (pSession)
	{
		if (!pSession->Lock(pClient)){
			sysout << timestamp << "Failed to lock session" << std::endl;

			// force the old client to release the session?
			pSession->Unlock(pSession->GetLock()); // this might be dangerous
			if (pSession->Lock(pClient)) return pSession; // and try again..

			return NULL;
		}
		pSession->Touch();
	}
	return pSession;
}