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

#include "client.h"
#include "session.h"

#include <syslog.h>
#include <stringop.h>
#include <basic_exception.h>

int Client::sLastClientID = 1;

Client::Client()
{
	mpSession	= NULL;
	mpClientID	= sLastClientID++;
}

Client::~Client()
{
}

void Client::ConnectionClosed()
{
	if (mpSession)
	{
		mpSession->Unlock(this);
	}

	if (mpSession && !mpSession->KeepAlive())
	{
		mpSession->Close();
		mpSession = NULL;
	}
}

bool Client::BindToSession(Session* pSession)
{
	// add this check when we have better tracking of the client session..
	/*if (mpSession != NULL && pSession != mpSession)
	{
		//throw BasicException("Session already bound to client");
		return false;
	}*/

	// Its fatal that we unlock the session before rebinding it
	// Otherwise the session can get a reference to a broken client and crash when it times out
	if (mpSession != NULL && pSession != mpSession)
	{
		mpSession->Unlock(this);
		syserr << "Client session has been replaced" << std::endl;
	}

	mpSession = pSession;
	return true;
}

Session* Client::GetSession()
{
	if (mpSession) mpSession->Touch();
	return mpSession;
}

void Client::SessionDestroyed()
{
	for(tListeners::iterator it = mListeners.begin(); it != mListeners.end(); it++)
	{
		(*it)->SessionDestroyed();
	}
	if (mpSession)
	{
		mpSession = NULL;
	}
}

void Client::AddListener(IClientEventListener* pListener)
{
	mListeners.push_back(pListener);
}

void Client::RemoveListener(IClientEventListener* pListener)
{
	mListeners.remove(pListener);
}