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

#ifndef __HTTP_REQUEST_H__
#define __HTTP_REQUEST_H__

#include <string>

class HTTPRequest
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

	std::string		GetPayload(char* data, size_t length);

	size_t			RequestSize();
	size_t			HeaderSize();
	size_t			ContentLength();

	void			Reset();

	HTTPRequest();
	virtual ~HTTPRequest();
private:
	bool ParseLine(char* data, size_t length, int& error);
	bool ProcessLine(char* data, size_t length, int& error);

	size_t	mCurrentOffset;
	size_t	mHeaderSize;
	size_t	mContentLength;

	eConnectionType	mConnectionType;

	std::string mVerb;
	std::string mURL;

	enum
	{
		eRequestLine,
		eHeaders,
		eData
	} mLineState;
};

#endif
