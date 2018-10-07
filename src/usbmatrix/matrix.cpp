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

#include "matrix.h"
#include "usb.h"
#include "log.h"

#include <stdio.h>

using namespace USBMatrix;

#define MATRIX_MAX_CARDS 32

Matrix::Matrix(USBConnection* pUSB)
{
	mpUSB = pUSB;
	mMatrixCache = tMatrixSetup(MATRIX_MAX_CARDS, 0);
}

Matrix::~Matrix()
{
	mpUSB = NULL;
}

void Matrix::ReleaseAll()
{
	// fixme: only release used cards
	for(size_t i=0; i<mMatrixCache.size(); i++)
	{
		SendCardSetup(i, 0, true);
	}
}

void Matrix::ReleaseSources()
{
	for(size_t i=16; i<mMatrixCache.size(); i++)
	{
		SendCardSetup(i, 0);
	}
}

void Matrix::SendCardSetup(unsigned int card, unsigned int data, bool force)
{
	if (mMatrixCache[card] != data || force)
	{
		char buf[16];
		unsigned char cardnr = (unsigned char) card;
		unsigned char c1 = data & 0x7f;
		unsigned char c2 = (data >> 7) & 0x7f;
		unsigned char c3 = (data >> 14) & 0x7f;
		_snprintf(buf, 15, "%03u%03u%03u%03u", cardnr, c1, c2, c3);
		matrixlog.Log(5) << "USB " << buf << std::endl;

		mpUSB->Send(buf, strlen(buf));

		mMatrixCache[card] = data;
	}
}

void Matrix::SetupCircuit(const Circuit::tCardCompList& comps)
{
	tMatrixSetup newSetup = tMatrixSetup(MATRIX_MAX_CARDS, 0);

	typedef Circuit::tCardCompList compvector;
	compvector::const_iterator it = comps.begin();
	while(it != comps.end())
	{
		newSetup[it->first] = 1 << (it->second - 1);
		it++;
	}

	// send the new config
	for(int i=0; i<MATRIX_MAX_CARDS; i++)
	{
		SendCardSetup(i, newSetup[i]);
	}
}

void Matrix::FlipSwitches(const Circuit::tCardCompList& comps)
{
	tMatrixSetup newSetup = tMatrixSetup(MATRIX_MAX_CARDS, 0);

	typedef Circuit::tCardCompList compvector;
	compvector::const_iterator it = comps.begin();

	// we need to do the masking in two steps, not to overwrite the new setup
	// first we copy the setup of the cards that we are going to write to
	// then we flip the component bits that needs to be flipped

	while(it != comps.end())
	{
		// copy the current setup
		newSetup[it->first] = mMatrixCache[it->first];
		it++;
	}

	it = comps.begin();
	while(it != comps.end())
	{
		// xor the component bit that needs to be flipped from the current setup
		newSetup[it->first] ^= (1 << (it->second - 1));
		it++;
	}

	for(int i=0; i<MATRIX_MAX_CARDS; i++)
	{
		SendCardSetup(i, newSetup[i]);
	}
}
