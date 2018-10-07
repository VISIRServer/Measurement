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

#include "cUSB.h"
#include "../doerror.h"
#include "../3rdparty/stdint.h"

#include <usb.h>
#include <stdio.h>

using namespace std;

cUSB::cUSB() {
	Initialised = false;
}

cUSB::~cUSB() {
	if (Initialised) {
		usb_release_interface(usbdev, 0);
		usb_close(usbdev);
	}
}

usb_dev_handle * cUSB::open_dev(void) {
	struct usb_bus *bus;
	struct usb_device *dev;

	for(bus = usb_get_busses(); bus; bus = bus->next) {
		for(dev = bus->devices; dev; dev = dev->next) {
			if(dev->descriptor.idVendor == MY_VID && 
								dev->descriptor.idProduct == MY_PID) {
				if (dev->descriptor.bcdDevice >= MINVERSION &&
						dev->descriptor.bcdDevice <= MAXVERSION) {
					Initialised = true;
					return usb_open(dev);
				} else {
					char msg[256];
					uint16_t MinVersion = MINVERSION, MaxVersion = MAXVERSION, CurVersion = dev->descriptor.bcdDevice;
					uint8_t  MinLow, MinHigh, MaxLow, MaxHigh, CurLow, CurHigh;
					uint8_t *cp;
					MinLow = (uint8_t)MinVersion;
					MaxLow = (uint8_t)MaxVersion;
					CurLow = (uint8_t)CurVersion;

					cp = (uint8_t*)(&MinVersion)+1;      MinHigh = *cp;
					cp = (uint8_t*)(&MaxVersion)+1;      MaxHigh = *cp;
					cp = (uint8_t*)(&CurVersion)+1;      CurHigh = *cp;
					
					//MinHigh = (uint8_t)*((&MinVersion)+1);
					//MaxHigh = (uint8_t)*((&MaxVersion)+1);
					//CurHigh = (uint8_t)*((&CurVersion)+1);

					sprintf(msg,"USB ERROR: Incorrect USB Matrix revision\nCurrent Version %hu.%hu\nMinimal Version %hu.%hu\nMaximal Version %hu.%hu\n",
						CurHigh,CurLow,MinHigh,MinLow,MaxHigh,MaxLow);
					doerror (msg);
					return NULL;
				}
			}
		}
	}
	return NULL;
}

int cUSB::Init() {
	usb_init(); /* initialize the library */
	usb_find_busses(); /* find all busses */
	usb_find_devices(); /* find all connected devices */

	if(!(usbdev = open_dev())) {
		doerror("USB ERROR: USB Matrix Not Found!");
	}

	if(usb_set_configuration(usbdev, 1) < 0) {
		usb_close(usbdev);
		doerror("USB ERROR: Error while initialising communication with USB matrix (setting config 1 failed)");
	}

	if(usb_claim_interface(usbdev, 0) < 0) {
		usb_close(usbdev);
		doerror("USB ERROR: Error while initialising communication with USB matrix (claiming interface 0 failed)");
	}
	
	return 1;
}

int cUSB::Send(char *data, int length) {
	return usb_bulk_write(usbdev, EP_OUT, data, length, USB_TIMEOUT);
}


int cUSB::Read(char *data, int length) {
	return usb_bulk_read(usbdev, EP_IN, data, length, USB_TIMEOUT);
}
