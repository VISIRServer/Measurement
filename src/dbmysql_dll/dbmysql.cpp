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

#include "dbmysql.h"
#include "dbmysqlmodule.h"
#include "dblog.h"

#include <config.h>

#include <stringop.h>

#include <windows.h>
#include <stdio.h>
#include <string.h>

#include <mysql.h>
#include <errmsg.h>

#include <syslog.h>

using namespace DBMysql;
using namespace std;

struct InternalDBEntry
{
	std::string		mCookie;
	std::string		mIP;
	unsigned int	mTimeout;
	int				mPrio;

	InternalDBEntry(std::string cookie, std::string ip, unsigned int timeout, int prio)
	{
		mCookie = cookie;
		mIP = ip;
		mTimeout = timeout;
		mPrio = prio;
	}
};

/// We don't want to expose the internal mysql structures to the rest of the system, so we wrap it here.
class DBMysql::InternalDB
{
public:
	MYSQL mMysql;

	typedef std::vector<InternalDBEntry> tEntries;
	tEntries mEntries;
};

MysqlDatabase::MysqlDatabase()
{
	mpDB = NULL;
	mCacheId = 0;
}

MysqlDatabase::~MysqlDatabase()
{
	if (mpDB)
	{
		mysql_close(&mpDB->mMysql);
		delete mpDB;
		mpDB = NULL;
	}
}

void MysqlDatabase::SetService(ModuleServices* pServices)
{
	mpServices = pServices;
}

int MysqlDatabase::Init()
{
	// connect to database.. and check if its up and running
	//printf("dbmysql: Initializing Mysql database\n");
	dblog.Out() << "Initializing Mysql database" << endl;

	mpDB = new InternalDB();

	// initialize
	if (!mysql_init(&mpDB->mMysql)) return false;
	return Connect();
}

bool MysqlDatabase::Connect()
{
	//syserr << "dbmysql: Connecting to database" << endl;
	dblog.Log(5) << "dbmysql: Connecting to database" << endl;

	Config* pConfig = mpServices->GetConfig();

	string host			= pConfig->GetString("DB.Host", "localhost");
	int port			= pConfig->GetInt("DB.Port", 3306);
	string user			= pConfig->GetString("DB.User", "");
	string password		= pConfig->GetString("DB.Password", "");
	string database		= pConfig->GetString("DB.Database", "");

	MYSQL* rv = mysql_real_connect(	&mpDB->mMysql,
						host.c_str(),
						user.c_str(),
						password.c_str(),
						database.c_str(),
						port,
						0, // UNIX SOCKET
						0 // FLAGS
						);
	
	if (!rv)
	{
		dblog.Error() <<"Failed to connect to database: " <<  mysql_error(&mpDB->mMysql) << endl;
		return false;
	}

	return true;
}

