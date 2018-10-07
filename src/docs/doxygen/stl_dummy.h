/**** BEGIN LICENSE BLOCK ****
 * This file is a part of the VISIR(TM) (Virtual Systems in Reality)
 * Software package.
 * 
 * VISIR(TM) is used to open laboratories for remote operation and control
 * as a supplement and a complement to local use.
 * 
 * Copyright (c) 2001 - 2006 Johan Zackrisson, Ingvar Gustavsson, Lars Håkansson,
 * Ingvar Claesson, and Thomas Lagö, All Rights Reserved.
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
 * Johan Zackrisson, jza@bth.se, initial developer
 */
#pragma once
#ifndef __STL_DUMMY__
#define __STL_DUMMY__

// without namespaces.. to match "using namespace std"
  /// STL container
  template<class T> class vector { public: T element; };
  /// STL container
  template<class K, class T> class map { public: K key; T element; };
  /// STL container
  template<class T> class list { public: T element; };
  /// STL container
  template<class T> class set { public: T element; }

namespace std {
  /// STL container
  template<class T> class vector { public: T element; };
  /// STL container
  template<class K, class T> class map { public: K key; T element; };
  /// STL container
  template<class T> class list { public: T element; };
  /// STL container
  template<class T> class set { public: T element; }
}

#endif
