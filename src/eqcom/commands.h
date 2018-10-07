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

#ifndef __EQ_COMMANDS_H__
#define __EQ_COMMANDS_H__

//#include <ostream>
#include <iosfwd>

class Serializer;

class Oscilloscope;
class Trigger;
class Channel;
class Measurement;
class DigitalMultimeter;
class FunctionGenerator;
class TripleDC;
class NodeInterpreter;
class NetList2;

namespace EqSrv
{

/// Serializes requests to the measurement server
class InstrumentCommands
{
public:
	static void	OscilloscopeSetup(std::ostream& out, Oscilloscope& osc);
	static void	OscilloscopeFetch(std::ostream& out, Oscilloscope& osc);

	static void DigitalMultimeterFetch(std::ostream& out, DigitalMultimeter& dmm);

	static void FunctionGeneratorSetup(std::ostream& out, FunctionGenerator& funcgen);

	static void TripleDCSetup(std::ostream& out, TripleDC& tripledc, NodeInterpreter* pNodeIntr);
	static void TripleDCFetch(std::ostream& out, TripleDC& tripledc);
	static void CircuitFetch(std::ostream& out);

	static void ExtDelay(std::ostream& out, int timeinms);
	static void ExtResetAll(std::ostream& out);

private:
	static void OscilloscopeTriggerSetup(std::ostream& out, Trigger* trigger);
	static void OscilloscopeChannelSetup(std::ostream& out, Channel* chan);
	static void OscilloscopeMeasureSetup(std::ostream& out, Measurement* measure);
};

class InstrumentResponses
{
public:
	//static void OscilloscopeSetup(Serializer& in, Oscilloscope& osc);
	static void OscilloscopeFetch(Serializer& in, Oscilloscope& osc);
	static void OscilloscopeFetch(std::istream& in, Oscilloscope& osc);

	static void DigitalMultimeterFetch(Serializer& in, DigitalMultimeter& dmm);
	static void DigitalMultimeterFetch(std::istream& in, DigitalMultimeter& dmm);
	static void TripleDCFetch(Serializer& in, TripleDC& tripledc);
	static void TripleDCFetch(std::istream& in, TripleDC& tripledc);
	/*
	static void TripleDCSetup(Serializer& in, TripleDC& tripledc);

	static void CircuitFetch(Serializer& in, NetList2& resultlist);
	*/

};

} // end of namespace

#endif
