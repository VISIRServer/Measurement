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
 * Copyright (c) 2004-2009 Johan Zackrisson
 * All Rights Reserved.
 */

#include "connectionpoint.h"
#include <stringop.h>

#include <basic_exception.h>

ConnectionPoint::ConnectionPoint()
{
	mPoint = "NC";
}

ConnectionPoint::ConnectionPoint(const ConnectionPoint& other)
{
	mPoint = other.mPoint;
}

ConnectionPoint::ConnectionPoint(const std::string& point)
{
	mPoint = point;
}

ConnectionPoint::~ConnectionPoint()
{
}

/*void ConnectionPoint::Assign(string point)
{
	mPoint = ToPointNumber(point);
}

ConnectionPoint& ConnectionPoint::operator=(string a)
{
	Assign(a);
	return *this;
}*/

const std::string ConnectionPoint::GetPointString() const
{
	return mPoint;
}

int ConnectionPoint::ToPointNumber() const
{
	return ToPointNumber(mPoint);
}

int ConnectionPoint::ToPointNumber(const std::string& pointstr)
{
	std::string point = ToUpper(pointstr);

	if (point == "NC") return -1;
	if (point == "GND" || point == "0") return 0;
	
	if (point.size() == 1)
	{
		char c = point[0];
		int out = -1;
		if (c >= 'A' && c <= 'Z') out = c - 'A' + 1;
		if (c >= '1' && c <= '9') out = c - '1' + 1;

		return out;
	}

	return -2;
}

const bool ConnectionPoint::IsConnected() const
{
	if (mPoint == "NC" || mPoint == "") return false;
	return true;
}
