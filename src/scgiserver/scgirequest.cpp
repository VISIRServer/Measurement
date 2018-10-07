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

#include "scgirequest.h"

#include <stringop.h>

using namespace std;

SCGIRequest::SCGIRequest()
{
	Reset();
}

SCGIRequest::~SCGIRequest()
{
}

void SCGIRequest::Reset()
{
	mState = eNetstringLine;
	mCurrentOffset = 0;

	mHeaderSize = 0;
	mContentLength = 0;
	mRequestSize = 0;

	//mKeepAlive = 0;
	mConnectionType = eConnectionClose;
}

char* NextNull(char* start, char* end)
{
	if (!(start && end)) return NULL;
	while(start < end) {
		if (*start == '\0') return start;
		start++;
	}
	return NULL;
}

bool SCGIRequest::ParseRequest(char* data, size_t length, int& error)
{
	bool done = false;
	while(ParseToken(data, length, error, done));
	if (error) return true;
	return done;
}

bool SCGIRequest::ParseToken(char* data, size_t length, int& error, bool& done)
{
	switch(mState)
	{
		case eNetstringLine:
		{
			if (length < 15) return false; // wait for more data
			size_t p = 0;
			while(p < length && data[p] != ':') ++p;
			if (p == length) return false;
			string lenstr(data, data + p);
			int len = atoi(lenstr.c_str());
			if (len == 0) {
				error = 1;
				return true;
			}
			mHeaderSize = len;
			mCurrentOffset = p + 1;
			mState = eHeaders;
			mHeaders.clear();
			return true;
		} break;

		case eHeaders:
		{
			// wait for all header data to come in
			if (mCurrentOffset + mHeaderSize > length) return false;

			char* p = data + mCurrentOffset;
			char* end = data + mCurrentOffset + mHeaderSize;

			while(p < end) {
				// tokenize key value pairs
				char* p2 = NextNull(p, end);
				char* p3 = NextNull(p2+1, end);
				// check if any are null
				if (!(p2 && p3)) {
					error = 1;
					return false;
				}

				string key(p, p2);
				string value(p2+1, p3);

				mHeaders[key] = value;
				p = p3+1;
			}

			mState = eData;
			mCurrentOffset = (p - data) + 1; // skip ,

			// requests should contain SCGI 1
			int isscgi = atoi(mHeaders["SCGI"].c_str());
			if (!isscgi) {
				error = 2;
				return false;
			}

			mContentLength = atoi(mHeaders["CONTENT_LENGTH"].c_str());
			mVerb = mHeaders["REQUEST_METHOD"];
			mURL = mHeaders["REQUEST_URI"];
			mRemoteAddr = mHeaders["REMOTE_ADDR"];

			return true;

		} break;

		case eData:
		{			
			if (mCurrentOffset + mContentLength > length) {
				return false;
			}

			mRequestSize = mCurrentOffset + mContentLength;

			mContent = string(data + mCurrentOffset, data + mCurrentOffset + mContentLength);
			mState = eNetstringLine;
			done = true;
			return false;
		} break;
	}

	return false;
}


size_t SCGIRequest::ContentLength()
{
	return mContentLength;
}

size_t SCGIRequest::RequestSize()
{
	return mRequestSize;
}

std::string SCGIRequest::GetPayload()
{
	return mContent;
}