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
 * Copyright (c) 2008-2009 Johan Zackrisson
 * All Rights Reserved.
 */

#pragma once
#ifndef __PROTOCOL_AUTH_H__
#define __PROTOCOL_AUTH_H__

#include "protocol.h"

#include <string>

namespace protocol
{

class AuthRequest : public Request
{
public:
	AuthRequest(std::string cookie, bool keepalive = false);
	virtual ~AuthRequest() {}
	inline std::string GetCookie() const { return mCookie; }
	inline bool GetKeepAlive() const { return mKeepAlive; }
private:
	bool		mKeepAlive;
	std::string mCookie;	
};

class AuthResponse : public Response
{
public:
	AuthResponse(std::string key);
	virtual ~AuthResponse() {}
	inline std::string GetSessionKey() const { return mKey; }
private:
	std::string mKey;
};

} // end of namespace protocol

#endif
