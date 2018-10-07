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

#pragma once
#ifndef __IVI_DRIVERS_H__
#define __IVI_DRIVERS_H__

#include <vector>

class cIVIDCPower;
class cNIDMM;
class cIVIDMM;
class cIVIFGEN;
class cIVIScope;
//class cUSB;

namespace USBMatrix { class USBConnection; }

class Config;

#ifdef _NI_EXT_
#define DMM_TYPE cNIDMM
#else
#define DMM_TYPE cIVIDMM
#endif

namespace IVIControl
{

class Drivers {
public:
	int Init(Config* pConfig);

	cIVIDCPower*	GetDCPower(size_t num);
	DMM_TYPE*		GetDMM(size_t num);
	cIVIFGEN*		GetFGEN(size_t num);
	cIVIScope*		GetScope(size_t num);
	USBMatrix::USBConnection*	GetUSB(size_t num);

	bool			MatrixEnabled();

	Drivers();
	~Drivers();
private:
	void InitUSBMatrix();
	
	std::vector<cIVIDCPower*>	mDCPower;
	std::vector<DMM_TYPE*>		mDMM;
	std::vector<cIVIFGEN*>		mFGEN;
	std::vector<cIVIScope*>		mScope;
	std::vector<USBMatrix::USBConnection*>	mUSB;
};

} // end of namespace

#endif
