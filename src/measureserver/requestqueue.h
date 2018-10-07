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

#ifndef __REQUEST_QUEUE_H__
#define __REQUEST_QUEUE_H__

//#include "request.h"

#include <list>

// forward decl.
class Client;

class Request;

/// Queues the request for future processing.
/// When a client issues a measurement, the request is encoded and placed in the queue.
/// When the server has time to process the request, the request is removed from the queue and sent to the server.
class RequestQueue
{
public:
	///		Add a request for processing on the queue
	void	AddRequest(Request* request);

	///		Remove all request from a specific client
	void	RemoveRequestsFrom(Client* client);

	///		Forcefully remove request from queue
	void	RemoveRequest(Request* pRequest);

	///		Check if new requests can be processed, and handle them if so.
	bool	ProcessQueue();

	///		Removes the request when the request is handled.
	void	RequestDone(Request* request);

	inline size_t	NumHandledRequests() { return mHandledRequests; }

			RequestQueue();
	virtual ~RequestQueue();
private:

	void		HandleRequest(Request* request);

	typedef		std::list< Request* > tQueue;
	tQueue		mQueue;
	bool		mWaiting;
	size_t		mHandledRequests;
};

#endif
