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

#include <instruments/instrumentblock.h>

#include <instruments/nodeinterpreter.h>
#include <instruments/oscilloscope.h>

#include <instruments/listparser.h>

#include "experiment.h"
#include "circuit.h"
#include "commands.h"

#include <usbmatrix/matrix.h>

#include "ividrivers.h"
#include "USB/cUSB.h"
#include "doerror.h"

#include <windows.h>

using namespace IVIControl;

#define SYSTEM_TRANSIENT_DELAY 25
#define SWITCH_DELAY 10

typedef InstrumentBlock::tInstruments tInstruments;

// combined setup for measurement and sources
class MeasureSetupVisitor : public InstrumentVisitor
{
public:
	MeasureSetupVisitor(InstrumentBlock* pBlock, Drivers* pDrivers) : mpBlock(pBlock) , mCommands(pDrivers)
	{
	}

	virtual void Visit(Oscilloscope& osc)
	{
		mCommands.OscilloscopeSetup(osc);
	}

	virtual void Visit(DigitalMultimeter& dmm)
	{
		mCommands.DigitalMultimeterFetch(dmm);
	}
private:
	InstrumentBlock*	mpBlock;
	InstrumentCommands	mCommands;
};

class SourceSetupVisitor : public InstrumentVisitor
{
public:
	SourceSetupVisitor(InstrumentBlock* pBlock, Drivers* pDrivers) : mpBlock(pBlock) , mCommands(pDrivers)
	{
	}

	virtual void Visit(FunctionGenerator& funcgen)
	{
		mCommands.FunctionGeneratorSetup(funcgen);
	}
	
	virtual void Visit(TripleDC& tripledc)
	{
		mCommands.TripleDCSetup(tripledc, mpBlock->GetNodeInterpreter());
	}

private:
	InstrumentBlock*	mpBlock;
	InstrumentCommands	mCommands;
};

class FetchVisitor : public InstrumentVisitor
{
public:
	FetchVisitor(Drivers* pDrivers) : mCommands(pDrivers)
	{
	}

	virtual void Visit(Oscilloscope& osc)
	{
		mCommands.OscilloscopeFetch(osc);
	}

	virtual void Visit(TripleDC& tripledc)
	{
		mCommands.TripleDCFetch(tripledc);
	}
private:
	InstrumentCommands	mCommands;
};

void Experiment::DoExperiment(InstrumentBlock* pBlock, Drivers* pDrivers, ListParser* pListParser, USBMatrix::Matrix* pMatrix)
{
	bool matrixEnabled = pDrivers->MatrixEnabled();

	SourceSetupVisitor sourceSetupVisitor(pBlock, pDrivers);
	MeasureSetupVisitor measureSetupVisitor(pBlock, pDrivers);	

	if (matrixEnabled)
	{
		Circuit::tCardCompList pairs;
		Circuit::tComponentList circuit =  pBlock->GetNodeInterpreter()->GetNetList().GetNodeList(); 
		Circuit::tComponentList components = pListParser->GetList();

		Circuit::BuildCircuitSetup(circuit, components, pairs);
		Circuit::BuildInstrumentSetup(pBlock, pairs);

		pMatrix->SetupCircuit(pairs);
	}

	NodeInterpreter* pNodeIntr = pBlock->GetNodeInterpreter();

	// sources setup
	tInstruments sources = pBlock->GetSources();
	for(tInstruments::iterator i = sources.begin(); i != sources.end(); i++)
	{
		if (!matrixEnabled || pNodeIntr->((*i)->GetType()) )
		{
			(*i)->Accept(sourceSetupVisitor);
		}
	}

	//Sleep(SYSTEM_TRANSIENT_DELAY);

	// measurement setup
	tInstruments measureEq = pBlock->GetMeasureEq();
	for(tInstruments::iterator i = measureEq.begin(); i != measureEq.end(); i++)
	{
		if (!matrixEnabled || pNodeIntr->((*i)->GetType()) )
		{
			(*i)->Accept(measureSetupVisitor);
		}
	}

	if (matrixEnabled)
	{
		bool hasSwitches = pBlock->GetNodeInterpreter()->ContainsSwitches();
		if (hasSwitches)
		{
			// switch delay
			Sleep(SWITCH_DELAY);

			// flip switches
			Circuit::tCardCompList pairs;
			Circuit::tComponentList circuit =  pBlock->GetNodeInterpreter()->GetNetList().GetNodeList(); 
			Circuit::tComponentList components = pListParser->GetList();
			Circuit::BuildCircuitSwitch(circuit, components, pairs);
			pMatrix->FlipSwitches(pairs);
		}
	}

	// fetch
	FetchVisitor fetchVisitor(pDrivers);
	for(tInstruments::iterator i = measureEq.begin(); i != measureEq.end(); i++)
	{
		if (!matrixEnabled || pNodeIntr->((*i)->GetType()) )
		{
			(*i)->Accept(fetchVisitor);
		}
		else
		{
			(*i)->ResetData();
		}
	}

	if (matrixEnabled)
	{
		pMatrix->ReleaseSources();
	}
}
