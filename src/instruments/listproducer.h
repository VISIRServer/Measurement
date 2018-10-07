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

#ifndef __LIST_PRODUCER_H__
#define __LIST_PRODUCER_H__

#include "listcomponent.h"
#include <vector>
#include <string>

class InstrumentBlock;

/// Converts a listcomponent list to string
class ListProducer
{
public:
	typedef std::vector<ListComponent>	tComponentList;

	static std::string	Produce(const tComponentList& list, std::string linesep = "\n");
	static std::string	ProduceComponent(const ListComponent& comp);
	static std::string	ProduceInstruments(InstrumentBlock* block, std::string linesep = "\n");
};

#endif
