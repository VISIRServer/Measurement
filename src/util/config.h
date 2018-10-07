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
 * Copyright (c) 2005-2009 Johan Zackrisson
 * All Rights Reserved.
 */

#pragma once
#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <map>
#include <string>

/// Simple configuration utility
/// Handles key -> value pairs, which can be read from commandline params
class Config
{
public:
	std::string	GetString(const std::string& key, std::string defaultval = "") const;
	int			GetInt(const std::string& key, int defaultval = 0) const;

	void	SetString(const std::string& key, const std::string& value);

	void	ParseParams(int argc, char** argv);
	bool	ParseFile(std::string filename);

			Config();
	virtual ~Config();
private:
	typedef std::map<std::string, std::string> tConfig;
	tConfig		mConfig;
};

#endif
