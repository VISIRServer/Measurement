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

#include "experiment.h"
#include "circuit.h"
#include "commands.h"

#include <instruments/instrumentblock.h>

#include <instruments/nodeinterpreter.h>
#include <instruments/oscilloscope.h>

#include <sstream>
#include <locale>

using namespace EqSrv;

#define SYSTEM_TRANSIENT_DELAY 25
#define SWITCH_DELAY 10

typedef InstrumentBlock::tInstruments tInstruments;

// combined setup for measurement and sources
class MeasureSetupVisitor : public InstrumentVisitor
{
public:
	MeasureSetupVisitor(std::ostream& out, InstrumentBlock* pBlock) : mRefStream(out), mpBlock(pBlock) {}

	virtual void Visit(Oscilloscope& osc)
	{
		InstrumentCommands::OscilloscopeSetup(mRefStream, osc);
	}

	virtual void Visit(DigitalMultimeter& dmm)
	{
		InstrumentCommands::DigitalMultimeterFetch(mRefStream, dmm);
	}
private:
	std::ostream&		mRefStream;
	InstrumentBlock*	mpBlock;
};

class SourceSetupVisitor : public InstrumentVisitor
{
public:
	SourceSetupVisitor(std::ostream& out, InstrumentBlock* pBlock) : mRefStream(out), mpBlock(pBlock) {}

	virtual void Visit(FunctionGenerator& funcgen)
	{
		InstrumentCommands::FunctionGeneratorSetup(mRefStream, funcgen);
	}
	
	virtual void Visit(TripleDC& tripledc)
	{
		InstrumentCommands::TripleDCSetup(mRefStream, tripledc, mpBlock->GetNodeInterpreter());
	}

private:
	std::ostream&		mRefStream;
	InstrumentBlock*	mpBlock;
};

class FetchVisitor : public InstrumentVisitor
{
public:
	FetchVisitor(std::ostream& out) : mRefStream(out) {}

	virtual void Visit(Oscilloscope& osc)
	{
		InstrumentCommands::OscilloscopeFetch(mRefStream, osc);
	}

	virtual void Visit(TripleDC& tripledc)
	{
		InstrumentCommands::TripleDCFetch(mRefStream, tripledc);
	}
private:
	std::ostream&		mRefStream;
};

void Experiment::BuildExperiment(std::ostream& out, InstrumentBlock* pBlock, NetList2& lookup, bool noMatrix)
{
	out.imbue(std::locale::classic());

	SourceSetupVisitor sourceSetupVisitor(out, pBlock);
	MeasureSetupVisitor measureSetupVisitor(out, pBlock);	

	if (!noMatrix)
	{
		Circuit::BuildCircuitSetup(out, pBlock, lookup);
		Circuit::BuildInstrumentSetup2(out, pBlock);
	}

	NodeInterpreter* pNodeIntr = pBlock->GetNodeInterpreter();

	// sources setup
	tInstruments sources = pBlock->GetSources();
	for(tInstruments::iterator i = sources.begin(); i != sources.end(); i++)
	{
		if (noMatrix || pNodeIntr->((*i)->GetType()) )
		{
			(*i)->Accept(sourceSetupVisitor);
		}
	}

	InstrumentCommands::ExtDelay(out, SYSTEM_TRANSIENT_DELAY);

	// measurement setup, ARM
	tInstruments measureEq = pBlock->GetMeasureEq();
	for(tInstruments::iterator i = measureEq.begin(); i != measureEq.end(); i++)
	{
		if (noMatrix || pNodeIntr->((*i)->GetType()) )
		{
			(*i)->Accept(measureSetupVisitor);
		}
	}

	bool hasSwitches = pBlock->GetNodeInterpreter()->ContainsSwitches();
	if (hasSwitches && !noMatrix)
	{
		// switch delay
		InstrumentCommands::ExtDelay(out, SWITCH_DELAY);

		// switch change
		Circuit::BuildCircuitSwitch(out, pBlock, lookup);
	}

	// fetch
	FetchVisitor fetchVisitor(out);
	for(tInstruments::iterator i = measureEq.begin(); i != measureEq.end(); i++)
	{
		if (noMatrix || pNodeIntr->((*i)->GetType()) )
		{
			(*i)->Accept(fetchVisitor);
		}
		else
		{
			(*i)->ResetData();
		}
	}
}
