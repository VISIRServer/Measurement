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

#ifndef __SOAP_RESPONSE_H__
#define __SOAP_RESPONSE_H__

#include <string>
#include <map>

namespace XMLUtil
{

class SoapParameter
{
public:
	const std::string&	GetValue() { return mValue; }
	void	SetValue(const std::string& value) { mValue = value; }

	// in case of struct constructs, we can have nested parameters
	SoapParameter& GetParam(std::string name)
	{
		return mParams[name];
	}
private:
	std::string mValue;

	typedef std::map< std::string , SoapParameter > tParams;
	tParams		mParams;
};

class SoapMethod
{
public:
	SoapParameter& GetParam(std::string name)
	{
		return mParams[name];
	}
	
	void SetName(const std::string& name) { mName = name; }
	const std::string& GetName() { return mName; }

private:
	typedef std::map< std::string , SoapParameter > tParams;

	tParams		mParams;
	std::string	mName;
};

/// Foamy
class SoapResponse
{
public:
	SoapMethod&		GetMethod() { return mMethod; }
	bool			IsFault();
	std::string		FaultSummary();
private:
	SoapMethod mMethod;
};

} // end of namespace

#endif