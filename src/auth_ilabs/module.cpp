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
 * Copyright (c) 2009 Johan Zackrisson
 * All Rights Reserved.
 */

#include "module.h"
#include "callback.h"

#include <iostream>
using namespace Auth_ILabs;
using namespace std;

ILabsAuthModule::ILabsAuthModule()
{
	mpCallback = new Callback();
}

ILabsAuthModule::~ILabsAuthModule()
{
	delete mpCallback;
}

int ILabsAuthModule::RegisterModule(ModuleServices* pServices)
{
	cout << "Register ILabs Authentication module" << endl;
	mpServices = pServices;

	mpCallback->SetService(mpServices);
	mpServices->RegisterAuthenticator(mpCallback);

	return 1;
}

int ILabsAuthModule::UnregisterModule()
{
	mpServices->UnregisterAuthenticator(mpCallback);

	cout << "Unregister ILabs Authentication module" << endl;
	mpServices = NULL;
	return 1;
}
