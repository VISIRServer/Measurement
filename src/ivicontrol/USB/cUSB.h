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
 *
 * Dependencies : libusb-win32 http://libusb-win32.sourceforge.net/
 */

#pragma once
#ifndef __CUSB_H__
#define __CUSB_H__


// the device's vendor and product id
#define MY_VID 0x1043
#define MY_PID 0x0000

// the device's endpoints 
#define EP_IN  0x81
#define EP_OUT 0x01

// min and max allowed device revision
#define MINVERSION 0x0000
#define MAXVERSION 0x0000

#define USB_TIMEOUT 5000

struct usb_dev_handle;

class cUSB {
public:
  cUSB();
  ~cUSB();

  int Init();
  int Send(char * data, int length);
  int Read(char * data, int length);
private:
  usb_dev_handle *open_dev(void);
  usb_dev_handle *usbdev;
  bool Initialised;
};

#endif
