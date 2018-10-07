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
 * Copyright (c) 2008 André van Schoubroeck
 * All Rights Reserved.
 */

#include "response.h"
#include "commands.h"
#include "eqlog.h"

#include <instruments/netlist2.h>
#include <instruments/listparser.h>

#include <instruments/instrumentblock.h>
#include <instruments/digitalmultimeter.h>
#include <instruments/tripledc.h>
#include <instruments/oscilloscope.h>

#include <serializer.h>
#include <basic_exception.h>
#include <stringop.h>

using namespace EqSrv;
using namespace std;

void EquipmentServerResponse::ParseResponse(Serializer& in, ResponseAdaptor& adaptor, const ListParser::tComponentDefinitions* compdefs)
{
	string typestr;
	in.GetString(typestr, "\n");

	if ("data" == typestr) ParseData(in, adaptor, compdefs);
	else if ("error" == typestr) ParseError(in, adaptor);
	else
	{
		throw BasicException("Unhandled packet type");
	}
}

void EquipmentServerResponse::ParseData(Serializer& in, ResponseAdaptor& adaptor, const ListParser::tComponentDefinitions* compdefs)
{
	while(!in.EndOfStream())
	{
		int id = -1;

		//in.GetInteger(id, "# \t");

		int instrid = 1;

		string type_id;
		in.GetString(type_id, " \t");
		size_t p = type_id.find_first_of("#");
		if (p == string::npos)
		{
			id = ToInt(type_id);
		}
		else
		{
			id = ToInt(string(type_id, 0, p));
			instrid = ToInt(string(type_id, p+1));
		}

		switch(id)
		{
		case 11:	HandleFGen(in, adaptor);					break;
		case 12:	HandleTripleDC(in, adaptor, instrid);		break;
		case 21:	HandleOscilloscope(in, adaptor, instrid);	break;
		case 22:	HandleMultimeter(in, adaptor, instrid);		break;
		case 31:	HandleExtPeriph(in, adaptor);				break;
		case 41:	HandleCircuit(in, adaptor, compdefs);		break;
		default:
			throw BasicException("Unhandled response");
		}
	}
}

void EquipmentServerResponse::ParseError(Serializer& in, ResponseAdaptor& adaptor)
{
	string errorstr;
	errorstr = in.GetCStream();
	//in.GetString(errorstr, "\n");
	throw BasicException(errorstr);
}

void EquipmentServerResponse::HandleCircuit(Serializer& in, ResponseAdaptor& adaptor, const ListParser::tComponentDefinitions* compdefs)
{
	int func = -1;
	in.GetInteger(func, " \t");
	switch(func)
	{
		// cases where we don't have any parameters
	case 0:
	case 1:
	case 3:
	case 5:
	case 6:
	case 7:
		break;

	case 4:
		{
			if (compdefs == NULL) throw BasicException("Equipment returned unexpected circuit information");
			std::string circuit;
			in.GetString(circuit, "\n");
			
			Serializer ser(circuit);
			std::string strlist;
			std::string line;
			while(!ser.EndOfStream())
			{
				ser.GetString(line, "?\n");
				strlist += line;
				strlist += "\n";
			}

			ListParser parser(*compdefs);

			if (!parser.Parse(strlist))
			{
				eqlog.Error() << "Equipment server returned invalid netlist" << endl;
				NetList2 empty;
				adaptor.CircuitSetup(NetList2());
				throw BasicException("Equipment server returned invalid netlist");
			}
			else
			{
				adaptor.CircuitSetup(NetList2(parser.GetList()));
			}
		}
		break;
	default:
		throw BasicException("Unhandled response in circuitbuilder");
	}
}

void EquipmentServerResponse::HandleMultimeter(Serializer& in, ResponseAdaptor& adaptor, int instrnr)
{
	//DigitalMultimeter* pDmm = (DigitalMultimeter*) adaptor.GetBlock()->AcquireInstrument(Instrument::TYPE_DigitalMultimeter);
	DigitalMultimeter* pDmm = adaptor.GetBlock()->Get<DigitalMultimeter>(instrnr);
	if (!pDmm) throw BasicException("Result for non existing dmm");

	InstrumentResponses::DigitalMultimeterFetch(in, *pDmm);
}

void EquipmentServerResponse::HandleTripleDC(Serializer& in, ResponseAdaptor& adaptor, int instrnr)
{
	int func = -1;
	in.GetInteger(func, " \t");

	if (func != 0)
	{
		//TripleDC * pTdc= (TripleDC *)adaptor.GetBlock()->AcquireInstrument(Instrument::TYPE_TripleDC);
		TripleDC * pTdc= adaptor.GetBlock()->Get<TripleDC>(instrnr);
		if (!pTdc) throw BasicException("Result from non existing TripleDC");
		InstrumentResponses::TripleDCFetch(in,*pTdc);
	}
}

void EquipmentServerResponse::HandleFGen(Serializer& in, ResponseAdaptor& adaptor)
{
	int func = -1;
	in.GetInteger(func, " \t");

	if (func != 0) throw BasicException("Unhandled function generator result");
}

void EquipmentServerResponse::HandleExtPeriph(Serializer& in, ResponseAdaptor& adaptor)
{
	int func = -1;
	in.GetInteger(func, " \t");

	if (func != 0) throw BasicException("Unhandled extended peripherals result");
}

void EquipmentServerResponse::HandleOscilloscope(Serializer& in, ResponseAdaptor& adaptor, int instrnr)
{
	// for now, the commands are handled by v3measure commands.. this is not optimal
	//Oscilloscope* pOsc = (Oscilloscope*) adaptor.GetBlock()->AcquireInstrument(Instrument::TYPE_Oscilloscope);
	Oscilloscope* pOsc = adaptor.GetBlock()->Get<Oscilloscope>(instrnr);
	if (!pOsc) throw BasicException("Result for non existing oscilloscope");
	InstrumentResponses::OscilloscopeFetch(in, *pOsc);

	/*
	int func = -1;
	in.GetInteger(func, " \t");

	switch(func)
	{
	case 0: break;
	case 1:
		{
			Oscilloscope* pOsc = (Oscilloscope*) adaptor.GetBlock()->AcquireInstrument(Instrument::TYPE_Oscilloscope);
			if (!pOsc) throw BasicException("Result for non existing oscilloscope");
			V3Measure::InstrumentResponses::OscilloscopeFetch(in, *pOsc);
		} break;
	default: throw BasicException("Unhandled oscilloscope result");
	}
	*/
}
