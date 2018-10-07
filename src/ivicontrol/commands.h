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
 * Copyright (c) 2008 Johan Zackrisson
 * Copyright (c) 2008 André van Schoubroeck
 * All Rights Reserved.
 */

#ifndef __IVI_COMMANDS_H__
#define __IVI_COMMANDS_H__

class Oscilloscope;
class Trigger;
class Channel;
class Measurement;
class DigitalMultimeter;
class FunctionGenerator;
class TripleDC;
class NodeInterpreter;
class NetList2;

namespace IVIControl
{

class Drivers;

/// Serializes requests to the measurement server
class InstrumentCommands
{
public:
	InstrumentCommands(Drivers* pDrivers);

	void	OscilloscopeSetup(Oscilloscope& osc);
	void	OscilloscopeFetch(Oscilloscope& osc);

	void DigitalMultimeterFetch(DigitalMultimeter& dmm);

	void FunctionGeneratorSetup(FunctionGenerator& funcgen);

	void TripleDCSetup(TripleDC& tripledc, NodeInterpreter* pNodeIntr);
	void TripleDCFetch(TripleDC& tripledc);

private:
	void OscilloscopeTriggerSetup(Trigger* trigger);
	void OscilloscopeChannelSetup(Channel* chan);
	void OscilloscopeMeasureSetup(Measurement* measure);

	Drivers* mpDrivers;
};

class InstrumentResponses
{
public:
	InstrumentResponses(Drivers* pDrivers);

	static void OscilloscopeFetch(Oscilloscope& osc);
	static void DigitalMultimeterFetch(DigitalMultimeter& dmm);
	static void TripleDCFetch(TripleDC& tripledc);

private:
	Drivers* mpDrivers;
};

} // end of namespace

#endif
