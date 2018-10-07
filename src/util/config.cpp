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

#include "config.h"
#include "stringop.h"

#include <fstream>

using namespace std;

// Config se trata de un diccionario de tipo map llamado mConfig que almacena los diferentes parametros de configuracion
// como clave - valor
Config::Config()
{
}

Config::~Config()
{
}

// retorna el valor correspondiente a la clave key, si no existe retorna el valor introducido como default
string Config::GetString(const string& key, string defaultval) const
{
	tConfig::const_iterator finder = mConfig.find(ToLower(key));
	if (finder != mConfig.end()) return finder->second;
	else return defaultval;
}
// lo mismo que la anterior pero en este caso no retorna en forma de string si no en forma de entero
int Config::GetInt(const string& key, int defaultval) const
{
	tConfig::const_iterator finder = mConfig.find(ToLower(key));
	if (finder != mConfig.end()) return atoi(finder->second.c_str());
	else return defaultval;
}
// almacena el par clave-valor en diccionario, las claves siempre en minusculas
void Config::SetString(const string& key, const string& value)
{
	mConfig[ToLower(key)] = value;
}
// Parseamos los parametros de configuración recibidos de la forma clave=valor
void Config::ParseParams(int argc, char** argv)
{
	for(int i=0;i<argc;i++)
	{
		string arg = argv[i];
		string key = Token(arg,0,"=");
		string value = Token(arg,1,"=");

		if (value == "") continue;
		else SetString(key,value);
	}
}
// leemos configuracion del archivo filename
bool Config::ParseFile(string filename)
{
	fstream file;
	file.open(filename.c_str(), ios_base::in);

	if (!file.is_open()) return false;

	while(!file.eof())
	{
		// get line
		string line;
		getline(file,line);

		// check if line is a comment
		line = CleanWhitespaces(line);
		if (line != "" && line[0] == '#') continue;

		// get key / value pair
		string key		= CleanWhitespaces(Token(line,0," \t"));
		string value	= CleanWhitespaces(Token(line,1," \t"));

		SetString(key,value);
	}

	return true;
}
