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
#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__

#include <string>
#include <list>

// forward decl.
class InstrumentBlock;

namespace protocol
{

// namespace for the enum
struct RequestType
{
	// XXX: replace with clsid or some unique id that doesn't need to be entered centrally
	enum eRequestType
	{
		Invalid			= 0,
		Authorize		= 1,
		Information		= 2,
		Measurement		= 3,
		Heartbeat		= 4,
		DomainPolicy	= 5,
	};
};

class Response
{
public:
	inline RequestType::eRequestType	GetType()	{ return mType; }

	Response() { mType = RequestType::Invalid; }
	virtual ~Response() {}
protected:
	RequestType::eRequestType		mType;
};

class Request
{
public:
	inline RequestType::eRequestType		GetType()	{ return mType; }

	// this will take ownership of the reponse
	void SetResponse(Response* pResponse) { mpResponse = pResponse; } // throw if already set?
	Response* GetResponse() { return mpResponse; }

	Request()
	{
		mType = RequestType::Invalid;
		mpResponse = NULL;
	}

	virtual ~Request()
	{
		if (mpResponse) delete mpResponse;
		mpResponse = NULL;
	}
protected:
	RequestType::eRequestType	mType;
	Response*					mpResponse;
};

enum TransactionErrorType
{
	NoError,
	Notification,
	Fatal
};

class Transaction;

class TransactionIssuer
{
public:
	/// Called after the transaction is handled successfully
	virtual void TransactionComplete(Transaction* pTransaction) = 0;
	/// Called when a transaction fails
	virtual void TransactionError(Transaction* pTransaction, const char* msg, TransactionErrorType type) = 0;

	TransactionIssuer() {}
	virtual ~TransactionIssuer() {}
};

/// Class for containing multiple requests that must be handled in sequence
/// If one fails, the whole transaction is canceled.
/// As soon as a request is added, the ownership is transfered to the Transaction.
class Transaction
{
public:
	typedef std::list< Request* > tRequests;

	void	AddRequest(Request* pRequest);

	inline const tRequests& GetRequests() { return mRequests; }

	void	Abort(std::string error, TransactionErrorType errortype);

	inline std::string GetError() { return mError; }
	inline TransactionErrorType	GetErrorState() const { return mErrorState; }

	// I don't like this way of handling the owner info.. but it will have to do for now..
	void	SetOwner(void* pOwner);
	void*	GetOwner();

	void				SetIssuer(TransactionIssuer* pIssuer);
	TransactionIssuer*	GetIssuer() { return mpIssuer; }

	Transaction();
	virtual ~Transaction();
private:
	TransactionIssuer*	mpIssuer;
	tRequests			mRequests;
	void*				mpOwner;

	std::string				mError;
	TransactionErrorType	mErrorState;
};

class TransactionCallback
{
public:
	virtual InstrumentBlock* GetInstrumentBlock() = 0;

	virtual void TransactionDone() = 0;
	virtual void TransactionError(const char* msg, TransactionErrorType type) = 0;

	virtual ~TransactionCallback() {}
};

class TransactionHandler
{
public:
	/// Check if the commander can handle this transaction
	virtual bool	CanHandle(Transaction* pTransaction)	= 0;

	/// Carry out the transaction
	virtual bool	Perform(Transaction* pTransaction, TransactionCallback* pCallback)		= 0;

	/// Cancel the transaction, for instance because of a timeout
	virtual bool	Cancel(Transaction* pTransaction, TransactionCallback* pCallback)		= 0;

	virtual ~TransactionHandler() {}
};

class IProtocolService
{
public:
	virtual const char* GetCrossDomainPolicy() = 0;
	virtual ~IProtocolService() {}
};

} // end of namespace protocol

#endif
