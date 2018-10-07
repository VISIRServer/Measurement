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

#ifndef __XML_COMMANDS_H__
#define __XML_COMMANDS_H__

//#include "xmldom.h"

#include <protocol/basic_types.h>
#include <xmlutil/domparser.h>

class InstrumentBlock;

// XXX: Search and replace

typedef XMLUtil::DOMNode	XmlDomNode;

namespace xmlprotocol
{

class XmlInstrumentCommand : public protocol::InstrumentCommand
{
public:
	virtual void SetData(XmlDomNode* pNode);
	virtual void ApplySettings(InstrumentBlock* pInstrument) {}

	XmlInstrumentCommand();
	virtual ~XmlInstrumentCommand();
protected:
	std::string	GetAttrValue(XmlDomNode* pNode);
	double		GetAttrValueDouble(XmlDomNode* pNode);
	int			GetAttrValueInt(XmlDomNode* pNode, bool throws);
	int			GetAttrValueInt(XmlDomNode* pNode, std::string attr, bool throws);

	XmlDomNode mDom;
};

class XmlInstrumentCommandFactory
{
public:
	static XmlInstrumentCommand* CreateFromDom(XmlDomNode* pNode);
};

class XmlFunctionGenerationCommand : public XmlInstrumentCommand
{
public:
	virtual Instrument::InstrumentType InstrumentType() { return Instrument::TYPE_FunctionGenerator; }
	virtual void ApplySettings(InstrumentBlock* pInstrument);
	virtual ~XmlFunctionGenerationCommand() {}
};

class XmlMultimeterCommand : public XmlInstrumentCommand
{
public:
	virtual Instrument::InstrumentType InstrumentType() { return Instrument::TYPE_DigitalMultimeter; }
	virtual void ApplySettings(InstrumentBlock* pInstrument);
	virtual ~XmlMultimeterCommand() {}
};

class XmlTripleDCCommand : public XmlInstrumentCommand
{
public:
	virtual Instrument::InstrumentType InstrumentType() { return Instrument::TYPE_TripleDC; }
	virtual void ApplySettings(InstrumentBlock* pInstrument);
	virtual ~XmlTripleDCCommand() {}
};

class XmlOscilloscopeCommand : public XmlInstrumentCommand
{
public:
	virtual Instrument::InstrumentType InstrumentType() { return Instrument::TYPE_Oscilloscope; }
	virtual void ApplySettings(InstrumentBlock* pInstrument);
	virtual ~XmlOscilloscopeCommand() {}
};

class XmlCircuitCommand : public XmlInstrumentCommand
{
public:
	virtual Instrument::InstrumentType InstrumentType() { return Instrument::TYPE_NodeInterpreter; }
	virtual void ApplySettings(InstrumentBlock* pInstrument);
	virtual ~XmlCircuitCommand() {}
};

class XmlSignalAnalyzerCommand : public XmlInstrumentCommand
{
public:
	virtual Instrument::InstrumentType InstrumentType() { return Instrument::TYPE_SignalAnalyzer; }
	virtual void ApplySettings(InstrumentBlock* pInstrument);
	virtual ~XmlSignalAnalyzerCommand() {}
};

} // end namespace
#endif
