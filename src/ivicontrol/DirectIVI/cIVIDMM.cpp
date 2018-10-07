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
* Contributor(s):
* André van Schoubroeck
*/


#ifdef _NI_EXT_
#include <nidmm.h>
#endif 

#include <ividmm.h>
#include "cIVIDMM.h"
#include "../doerror.h"
#include "../log.h"

using namespace std;

//-----------------------------------------------------------------------------
void cIVIDMM::GetError (int32_t error) {
	char str[2048];
	IviDmm_GetError(mSession, (ViStatus *)&error, 2048, str);
	doerror(str,error);
}
//-----------------------------------------------------------------------------
cIVIDMM::cIVIDMM() {
	ivilog.Log(5) << "cIVIDMM::cIVIDMM()" << endl;
}
//-----------------------------------------------------------------------------
cIVIDMM::~cIVIDMM() {
	IviDmm_close(mSession);
}
//-----------------------------------------------------------------------------
int32_t cIVIDMM::Init(std::string driver) {
	ivilog.Log(5) << "cIVIDMM::Init(" << driver << ")" << endl;
	int32_t error = VI_SUCCESS;
	uint32_t lSession;
	error = IviDmm_init ( (ViRsrc)driver.c_str() , VI_FALSE, VI_TRUE, (ViSession*)&lSession);
	mSession = lSession;
	if (error) GetError(error);
	return error;
}
//-----------------------------------------------------------------------------
double cIVIDMM::Measure(int32_t type, double range, double resolution) {
	double Result= 0;
	int32_t error = VI_SUCCESS;
	error = IviDmm_ConfigureMeasurement( mSession, type, range, resolution);
	if (error) GetError(error);
	error =  IviDmm_ConfigureTrigger(mSession, IVIDMM_VAL_IMMEDIATE,0);
	if (error) GetError(error);
	error = IviDmm_Read (mSession,5000, &Result);
	if (error) GetError(error);
	return Result;
}
//-----------------------------------------------------------------------------
void cIVIDMM::SetAutoZero (int32_t mode) {
	int32_t error;
	error = IviDmm_ConfigureAutoZeroMode(mSession, (ViInt32)mode);
	IVIDMM_VAL_AUTO_ZERO_ON;
	if (error) GetError(error);
}
