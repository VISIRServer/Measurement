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
 * Copyright (c) 2007-2009 Johan Zackrisson
 * Copyright (c) 2008 André van Schoubroeck
 * All Rights Reserved.
 */

#include "maxlists.h"
#include <instruments/instrumentblock.h>
#include <instruments/nodeinterpreter.h>
#include <instruments/circuitlist.h>
#include <instruments/listparser.h>

#include <contrib/md5.h>

#include <syslog.h>
#include <basic_exception.h>
#include <stringop.h>
#include <timer.h>

#include <fstream>
#include <math.h>
#include <sstream>

MaxLists::MaxLists()
{
	mSaveCircuits = false;
	mSaveLocation = "";
}

MaxLists::~MaxLists()
{
}

bool MaxLists::Init(const std::string& confBase, const std::string& maxListConfig, const std::string& saveLocation, const ListParser::tComponentDefinitions& compdefs)
{
	mCompDefs = compdefs; // copy the component definitions
	mSaveLocation = saveLocation;
	if (mSaveLocation != "") mSaveCircuits = true;

	if (!ReadConfig(confBase, maxListConfig)) return false;
	return true;
}

bool MaxLists::ReadConfig(std::string basedir, std::string filename)
{
	sysout << "[+] Reading maxlist config" << std::endl;
	mBaseDir = basedir;
	std::fstream file((mBaseDir + filename).c_str());
	if (!file.is_open())
	{
		syserr << "Can't find maxlist: " << filename << std::endl;
		return false;
	}

	while(!file.eof())
	{
		std::string str;
		getline(file,str);

		if (!ReadMaxList(str))
		{
			syserr << "failed on : " << str << std::endl;
			return false;
		}
	}

	return true;
}

bool MaxLists::ReadMaxList(std::string filename)
{
	if (filename.empty()) return true;
	if ((filename[0] == '#') || (filename[0] == '*')) return true;

	sysout << "[+] Reading maxlist: " << filename << std::endl;

	NetList2 aNetList;
	try
	{
		ListParser parser(mCompDefs);
		if (!parser.ParseFile(mBaseDir + filename))
		{
			syserr << "Failed to parse maxlist: " << filename << std::endl;
			return false;
		}

		aNetList.SetNodeList(parser.GetList());
	}
	catch(BasicException e)
	{
		syserr << "MaxLists::ReadMaxLists(): input maxlist is corrupt.." << std::endl;
		syserr << e.what() << std::endl;
		return false;
	}

	mMaxLists.push_back(aNetList);
	mListNames.push_back(filename); // XXX: Store in a pair instead
	return true;
}

bool MaxLists::CheckAndValidate(InstrumentBlock* block)
{
	const NetList2& netlist = block->GetNodeInterpreter()->GetNetList();

	for(tMaxLists::const_iterator it = mMaxLists.begin(); it != mMaxLists.end(); ++it)
	{
		if (netlist.IsSubsetOf(*it))
		{
			if (CheckMaxValues(block, *it)) return true;
			else syslog << "Some limits exceeded" << std::endl;
		}
	}

	syserr << "MaxLists::CheckSubset(): netlist is not a subset of any maxlist" << std::endl;
	syserr << netlist.GetNetListAsString() << std::endl;

	return false;
}

bool MaxLists::CheckMaxValues(InstrumentBlock* pBlock, const NetList2& maxlist)
{
	InstrumentBlock::tInstruments instruments = pBlock->GetInstruments();
	for(InstrumentBlock::tInstruments::const_iterator it = instruments.begin(); it != instruments.end(); ++it)
	{
		if (!(*it)->CheckMaxValues(maxlist)) return false;
	}

	return true;
}