bool MysqlDatabase::UpdateDatabase()
{
	mpDB->mEntries.clear();

	string query = "SELECT ip, cookie, UNIX_TIMESTAMP(end) as timeout, prio FROM exp_sessions WHERE end > now()";
	int ip_col		= 0;
	int cookie_col	= 1;
	int timeout_col	= 2;
	int prio_col	= 3;

	int rv = mysql_real_query( &mpDB->mMysql, query.c_str(), query.size() );
	if (rv != 0)
	{
		int err = mysql_errno(&mpDB->mMysql);
		//dblog.Error() << "We have a db error: " << err << endl;
		if (err == CR_SERVER_LOST || err == CR_SERVER_GONE_ERROR)
		{
			dblog.Error() << "Lost database conenction, trying to reconnect" << endl;
			bool connected = Connect();
			if (!connected)
			{
				return false;
			}

			rv = mysql_real_query( &mpDB->mMysql, query.c_str(), query.size() );
			if (rv != 0) return false;
		}
		else
		{
			// mysql tries to recover from the error next time mysql_real_query is called
			dblog.Error() << "Database query failed: " << rv << " " << mysql_errno(&mpDB->mMysql) << " " << mysql_error(&mpDB->mMysql) << endl;
			return false;
		}
	}

	MYSQL_RES* result = mysql_store_result( &mpDB->mMysql );
	if (result)
	{
        MYSQL_ROW row;
		//unsigned int num_fields = mysql_num_fields(result);
		while ((row = mysql_fetch_row(result)) != NULL)
		{
			unsigned long *lengths = mysql_fetch_lengths(result);
			
			/*for(int i = 0; i < num_fields; i++)
			{
				printf("[%.*s] ", (int) lengths[i], row[i] ? row[i] : "NULL");
			}
			printf("\n");
			*/
			
			string ip(row[ip_col], lengths[ip_col]);
			string cookie(row[cookie_col], lengths[cookie_col]);
			string timeout(row[timeout_col], lengths[timeout_col]);
			string prio(row[prio_col], lengths[prio_col]);
			mpDB->mEntries.push_back( InternalDBEntry(cookie, ip, ToInt(timeout), ToInt(prio)) );
			//sysout << "OUT: " << ip << " " << cookie << endl;

		}

		mysql_free_result(result);
	}

	return true;
}

int MysqlDatabase::FetchAuthEntry(const ICredentials* pInCred, IAuthEntry* pOutAuthEntry, int cacheId)
{
	if (mCacheId != cacheId)
	{
		UpdateDatabase();
		mCacheId = cacheId;
	}

	std::string cookie = pInCred->GetCookie();

	for(InternalDB::tEntries::iterator it = mpDB->mEntries.begin(); it != mpDB->mEntries.end(); it++)
	{
		if (cookie == it->mCookie)
		{
			pOutAuthEntry->SetCookie(it->mCookie.c_str());
			pOutAuthEntry->SetIP(it->mIP.c_str());
			pOutAuthEntry->SetTimeout(it->mTimeout);
			pOutAuthEntry->SetPrio(it->mPrio);
			return 1;
		}
	}

	return 0;
}


/*DatabaseEntry* MysqlDatabase::FindCookie(const char* cookie)
{
	for(tEntries::iterator it = mEntries.begin(); it != mEntries.end(); it++)
	{
		if (string(cookie) == it->GetCookie()) return &(*it);
	}

	return NULL;
}*/

#if 0
bool MysqlDatabase::Authorize(std::string user, std::string cookie, std::string ip)
{
	// check if authorized
/*	string query = "select password from users, booking where users.id = booking.userid and start < now() and end > now() and name = '";
	//string query = "select password from users, booking where users.id = booking.userid and name = '";

	// escape user!
	char escuser[513];
	mysql_real_escape_string( &mDB->mMysql, escuser, name.c_str(), name.size());

	query += escuser;
	query += "'";
	
	// query

	string outpass;
	if (QueryPass(query, outpass))
	{
		// check if same password whas given
		if (outpass == password) return true;
	}
	else
	{
		// failed to query.. what should we do?
		// return false and try to recover
		// a problem with this aproch is that it takes a failed login for the database to recover
		Recover();
	}
*/
	return false;
}

/*bool MysqlDatabase::QueryPass(string query, string& outpass)
{
	outpass = "";
	int rv = mysql_real_query( &mDB->mMysql, query.c_str(), query.size() );

	if (rv == 0)
	{
		// success
		MYSQL_RES* result = mysql_store_result( &mDB->mMysql );
		if (result)
		{
			int num_fields = mysql_num_fields(result);

			if (num_fields > 0)
			{
				MYSQL_ROW row;
				if ((row = mysql_fetch_row(result)))
				{
					unsigned long *lengths;
					lengths = mysql_fetch_lengths(result);
					outpass = string(row[0], lengths[0]);
				}
			}
		}

		mysql_free_result(result);
		return true;
	}
	else
	{
		// something has happened
	}

	return false;
}

bool MysqlDatabase::Recover()
{
	return true;
}
*/

#endif
