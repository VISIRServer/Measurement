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
 * Copyright (c) 2005-2009 Johan Zackrisson
 * All Rights Reserved.
 */

#include "observable.h"
#include <algorithm>

using namespace std;

/// Smart stl function for notifying all listeners in a container
/// \internal
class ForAllNotify
{
public:
	ForAllNotify(int notifyselection) { mSelection = notifyselection; }
	void operator() (Observer* pObserver) { pObserver->Notify(mSelection); }
	int mSelection;
};

Observable::Observable()
{
}

Observable::~Observable()
{
}

void Observable::AddObserver(Observer* pObserver)
{
	mObservers.push_back(pObserver);
}

void Observable::RemoveObserver(Observer* pObserver)
{
	mObservers.remove(pObserver);
}

void Observable::NotifyAll(int notifyselection)
{
	for_each(mObservers.begin(), mObservers.end(), ForAllNotify(notifyselection));
}

void Observable::Notify(Observer* pObserver, int notifyselection)
{
	pObserver->Notify(notifyselection);
}

