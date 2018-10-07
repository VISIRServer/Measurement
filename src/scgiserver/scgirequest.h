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
 * Copyright (c) 2008 Johan Zackrisson
 * All Rights Reserved.
 */

#ifndef __SCGI_REQUEST_H__
#define __SCGI_REQUEST_H__

#include <string>
#include <map>

class SCGIRequest
{
public:
	enum eConnectionType
	{
		eConnectionClose,
		eConnectionKeepAlive
	};

	bool	ParseRequest(char* data, size_t length, int& error);

	const std::string& Verb()	{ return mVerb; }
	const std::string& URL()	{ return mURL; }

	eConnectionType ConnectionType() { return mConnectionType; }

	std::string		GetPayload();
	std::string		RemoteAddr() { return mRemoteAddr; }

	size_t			RequestSize();
	size_t			HeaderSize();
	size_t			ContentLength();

	void			Reset();

	SCGIRequest();
	virtual ~SCGIRequest();
private:
	bool ParseToken(char* data, size_t length, int& error, bool& done);

	size_t	mCurrentOffset;
	size_t	mHeaderSize;
	size_t	mContentLength;
	size_t	mRequestSize;
	std::string mContent;

	eConnectionType	mConnectionType;

	std::string mVerb;
	std::string mURL;
	std::string mRemoteAddr;

	typedef std::map<std::string, std::string> tHeaders;
	tHeaders mHeaders;

	enum
	{
		eNetstringLine,
		eHeaders,
		eData
	} mState;
};

#endif
