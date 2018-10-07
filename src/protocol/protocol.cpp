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

#include "protocol.h"

using namespace protocol;

/// XXX: Move these..
Transaction::Transaction()
{
	mpOwner = NULL;
	mpIssuer = NULL;
	mErrorState = NoError;
}

Transaction::~Transaction()
{
	for(tRequests::iterator it = mRequests.begin(); it != mRequests.end(); it++)
	{
		delete *it;
	}
	mRequests.clear();
}

void Transaction::AddRequest(Request* pRequest)
{
	mRequests.push_back(pRequest);
}

void Transaction::SetOwner(void* pOwner)
{
	mpOwner = pOwner;
}

void* Transaction::GetOwner()
{
	return mpOwner;
}

void Transaction::SetIssuer(TransactionIssuer* pIssuer)
{
	mpIssuer = pIssuer;
}

void Transaction::Abort(std::string error, TransactionErrorType errortype)
{
	mError = error;
	mErrorState = errortype;
}