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
 * Copyright (c) 2008 André van Schoubroeck
 * Copyright (c) 2009 Johan Zackrisson
 * All Rights Reserved.
 */

#include "ividrivers.h"

#include "circuit.h"

#include "DirectIVI/cIVIDCPower.h" 
#include "DirectIVI/cIVIDMM.h"
#include "DirectIVI/cIVIFGEN.h"
#include "DirectIVI/cIVIScope.h"

//#include "USB/cUSB.h"
#include <usbmatrix/usb.h>

#include "log.h"

#include <util/config.h>
#include <util/stringop.h>
#include <util/basic_exception.h>

using namespace IVIControl;
using namespace std;

Drivers::Drivers()
{
}

Drivers::~Drivers()
{
	while(!mDCPower.empty())
	{
		delete mDCPower.back();
		mDCPower.pop_back();
	}

	while(!mDMM.empty())
	{
		delete mDMM.back();
		mDMM.pop_back();
	}

	while(!mFGEN.empty())
	{
		delete mFGEN.back();
		mFGEN.pop_back();
	}

	while(!mScope.empty())
	{
		delete mScope.back();
		mScope.pop_back();
	}
	
	while(!mUSB.empty())
	{
		delete mUSB.back();
		mUSB.pop_back();
	}
}

int Drivers::Init(Config* pConfig)
{
	ivilog.Log(1) << "DirectIVI (" << __DATE__ << " " << __TIME__ << ") "
#ifdef _NI_EXT_  
	<< "with NI extentions"
#else
	<< "without NI extentions"
#endif
	<< endl; 

	cIVIDCPower * IVIDCPower = NULL;
	DMM_TYPE    * IVIDMM     = NULL;

	cIVIFGEN    * IVIFGEN    = NULL;
	cIVIScope   * IVIScope   = NULL;

	USBMatrix::USBConnection       *  USBdriver  = NULL;

	string sDCPowerDriver = pConfig->GetString("IVI.DCPowerDriver"  ,"MyDCPower");
	string sDMMDriver     = pConfig->GetString("IVI.DMMDriver"      ,"MyDMM");
	string sFGENDriver    = pConfig->GetString("IVI.FGENDriver"     ,"MyFGEN");
	string sScopeDriver   = pConfig->GetString("IVI.OSCDriver"    ,"MyScope");
	// the getstring only returns till the 2nd space, so i cannot use space separated driver names here
	vector <string> vDCPowerDriver;
	vector <string> vDMMDriver;
	vector <string> vFGENDriver;
	vector <string> vScopeDriver;

	Tokenize(sDCPowerDriver	,vDCPowerDriver	," ,;",true);
	Tokenize(sDMMDriver		,vDMMDriver		," ,;",true);
	Tokenize(sFGENDriver	,vFGENDriver	," ,;",true);
	Tokenize(sScopeDriver	,vScopeDriver	," ,;",true);

	ivilog.Log(5)
		<< "Configuration:" << endl
		<< "DCPowerDriver " << sDCPowerDriver << endl
		<< "DMMDriver " << sDMMDriver << endl
		<< "FGENDriver " << sFGENDriver << endl
		<< "ScopeDriver " << sScopeDriver << endl ;

	try {
		
		if (pConfig->GetInt("IVI.USBMatrix", 1) != 0) InitUSBMatrix();

		const char* drvstr;
		while (!vDCPowerDriver.empty()) {
			drvstr = vDCPowerDriver.back().c_str();
			ivilog.Log(5) << "Initlializing " << drvstr << endl;
			IVIDCPower = new cIVIDCPower();
			mDCPower.push_back(IVIDCPower);
			IVIDCPower->Init(drvstr);
			vDCPowerDriver.pop_back();
		}

		while (!vDMMDriver.empty()) {
			drvstr = vDMMDriver.back().c_str();
			ivilog.Log(5) << "Initlializing " << drvstr << endl;
			IVIDMM     = new DMM_TYPE(); // DMM with NI extentions (ind/cap/diode)
			mDMM.push_back(IVIDMM);
			IVIDMM->Init(drvstr);			
			vDMMDriver.pop_back();
		}

		while (!vFGENDriver.empty()) {
			drvstr = vFGENDriver.back().c_str();
			ivilog.Log(5) << "Initlializing " << drvstr << endl;
			IVIFGEN    = new cIVIFGEN();			
			mFGEN.push_back(IVIFGEN);
			IVIFGEN->Init(drvstr);
			vFGENDriver.pop_back();
		}


		while (!vScopeDriver.empty()) {
			drvstr = vScopeDriver.back().c_str();
			ivilog.Log(5) << "Initlializing " << drvstr << endl;
			IVIScope   = new cIVIScope();			
			mScope.push_back(IVIScope);
			IVIScope->Init(drvstr);
			vScopeDriver.pop_back();
		}      
	} catch (BasicException e) {
		ivilog.Error() << "Driver Initializing failed!" << endl << e.what() << endl; 
		return 0;
	}

	return 1;
}

void Drivers::InitUSBMatrix()
{
	ivilog.Log(5) << "Initlializing USB Driver" << endl;
	USBMatrix::USBConnection*  USBdriver = new USBMatrix::USBConnection();
	mUSB.push_back(USBdriver);
	USBdriver->Init();
}

cIVIDCPower* Drivers::GetDCPower(size_t num)
{
	if (num >= mDCPower.size()) return NULL;
	return mDCPower[num];
}

DMM_TYPE* Drivers::GetDMM(size_t num)
{
	if (num >= mDMM.size()) return NULL;
	return mDMM[num];
}

cIVIFGEN* Drivers::GetFGEN(size_t num)
{
	if (num >= mFGEN.size()) return NULL;
	return mFGEN[num];
}

cIVIScope* Drivers::GetScope(size_t num)
{
	if (num >= mScope.size()) return NULL;
	return mScope[num];
}

USBMatrix::USBConnection* Drivers::GetUSB(size_t num)
{
	if (num >= mUSB.size()) return NULL;
	return mUSB[num];
}

bool Drivers::MatrixEnabled()
{
	return (mUSB.size() > 0);
}