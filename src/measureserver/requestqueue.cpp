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

#include "requestqueue.h"
#include "request.h"

#include <basic_exception.h>
#include <syslog.h>

RequestQueue::RequestQueue()
{
	mWaiting = false;
	mHandledRequests = 0;
}

RequestQueue::~RequestQueue()
{
	while(!mQueue.empty())
	{
		delete mQueue.front();
		mQueue.pop_front();
	}
}

void RequestQueue::AddRequest(Request* request)
{
	mQueue.push_back(request);
}

void RequestQueue::RemoveRequestsFrom(Client* client)
{
	if (mQueue.empty()) return;

	if (mQueue.front()->GetOwner() == client)
	{
		mWaiting = false;
	}

	tQueue::iterator i = mQueue.begin();
	while(i != mQueue.end())
	{		
		if ((*i)->GetOwner() == client)
		{
			Request* temp = *i;
			i = mQueue.erase(i);

			temp->Cancel();
			delete temp;
		}
		else i++;
	}
}

void RequestQueue::RemoveRequest(Request* pRequest)
{
	if (pRequest == mQueue.front())
	{
		syserr << "Forcefully removing the currently handled request." << std::endl;
		mWaiting = false; // xxx
	}
	mQueue.remove(pRequest);
	delete pRequest;
}

bool RequestQueue::ProcessQueue()
{
	if (mWaiting)
	{
		mQueue.front()->HasTimedOut();
		return true;
	}

	if (mQueue.empty()) return true;

	HandleRequest(mQueue.front());

	return true;
}

void RequestQueue::HandleRequest(Request* request)
{
	mWaiting = true;
	request->Send();
}

void RequestQueue::RequestDone(Request* request)
{
	if ( request != mQueue.front())
	{
		syserr << "Calling RequestDone on request not on the stack. Probably because of a client that shut down during handling of a transaction." << std::endl;
		RemoveRequest(request); // make sure to delete the request in any case
		// xxx: throw here instead? This shouldn't happen. Problem is that noone catches any exceptions upstream
		return;
	}

	// allow next request to be processed
	mHandledRequests++;
	mWaiting = false;

	mQueue.pop_front();

	delete request; // delete the frontmost request
}
