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
 * All Rights Reserved.
 */

#pragma once
#ifndef __SERVICE_MAXLISTS_H__
#define __SERVICE_MAXLISTS_H__

#include <instruments/netlist2.h>
#include <instruments/listparser.h>

#include <list>
#include <string>

class InstrumentBlock;
class Service;

class MaxLists
{
public:
	bool	Init(
		const std::string& confBase,
		const std::string& maxListConfig,
		const std::string& saveLocation,
		const ListParser::tComponentDefinitions& compdefs
		);

	bool ReadConfig(std::string basedir, std::string filename);
	bool CheckAndValidate(InstrumentBlock* block);
	bool CircuitToNetlist(InstrumentBlock* block);

	bool	IsSubsetsOfComponentlist(const NetList2& componentlist);

	MaxLists();
	virtual ~MaxLists();
private:
	//std::string GetTokenValue(const std::string& name, const std::string& in) const;

	bool ReadMaxList(std::string filename);
	bool CheckMaxValues(InstrumentBlock* block, const NetList2& maxlist);

	typedef std::list<NetList2>	tMaxLists;
	tMaxLists mMaxLists;

	typedef std::list<std::string>	tListNames;
	tListNames mListNames;

	std::string mBaseDir;

	bool		mSaveCircuits;
	std::string	mSaveLocation;
	void		SaveCircuits(const std::string& circuit);
	
	ListParser::tComponentDefinitions mCompDefs;
};

#endif
