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
#ifndef __DATABASE_MYSQL_H__
#define __DATABASE_MYSQL_H__

#include <measureserver/authentry.h> // move this to protocol

/// \TODO: WARNING WARNING Must change mysql library, its GPL!

class ModuleServices;

namespace DBMysql
{

class InternalDB;

/// MySQL Database authorization module
class MysqlDatabase : public IAuthenticator
{
public:
	// functions from base class
	virtual int Init();
	virtual int FetchAuthEntry(const ICredentials* pInCred, IAuthEntry* pOutAuthEntry, int cacheId);

	void	SetService(ModuleServices* pServices);

	// ctor/dtor
					MysqlDatabase();
	virtual			~MysqlDatabase();
private:
	bool UpdateDatabase();
	bool Connect();

	/// Hidden database structures
	/// We don't want to expose the Mysql data structures to the rest of the code..
	InternalDB*		mpDB;
	ModuleServices* mpServices;

	int mCacheId;
};

} // end of namespace

#endif
