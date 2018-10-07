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

#include "gpib_device.h"

#include <syslog.h>

#include <windows.h>
#include "ni488.h"

using namespace gpib;

GPIBDevice::GPIBDevice()
{
	mMustRecover	= false;
	mBoardIndex		= 0;
	mPrimAddr		= 0;
	mSecAddr		= 0;
}

GPIBDevice::~GPIBDevice()
{
}

bool GPIBDevice::Open(int boardindex, int primaddr, int secaddr)
{
	mBoardIndex = boardindex;
	mPrimAddr = primaddr;
	mSecAddr = secaddr;

	Reset();

	mDevice = ibdev(boardindex, primaddr, secaddr, T1000s, 1, 0);
	if (ibsta & ERR)
	{
		Error("unable to open gpib device");
		return false;
	}

	Reset();

	//ibconfig(mDevice, IbcAUTOPOLL, 1);
	//ibnotify(mDevice, SRQI | RQS, func, 0);

	return true;
}

bool GPIBDevice::Close()
{
	ibonl (mDevice,0);
	if (ibsta & ERR)
	{
		Error("Failed to close GPIB device");
		return false;
	}
	return true;
}

bool GPIBDevice::SendCommand(const char* command)
{
	//printf("GPIB: %s\n", command);

	if (mMustRecover && !Recover()) return false;

	ibwrt(mDevice, (void*)command, strlen(command)+1);
	if (ibsta & ERR)
	{
		mMustRecover = true;
		Error("GPIB command failed");
		return false;
	}

	return true;
}

bool GPIBDevice::ReadData(char* buffer, size_t len)
{
	// this is a little weird..
	// because if the previous command has failed,
	// there is no real data to be read
	if (mMustRecover && !Recover()) return false;

	ibrd(mDevice, buffer, len);
	if (ibsta & ERR)
	{
		mMustRecover = true;
		Error("Failed to read from GPIB device");
		return false;
	}

	return true;
}

void GPIBDevice::Error(const char* error)
{
	syserr << timestamp << std::endl << "GPIB ERROR: " << error << std::endl;
}

bool GPIBDevice::Reset()
{
	Addr4882_t addr = MakeAddr(mPrimAddr, mSecAddr);
	DevClear(mBoardIndex, addr); // is there a way to error check?

	return true;
}

bool GPIBDevice::Recover()
{
	syserr << timestamp << "GPIB TRYING TO RECOVER FROM ERROR" << std::endl;
	if (!Reset()) return false;
	mMustRecover = false;
	// *RST (or equivalent) will be sent by the instrument controller
	return true;
}
