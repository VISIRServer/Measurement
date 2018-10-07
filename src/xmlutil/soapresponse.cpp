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

#include "soapresponse.h"

#include <sstream>

#define SOAP_NS "http://www.w3.org/2003/05/soap-envelope"

using namespace XMLUtil;

bool SoapResponse::IsFault()
{
	if (GetMethod().GetName() == SOAP_NS "#Fault") return true;
	return false;
}

std::string SoapResponse::FaultSummary()
{
	std::string code = GetMethod().GetParam(SOAP_NS "#Code").GetParam( SOAP_NS "#Value").GetValue();
	std::string reason = GetMethod().GetParam(SOAP_NS "#Reason").GetParam( SOAP_NS "#Text").GetValue();

	std::stringstream out;
	out << code << " (" << reason << ")";

	return out.str();
}