bool MaxLists::CircuitToNetlist(InstrumentBlock* block)
{
	if (block->GetNodeInterpreter()->GetCircuitList().empty())
	{
		// don't bother.. just reset the "solved" netlist
		block->GetNodeInterpreter()->SetNetList(NetList2());
		return true;
	}

	ListParser circuitparser(mCompDefs);
	try
	{
		if (!circuitparser.Parse(block->GetNodeInterpreter()->GetCircuitList()))
		{
			syserr << "Failed to parse input circuit" << std::endl;
			return false;
		}
	}
	catch(ValidationException e)
	{
		block->GetNodeInterpreter()->SetNetList(NetList2());
		return false;
	}

	if (mSaveCircuits) SaveCircuits(block->GetNodeInterpreter()->GetCircuitList());

	timer circuittimer;

	tListNames::const_iterator nameit = mListNames.begin();

	for(tMaxLists::const_iterator it = mMaxLists.begin(); it != mMaxLists.end(); ++it)
	{
		// this requires the fgen and tripledc to be set up properly..
		// may be that they are not yet validated.. can that be a problem?
		if (!CheckMaxValues(block, *it))
		{
			syslog << "Limits exceeded, skipping: " << *nameit << std::endl;
		}
		else
		{
			CircuitList aList;
			if (aList.Solve(circuitparser.GetList(), it->GetNodeList()))
			{
				NetList2 solvednetlist;
				solvednetlist.SetNodeList(aList.GetSolution());

				// ugly fix.. this should be done inside the solver
				// the problem is that instrument nodes can be introduces that is not allowed
				if (!solvednetlist.IsSubsetOf(*it))
				{
					LogLevel(sysout, 4) << "solved but not a subset of: " << *nameit << std::endl;
					continue;
				}

				LogLevel(sysout, 4) << "Matching maxlist: " << *nameit << std::endl;
				LogLevel(sysout, 4) << "Solved list is:" << std::endl << solvednetlist.GetNetListAsString() << std::endl;

				block->GetNodeInterpreter()->SetNetList(solvednetlist);
				LogLevel(timerlog, 4) << timestamp << "MaxLists::CircuitToNetlist solved after: " << circuittimer.elapsed() << std::endl;
				return true;
			}
		}

		++nameit;
	}

	syslog << "MaxLists::CircuitToNetlist failed to solve after: " << circuittimer.elapsed() << std::endl;

	return false;
}

bool MaxLists::IsSubsetsOfComponentlist(const NetList2& componentlist)
{
	bool rv = true;
	tListNames::const_iterator nameit = mListNames.begin();
	for(tMaxLists::const_iterator it = mMaxLists.begin(); it != mMaxLists.end(); ++it)
	{
		ListComponent::tComponentList failed;
		if (!it->IsSubsetOfWithFailed(componentlist, failed))
		{
			syserr << "WARNING: Maxlist " << *nameit << " is not a subset of the componentlist" << std::endl;
			for(ListComponent::tComponentList::const_iterator dumpit = failed.begin(); dumpit != failed.end(); ++dumpit)
			{
				syserr << "Failed on component: " << dumpit->Dump() << std::endl;
			}
			rv = false;
		}

		++nameit;
	}

	return rv;
}

void MaxLists::SaveCircuits(const std::string& circuit)
{
	unsigned char md5sum[16];
	md5((unsigned char*)circuit.c_str(), circuit.size(), md5sum);

	char output[33];
	for( int i = 0; i < 16; ++i )
	{
		sprintf(&output[i*2] , "%02x", md5sum[i]);
	}
	output[32] = '\0';

	LogLevel(sysout, 3) << "Circuit checksum: " << output << std::endl;

	std::string filename = mSaveLocation + output + ".circuit";
	FILE* testfile = fopen(filename.c_str(), "r");
	if (testfile != NULL)
	{
		fclose(testfile);
		return; // file does already exist, we don't have to write the data
	}

	LogLevel(sysout, 3) << "Saving to file: " << filename << std::endl;

	FILE* savefile = fopen(filename.c_str(), "w");
	if (!savefile)
	{
		syserr << "Failed to write circuit to file" << std::endl;
		return;
	}

	fwrite(circuit.c_str(), circuit.size(), 1, savefile);

	fclose(savefile);
}