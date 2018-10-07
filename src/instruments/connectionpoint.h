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

#pragma once
#ifndef __CONNECTION_POINT_H__
#define __CONNECTION_POINT_H__

#include <string>

/// ConnectionPoint handles instrument connections to the circuit.
/// The point is represented as an integer, where -1 is NC (not connected), 0 is ground, 1 is point A and so on.
class ConnectionPoint
{
public:
	/// Converts a pointstring to pointnumber representation
	static int		ToPointNumber(const std::string& point);
	int		ToPointNumber() const;

	/// Assigns the connection.
	///	Assigns a connection to a point, if the input data is invalid, the point is set to NC (not connected)
	//void	Assign(std::string point);

	/// returns the point as a string
	const std::string	GetPointString() const;

	const bool IsConnected() const;

	//ConnectionPoint&	operator=(std::string a);

	/// ctor
	ConnectionPoint();

	/// ctor
	ConnectionPoint(const ConnectionPoint& other);
	/// ctor
	ConnectionPoint(const std::string& point);
	/// dtor	
	~ConnectionPoint();
private:
	std::string mPoint;
};

// comparison operator needed for stl containers
inline bool operator<(const ConnectionPoint& p1, const ConnectionPoint& p2)
{
	return p1.GetPointString() < p2.GetPointString();
}

#endif
