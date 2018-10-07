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

#include "validate.h"
#include "instrumentblock.h"
#include "nodeinterpreter.h"

#include <basic_exception.h>

typedef InstrumentBlock::tInstruments tInstruments;

bool Validator::ValidateBlock(InstrumentBlock* block)
{
	if (!block) return false;

	if (!block->GetNodeInterpreter()->Validate())
	{
		return false;
	}

	tInstruments sources	= block->GetSources();
	tInstruments measureEq	= block->GetMeasureEq();

	for(tInstruments::const_iterator srcit = sources.begin(); srcit != sources.end(); srcit++)
	{
		if ( !(*srcit)->Validate() )
		{
			return false;
		}
	}

	for(tInstruments::const_iterator meas = measureEq.begin(); meas != measureEq.end(); meas++)
	{
		if ( !(*meas)->Validate() )
		{
			return false;
		}
	}

	// check if we have any measurement requests to include.. if not something is wrong..
	if (measureEq.empty())
	{
		// hmm.. no measuments? throw exception
		throw ValidationException("No measurements requested");
		return false; // not reached..
	}

	return true;
}
