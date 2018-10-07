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

#ifndef __SESSION_H__
#define __SESSION_H__

#include <string>
#include <map>

class InstrumentBlock;
class Client;
class IAuthEntry;
class SessionRegistry;

namespace protocol { class Transaction; }

class Session
{
public:
	inline	size_t				GetNumber()		{ return mNumber; }
	inline	std::string			GetKey()		{ return mKey; }
	inline	InstrumentBlock*	GetBlock()		{ return mpBlock; }
	inline	std::string			GetCookie()		{ return mCookie; }

	inline	bool				KeepAlive()		{ return mKeepAlive; }
	inline	time_t				LastActive()	{ return mLastActive; }
	inline	Client*				GetLock()		{ return mpLock; }
	inline	int					GetPriority()	{ return mPrio; }

	// Updates last active
	void	Touch();

	bool	Lock(Client*	pClient);
	bool	Unlock(Client*	pClient);

	void	SetActiveTransaction(protocol::Transaction* pTransaction);
	bool	HasActiveTransaction();

	void	Close();

	Session(SessionRegistry* pSessionReg, std::string key, std::string cookie, bool keepalive, int prio);
	virtual ~Session();

	bool	LessPrioThan(Session* pOther);

	static size_t		sSessionCounter;
private:
	SessionRegistry*	mpSessionReg;
	InstrumentBlock*	mpBlock;
	protocol::Transaction*	mpTransaction;
	std::string			mKey;
	size_t				mNumber;
	
	time_t				mCreatedAt;
	time_t				mValidUntil;
	time_t				mLastActive;

	bool				mKeepAlive;		// long lived session
	//std::string		mIP;			// lock to ip
	std::string			mCookie;
	Client*				mpLock;
	int					mPrio;
};

class SessionRegistry
{
public:
	Session*	CreateSession(std::string cookie, bool keepalive, IAuthEntry* entry);
	void		CloseSession(Session* pSession);

	Session*	GetSession(std::string sessionkey) const;
	bool		CheckCookie(Client* pClient, const std::string& cookie) const;
	Session*	GetSessionFromCookie(const std::string& cookie) const;

	Session*	BindToSession(Client* pClient, std::string sessionKey);

	size_t		NumActiveSessions() const;

	bool		Tick();

	SessionRegistry(size_t maxSessions, double sessionTimeout);
	~SessionRegistry();
private:
	std::string	GenerateName() const;

	void		DestroySession(Session* pSession);
	bool		DestroyLeastPrio(int lowerthan);

	typedef		std::map<std::string, Session*> tSessions;

	tSessions	mSessions;
	size_t		mMaxSessions;
	double		mSessionTimeout;
};

#endif
