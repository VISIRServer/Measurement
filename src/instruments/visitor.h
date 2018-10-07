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

 // visitor en python no es necesario es virtual de por si
 
#ifndef __INSTRUMENT_VISITOR_H__
#define __INSTRUMENT_VISITOR_H__

class Oscilloscope;
class DigitalMultimeter;
class FunctionGenerator;
class NodeInterpreter;
class TripleDC;
class SignalAnalyzer;

/// Handles instrument generalisation without the need to downcast.
/// Must handle all available instrument classes.

class InstrumentVisitor
{
public:
	virtual void Visit(Oscilloscope&		) {}
	virtual void Visit(DigitalMultimeter&	) {}
	virtual void Visit(FunctionGenerator&	) {}
	virtual void Visit(NodeInterpreter&		) {}
	virtual void Visit(TripleDC&			) {}
	virtual void Visit(SignalAnalyzer&		) {}
	
	virtual ~InstrumentVisitor() {}
};

#endif
