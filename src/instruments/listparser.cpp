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
 * Copyright (c) 2004-2009 Johan Zackrisson
 * All Rights Reserved.
 */

#include "listparser.h"
#include <basic_exception.h>

#include <stringop.h>
#include <fstream>

#define WHITESPACE " \t"

ListParser::ListParser(const tComponentDefinitions& definitions)
{
	for(tComponentDefinitions::const_iterator it = definitions.begin(); it != definitions.end(); it++)
	{
		mCompDefMap[it->Type()] = *it;
	}
}

bool ListParser::IsComment(const std::string& instring)
{
	if (instring.empty())		return true;

	size_t pos = instring.find_first_not_of(WHITESPACE);
	if (pos != std::string::npos)
	{
		if ((instring[pos] == '#') || (instring[pos] == '*')) return true;
	}
	else return true; // only whitespace on line

	return false;
}

bool ListParser::Parse(std::string aList)
{
	//mComponentList se define en el .h como ListComponent::tComponentList
	mComponentList.clear(); // nuke all old nodes

	std::string line; // declaramos line como tipo string
	while(GetLine(aList, line)) // cogemos una linea y la almacenamos en line
	{
		size_t pos = line.find_first_of("*#"); //buscamos la posicion de * o #
		if (pos != std::string::npos) line = std::string(line, 0, pos); // hay comentarios, los eliminamos

		//line = Token(line,0,"*");	// remove everything after the first '*', which is the comment separator		
		//line = CleanWhitespaces(line);		
		//line = ToUpper(line);		// make uppercase.. easier parsing..
		
		if (IsComment(line)) continue; // si es una linea de comentario, continuamos con la siguiente
		line = ToUpper(line); // Convertimos a mayusculas la linea

		ListComponent* pComp = CreateComponent(line); //
		if (!pComp)
		{
			delete pComp;
			return false;
		}
		else
		{
			mComponentList.push_back(*pComp);
			delete pComp; // we copy the content.. then we can destroy this!
		}
	}
	return true;
}

ListComponent* ListParser::CreateComponent(const std::string& instring)
{
	std::string type_name = Token(instring, 0); // obtenermos la primera palabra de la linea que es el tipo_nombre
	std::string restofstring = RemoveToken(instring, 0); //le eliminamos la primera palabra y guardamos el resultado en restofstring

	std::string type = Token(type_name, 0, "\t _"); // buscamos el tipo, separando por \t o _ el primero que encuentre
	std::string name = RemoveToken(type_name, 0, "_"); // eliminamos la primera parte y nos quedamos el resto en name
	
	size_t groupID = 0;
	size_t atPos = name.find_first_of("@"); //buscamos la @ en el nombre
	if (atPos != std::string::npos) { //¿existe posicion con @ en el nombre? si existe entra
		groupID = atoi(std::string(name, atPos+1).c_str()); //atoid convierte a entero 
		name = std::string(name, 0, atPos);
		if (groupID > 20) throw BasicException("unexpected group id in maxlist"); //lanzamos una exception para la que hemos personalizado el what
	}

	const ComponentTypeDefinition* pType = GetTypeDefinition(type); 
	if (!pType)
		return NULL;

	ListComponent* pComponent = new ListComponent(type, name);
	// read connections
	for(int i=0;i<pType->NumConnections();i++)
	{
		std::string con = Token(restofstring, 0);
		if (con == "")
			return NULL; // fail
		restofstring = RemoveToken(restofstring, 0); // cut away the parsed bit
		pComponent->AddConnection(con);
	}

	if (!pType->IgnoreValue()) {
		std::string value = Token(restofstring, 0);
		pComponent->SetValue(value);
		restofstring = RemoveToken(restofstring, 0);
	}

	if (pType->HasSpecialValue()) pComponent->SetSpecial(restofstring);
	pComponent->SetGroup(groupID);

	return pComponent;
}

const ListComponent::tComponentList& ListParser::GetList() const
{
	return mComponentList;
}

bool ListParser::ParseFile(const std::string& filename) //parseamos archivo
{
	std::fstream file(filename.c_str()); //abrimos el archivo
	if (!file.is_open()) //¿ha podido abrir el archivo?
	{
		return false; //no, retornamos false
	}

	std::string buffer; // buffer declarado como string

	while(!file.eof()) // leemos el archivo linea a linea hasta el final
	{
		std::string str; 
		getline(file, str); // leemos linea y la almacenamos en str
		buffer += str + "\n"; // la alamcenamos en buffer con un retorno de linea al final
	}

	return Parse(buffer); //retonamos buffer parseado
}

const ComponentTypeDefinition* ListParser::GetTypeDefinition(const std::string& type) const
{
	tCompDefMap::const_iterator finder = mCompDefMap.find(type);
	if (finder == mCompDefMap.end()) return NULL;
	else return &finder->second;
}