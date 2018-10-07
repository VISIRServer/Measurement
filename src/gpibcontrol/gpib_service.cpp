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

#include "gpib_service.h"
#include "gpib_signalanalyzer.h"
#include "gpib_transactions.h"

#include <config.h>
#include <stringop.h>
#include <syslog.h>

using namespace std;
using namespace gpib;

GPIBService::GPIBService(Config* pConfig)
	: mpConfig(pConfig)
{
	mpSignalAnalyzer = new gpib::GPIBSignalAnalyzer();
	mpHandler = new GPIBTransactionHandler();
}

GPIBService::~GPIBService()
{
	delete mpHandler;
	delete mpSignalAnalyzer;
}

bool GPIBService::Init()
{
	string signaddr	= mpConfig->GetString("GPIB.SignalAnalyser", "0:11:0");
	int boardnr = ToInt(Token(signaddr,0,":"));
	int prim	= ToInt(Token(signaddr,1,":"));
	int sec		= 0;
	if (!mpSignalAnalyzer->Init(boardnr, prim, sec))
	{
		syserr << timestamp << "*** Failed to initialize GPIB Signal Analyzer" << endl;
		return false;
	}

	mpHandler->Init(mpSignalAnalyzer);

	sysout << "[+] GPIB Signal Analyzer Initialized" << endl;

	return true;
}

bool GPIBService::Tick()
{
	return mpSignalAnalyzer->Tick();
}


GPIBTransactionHandler* GPIBService::GetTransactionHandler()
{
	return mpHandler;
}