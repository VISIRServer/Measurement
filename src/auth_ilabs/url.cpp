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
 * Copyright (c) 2009 Johan Zackrisson
 * All Rights Reserved.
 */

#include "url.h"

#include <Wtypes.h>
#include <WinHttp.h>

using namespace std;

URL::URL()
{
}

URL::~URL()
{
}

int URL::Set(std::wstring url)
{
	mURL = url;

	URL_COMPONENTS urlComp;
	ZeroMemory(&urlComp, sizeof(URL_COMPONENTS));
    urlComp.dwStructSize = sizeof(URL_COMPONENTS);
	urlComp.dwSchemeLength    = -1;
	urlComp.dwHostNameLength  = -1;
	urlComp.dwUrlPathLength   = -1;
	urlComp.dwExtraInfoLength = -1;

    if (!WinHttpCrackUrl( mURL.c_str(), mURL.size(), 0, &urlComp))
    {
		return 0;
    }

	mSchema		= wstring(urlComp.lpszScheme, urlComp.dwSchemeLength);
	mHostName	= wstring(urlComp.lpszHostName, urlComp.dwHostNameLength);
	mPath		= wstring(urlComp.lpszUrlPath, urlComp.dwUrlPathLength);

	return 1;
}

