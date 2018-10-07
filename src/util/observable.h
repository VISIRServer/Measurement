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

#ifndef __OBSERVABLE_H__
#define __OBSERVABLE_H__

// this is not used anymore.. remove?

#include <list>

/// Observer, event listener (see Observer / Observable design pattern for more information )
class Observer
{
public:
	Observer()			{}
	virtual ~Observer()	{}
	virtual void Notify(int notifyselection = 0) = 0;
};

/// Observable, event producer (see Observer / Observable design pattern for more information )
/// Keeps track of all observers to notify with events when something has happened.
class Observable
{
public:
	//		add/remove observer from list
	void	AddObserver(Observer* pObserver);
	void	RemoveObserver(Observer* pObserver);

	//		nofity on change etc.
	void	NotifyAll(int notifyselection = 0);
	void	Notify(Observer* pObserver, int notifyselection = 0);

	// ctor / dtor
	Observable();
	virtual ~Observable();
private:
	typedef std::list< Observer* > tObservers;
	tObservers			mObservers;
};



#endif
