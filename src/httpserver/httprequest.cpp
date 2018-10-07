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

#include "httprequest.h"

#include <stringop.h>

using namespace std;

HTTPRequest::HTTPRequest()
{
	Reset();
}

HTTPRequest::~HTTPRequest()
{
}

void HTTPRequest::Reset()
{
	mLineState = eRequestLine;
	mCurrentOffset = 0;

	mHeaderSize = 0;
	mContentLength = 0;

	//mKeepAlive = 0;
	mConnectionType = eConnectionClose;
}

bool HTTPRequest::ParseRequest(char* data, size_t length, int& error)
{
	switch(mLineState)
	{
	case eRequestLine:
	case eHeaders:
		while(ParseLine(data, length, error))
		{
			continue;
		}
		if (error < 0) return true;
		break;
	case eData: break;
	}

	if (mLineState == eData)
	{
		// just wait for the complete packet to come in
		if (length >= mHeaderSize + ContentLength()) return true;
		return false;
	}

	return false;
}

bool HTTPRequest::ParseLine(char* data, size_t length, int& error)
{
	int startOffset = mCurrentOffset;
	int endOffset = -1;

	// check if we have a complete line
	for(size_t i = startOffset+1; i < length; i++)
	{
		if (data[i-1] == '\r' && data[i] == '\n')
		{
			endOffset = i;
			break;
		}
	}

	if (endOffset > 0)
	{
		if (endOffset - startOffset <= 2) // empty line detected
		{
			mHeaderSize = endOffset + 1;
			mLineState = eData;
			error = 0;
			return false;
		}

		if (!ProcessLine(data + startOffset, endOffset - startOffset - 1, error)) return false;

		mCurrentOffset = endOffset + 1;

		return true;
	}

	// no complete line
	return false;
}

#define WHITESPACE " \t"

bool HTTPRequest::ProcessLine(char* data, size_t length, int& error)
{
	string line;
	line.assign(data, data + length);

	if (mLineState == eRequestLine)
	{
		size_t ws, start;

		start = line.find_first_not_of(WHITESPACE);
		if (start == string::npos) { error = -1; return false;	}

		ws = line.find_first_of(WHITESPACE, start);
		if (ws == string::npos)	{ error = -1; return false;	}
		string verb(line, start, ws);

		start = line.find_first_not_of(WHITESPACE, ws+1);
		if (start == string::npos) { error = -1; return false;	}
		ws = line.find_first_of(WHITESPACE, start);
		if (ws == string::npos)	{ error = -1; return false;	}

		string url(line, start, ws - start);

		start = line.find_first_not_of(WHITESPACE, ws+1);

		string http(line, start);

		if ((http == "HTTP/1.1") || (http == "HTTP/1.0"))
		{
			mVerb = verb;
			mURL = url;
			mLineState = eHeaders;
		}
		else
		{
			error = -1;
			return false;
		}
	}
	else
	{
		size_t colonpos = line.find_first_of(':');
		if (colonpos == string::npos || colonpos == 0)
		{
			error = -1;
			return false;
		}

		string key(line, 0, colonpos);

		size_t nonwhite = line.find_first_not_of(WHITESPACE, colonpos+1);
		if (nonwhite == string::npos) { error = -1; return false; }
		string value(line, nonwhite);

		string lowkey = ToLower(key);

		if (lowkey == "content-length")
		{
			mContentLength = strtol(value.c_str(), NULL, 10);
		}
		if (lowkey == "connection")
		{
			string lowvalue = ToLower(value);
			if (lowvalue == "keep-alive") mConnectionType = eConnectionKeepAlive;
		}		
	}

	return true;
}


size_t HTTPRequest::ContentLength()
{
	return mContentLength;
}

size_t HTTPRequest::RequestSize()
{
	return mContentLength + mHeaderSize;
}

std::string HTTPRequest::GetPayload(char* data, size_t length)
{
	if (mHeaderSize + mContentLength > length) return "";

	return string(data + mHeaderSize, mContentLength);
}