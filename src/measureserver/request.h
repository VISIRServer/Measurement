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

#ifndef __REQUEST_H__
#define __REQUEST_H__

// forward decl.
class Client;
class RequestQueue;

/// Base class for requests
/// Keeps track of the owning client and the queue where is belongs.
class Request
{
public:
	///						Send the request
	virtual void			Send()		= 0;

	///						Encode and prepare the request for handling
	virtual bool			BuildRequest() = 0;

	///						Check if request has timed out, and if so kill the request
	virtual bool			HasTimedOut();

	virtual void			Cancel() = 0;

	///						Get the owner, the client, who issued the request
	Client*					GetOwner();

	Request(RequestQueue* pQueue, Client* pOwner);
	virtual ~Request();
protected:
	Client*			mpOwner;
	RequestQueue*	mpQueue;
};

#endif
