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

#include "widestring.h"

#include <vector>

#include <windows.h>

using namespace std;

void Ascii2Wide(const std::string& data, std::wstring& out)
{
	// returns the length or 0 if unsuccessful
	int rv = MultiByteToWideChar(CP_ACP, 0, data.c_str(), data.size(), NULL, 0);
	if (rv == 0)
	{
		out = L"";
		return; // error code?
	}

	std::vector<TCHAR> wcdata(rv);
	rv = MultiByteToWideChar(CP_ACP, 0, data.c_str(), data.size(), &wcdata[0], wcdata.size());
	if (rv == 0)
	{
		out = L"";
		return;
	}

	out.insert(out.end(), wcdata.begin(), wcdata.end());
}

void Wide2Ascii(const std::wstring& data, std::string& out)
{
	// returns the length or 0 if unsuccessful
	int rv = WideCharToMultiByte(CP_ACP, 0, data.c_str(), data.size(), NULL, 0, NULL, NULL);
	if (rv == 0)
	{
		out = "";
		return;
	}

	std::vector<char> cdata(rv);
	rv = WideCharToMultiByte(CP_ACP, 0, data.c_str(), data.size(), &cdata[0], cdata.size(), NULL, NULL);
	if (rv == 0)
	{
		out = "";
		return;
	}

	out.insert(out.end(), cdata.begin(), cdata.end());
}

std::wstring Ascii2Wide(const std::string& data)
{
	std::wstring out;
	Ascii2Wide(data, out);
	return out;
}

std::string Wide2Ascii(const std::wstring& data)
{
	std::string out;
	Wide2Ascii(data, out);
	return out;
}
