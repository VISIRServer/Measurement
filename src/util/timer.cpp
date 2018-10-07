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

#include "timer.h"

#ifdef _WIN32
#include <windows.h>
#pragma comment(lib, "winmm.lib")

timer::timer()
{
	restart();
}

void timer::restart()
{
	mStart = timeGetTime();
}

double timer::elapsed()
{
	double end = timeGetTime();
	return (end - mStart) / 1000.0;
}

void sys::Sleep(int timems)
{
	::Sleep(timems);
}

#else

#include <sys/time.h>
#include <unistd.h>

timer::timer()
{
	restart();
}

void timer::restart()
{
	timeval now;
	gettimeofday(&now, NULL);
	mStart = now.tv_sec+(now.tv_usec/1000000.0);
}

double timer::elapsed()
{
	timeval now;
	gettimeofday(&now, NULL);

	double end = now.tv_sec+(now.tv_usec/1000000.0);
	return (end - mStart);
}

void sys::Sleep(int timems)
{
	usleep(timems);
}

#endif