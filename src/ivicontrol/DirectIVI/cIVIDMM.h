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

#pragma once
#ifndef __IVI_DMM_H__
#define __IVI_DMM_H__

#include "../3rdparty/stdint.h" 
#include <string>

#define DMM_DC_VOLTS                     (1L)
#define DMM_AC_VOLTS                     (2L)
#define DMM_DC_CURRENT                   (3L)
#define DMM_AC_CURRENT                   (4L)
#define DMM_2_WIRE_RES                   (5L)
#define DMM_4_WIRE_RES                   (101L)
#define DMM_AC_PLUS_DC_VOLTS             (106L)
#define DMM_AC_PLUS_DC_CURRENT           (107L)
#define DMM_FREQ                         (104L)
#define DMM_PERIOD                       (105L)
#define DMM_TEMPERATURE                  (108L)

#ifdef _NI_EXT_
#define DMM_AC_VOLTS_DC_COUPLED  (1001L)
#define DMM_DIODE                (1002L)
#define DMM_WAVEFORM_VOLTAGE     (1003L)
#define DMM_WAVEFORM_CURRENT     (1004L)
#define DMM_CAPACITANCE          (1005L)
#define DMM_INDUCTANCE           (1006L)
#endif

class cIVIDMM  {
public:
  cIVIDMM();
  ~cIVIDMM();
  int Init(std::string driver );
  
  void GetError (int32_t error);
  double Measure(int32_t Type, double Range, double Resolution);
  void SetAutoZero (int32_t mode);
protected:
  uint32_t mSession; 
};

#endif