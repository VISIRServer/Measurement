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
 * All Rights Reserved.
 */

#pragma once
#ifndef __MODULE_H__
#define __MODULE_H__

#include <instruments/listparser.h>

class Config;
class InstrumentBlock;
class IAuthenticator;
class NetList2;

namespace protocol
{
	class TransactionHandler;
}

namespace Net { class Multiplexer; }

class ModuleServices
{
public:
	virtual int		RegisterTransactionHandler(protocol::TransactionHandler* pHandler) = 0;
	virtual int		UnregisterTransactionHandler(protocol::TransactionHandler* pHandler) = 0;

	virtual int		RegisterAuthenticator(IAuthenticator* pAuth) = 0;
	virtual int		UnregisterAuthenticator(IAuthenticator* pAuth) = 0;

	virtual Config*				GetConfig() = 0;
	virtual Net::Multiplexer*	GetMultiplexer() = 0;

	virtual int		TranslateCircuitAndValidate(InstrumentBlock* pBlock) = 0;
	virtual const ListParser::tComponentDefinitions* GetComponentDefinitions() = 0;
	virtual bool	ValidateMaxlists(const NetList2& componentList) = 0;

	virtual ~ModuleServices() {}
};

class Module
{
public:
	virtual int		Init()			{ return 1; }
	virtual int		IsInitDone()	{ return 1; }
	virtual int		HasInitFailed()	{ return 0; }
	virtual void	Tick() {}
	virtual int		RegisterModule(ModuleServices* pServices) = 0;
	virtual int		UnregisterModule() = 0;
	virtual			~Module() {}
};

// types for the exported dll functions
typedef Module* (*CreateModuleFunc_t)();
typedef void (*FreeModuleFunc_t)(Module*);

#endif
