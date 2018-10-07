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
#ifndef __MODULE_REGISTRY_H__
#define __MODULE_REGISTRY_H__

#include <list>

class Module;
class ConcreteModuleServices;
class Authentication;
class Config;
class TransactionControl;
class Service;

namespace Net { class Multiplexer; }

class ModuleRegistry
{
public:
	bool	LoadModules();
	bool	UnloadModules();

	bool	InitModules();
	bool	IsInitDone();
	bool	HasInitFailed();

	ModuleRegistry(Net::Multiplexer* pMultiplexer, Authentication* pAuth, Config* pConfig, TransactionControl* pTransactionControl, Service* pService);
	~ModuleRegistry();
private:
	typedef std::pair<Module*, void*> tModuleDLL;
	typedef std::list<tModuleDLL> tModules;
	typedef std::list<Module*> tInitModules;

	tModules	mModules;
	ConcreteModuleServices* mpServices;
	Config*		mpConfig;
	TransactionControl* mpTransactionControl;

	bool	mInitFailed;
	tInitModules mInitModules;
};

#endif
