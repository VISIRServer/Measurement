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

#ifndef __EQ_RESPONSE_H__
#define __EQ_RESPONSE_H__

#include <instruments/listparser.h>

class NetList2;
class Serializer;
class InstrumentBlock;
class Service;

namespace EqSrv
{

class ResponseAdaptor
{
public:
	virtual void CircuitSetup(const NetList2& out) { NotHandled(); }
	virtual void NotHandled() {}
	virtual InstrumentBlock* GetBlock() { return 0; }
	virtual ~ResponseAdaptor() {}
};

class EquipmentServerResponse
{
public:
	static void ParseResponse(Serializer& in, ResponseAdaptor& adaptor, const ListParser::tComponentDefinitions* compdefs);
private:
	static void ParseData(Serializer& in, ResponseAdaptor& adaptor, const ListParser::tComponentDefinitions* compdefs);
	static void ParseError(Serializer& in, ResponseAdaptor& adaptor);

	static void HandleCircuit(Serializer& in, ResponseAdaptor& adaptor, const ListParser::tComponentDefinitions* compdefs);
	static void HandleMultimeter(Serializer& in, ResponseAdaptor& adaptor, int instrnr);
	static void HandleTripleDC(Serializer& in, ResponseAdaptor& adaptor, int instrnr);
	static void HandleFGen(Serializer& in, ResponseAdaptor& adaptor);
	static void HandleExtPeriph(Serializer& in, ResponseAdaptor& adaptor);
	static void HandleOscilloscope(Serializer& in, ResponseAdaptor& adaptor, int instrnr);
};

} // end of namespace

#endif
