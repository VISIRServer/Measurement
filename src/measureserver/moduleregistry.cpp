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
 * Copyright (c) 2010 Johan Zackrisson
 * All Rights Reserved.
 */

#include "moduleregistry.h"
#include "module.h"
#include "transactioncontrol.h"
#include "service.h"
#include "authentication.h"

#include <config.h>
#include <stringop.h>

#include <syslog.h>
#include <dynlib.h>

class ConcreteModuleServices : public ModuleServices
{
public:
	virtual Config*	GetConfig()
	{
		return mpConfig;
	}

	virtual Net::Multiplexer* GetMultiplexer()
	{
		return mpMultiplexer;
	}

	virtual int		RegisterTransactionHandler(protocol::TransactionHandler* pHandler)
	{
		return mpTransactionControl->RegisterHandler(pHandler);
	}

	virtual int		UnregisterTransactionHandler(protocol::TransactionHandler* pHandler)
	{
		return mpTransactionControl->UnregisterHandler(pHandler);
	}

	virtual int		TranslateCircuitAndValidate(InstrumentBlock* pBlock)
	{
		return mpService->TranslateCircuitAndValidate(pBlock);
	}

	virtual int		RegisterAuthenticator(IAuthenticator* pAuth)
	{
		return mpAuth->RegisterAuthenticator(pAuth);
	}

	virtual int		UnregisterAuthenticator(IAuthenticator* pAuth)
	{
		return mpAuth->UnregisterAuthenticator(pAuth);
	}

	virtual const ListParser::tComponentDefinitions* GetComponentDefinitions()
	{
		return &mpService->GetComponentDefinitions();
	}

	virtual bool ValidateMaxlists(const NetList2& componentList)
	{
		return mpService->ValidateMaxlists(componentList);
	}

	ConcreteModuleServices(Net::Multiplexer* pMultiplexer,
						   Authentication* pAuth,
						   Config* pConfig,
						   TransactionControl* pTransactionControl,
						   Service* pService)
		: mpMultiplexer(pMultiplexer)
		, mpAuth(pAuth)
		, mpConfig(pConfig)
		, mpTransactionControl(pTransactionControl)
		, mpService(pService)
	{
	}
	virtual ~ConcreteModuleServices() {}
private:
	Net::Multiplexer*	mpMultiplexer;
	Authentication*		mpAuth;
	Config*				mpConfig;
	TransactionControl*	mpTransactionControl;
	Service*			mpService;
};


ModuleRegistry::ModuleRegistry(Net::Multiplexer* pMultiplexer, Authentication* pAuth, Config* pConfig, TransactionControl* pTransactionControl, Service* pService)
{
	mpServices = new ConcreteModuleServices(pMultiplexer, pAuth, pConfig, pTransactionControl, pService);
	mpConfig = pConfig;
	mInitFailed = false;
}

ModuleRegistry::~ModuleRegistry()
{
	UnloadModules();
	delete mpServices;
}

#ifdef _WIN32
#define MODULE_SUFFIX ".dll"
#define NEED_SUFFIX 0
#else
#define MODULE_SUFFIX ".module"
#define NEED_SUFFIX 1
#endif

bool ModuleRegistry::LoadModules()
{
	std::string loadModules = mpConfig->GetString("LoadModules", "");
	std::list<std::string> tokens;
	Tokenize(loadModules, tokens, ",");

	// backwards compability
	int useEq = mpConfig->GetInt("UseEQ", 0);
	if (useEq != 0)
	{
		tokens.push_back("eqcom");
		// todo: check for double module usage?
	}

	for(std::list<std::string>::const_iterator it = tokens.begin(); it != tokens.end(); it++)
	{
		std::string modulename = *it;
#if defined(_DEBUG)

#if defined(_WIN32)
		size_t pos = modulename.find_last_of(".");
		if (pos != std::string::npos)
		{
			modulename = std::string(modulename, 0, pos) + "_debug" + MODULE_SUFFIX;
		}
		else
		{
			modulename += "_debug";
		}
#else // if not windows
		modulename += "_debug" MODULE_SUFFIX;
#endif
		
#else // if not debug

#endif // END _DEBUG
		

		sysout << "Loading module: " << modulename << std::endl;

		dynlib_t dynlib = OpenLibrary(modulename.c_str());
		if (!dynlib)
		{
			syserr << "Failed to load: " << *it << " (" << modulename << ")" << std::endl;
			return false;
		}

		CreateModuleFunc_t CreateFunc = (CreateModuleFunc_t)GetLibrarySymbol(dynlib, "CreateModule");
		FreeModuleFunc_t FreeFunc = (FreeModuleFunc_t)GetLibrarySymbol(dynlib, "FreeModule");

		if (!CreateFunc || !FreeFunc)
		{
			syserr << "Module is missing exported functions" << std::endl;
			CloseLibrary(dynlib);
			return false;
		}

		Module* pModule = CreateFunc();
		if (!pModule)
		{
			syserr << "Failed to create module" << std::endl;
			CloseLibrary(dynlib);
			return false;
		}

		if (!pModule->RegisterModule(mpServices))
		{
			syserr << "Failed to register module" << std::endl;
			CloseLibrary(dynlib);
			return false;
		}

		mModules.push_back( tModuleDLL(pModule, dynlib) );
	}

	return true;
}

bool ModuleRegistry::UnloadModules()
{
	while(!mModules.empty())
	{
		tModules::iterator it = mModules.begin();		
		it->first->UnregisterModule();
		FreeModuleFunc_t FreeFunc = (FreeModuleFunc_t)GetLibrarySymbol(it->second, "FreeModule");
		FreeFunc(it->first);
		if (it->second) CloseLibrary(it->second);

		mModules.pop_front();
	}

	return false;
}

bool ModuleRegistry::InitModules()
{
	// this seem to be needed to avoid a crash when we have a exception in the Init method
	tModules modulesCopy = mModules;

	for(tModules::const_iterator it = modulesCopy.begin(); it != modulesCopy.end(); it++)
	{
		if (it->first->Init() == 0) return false;
		mInitModules.push_back(it->first);
	}

	return true;
}

bool ModuleRegistry::IsInitDone()
{
	if (mInitFailed) return true;
	if (mInitModules.empty()) return true;

	tInitModules::iterator it = mInitModules.begin();
	while(it != mInitModules.end())
	{
		if ((*it)->IsInitDone())
		{
			if ((*it)->HasInitFailed())
			{
				mInitFailed = true;
				return true;
			}
			it = mInitModules.erase(it);
		}
		else it++;
	}

	return false;
}

bool ModuleRegistry::HasInitFailed()
{
	return mInitFailed;
}