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

#include "compdefreader.h"

#include <instruments/netlist2.h>
#include <instruments/listparser.h>

#include <fstream>
#include <stringop.h>

#define WHITESPACE " \t"

ComponentDefinitionReader::ComponentDefinitionReader()
{
}

ComponentDefinitionReader::~ComponentDefinitionReader()
{
}

bool ComponentDefinitionReader::ReadFile(const std::string& filename)
{
	std::fstream file(filename.c_str()); //abrimos el archivo
	if (!file.is_open()) // ¿Es posible abrir el archivo?
	{
		return false; // No, retornamos false
	}

	while(!file.eof()) // Mientras no lleguemos al final del archivo
	{
		std::string line; // linea es de tipo string
		getline(file, line); //leemos linea

		if (IsComment(line)) continue; // es una linea de comentario?
		else // si no es una linea de comentarios parseamos la linea
		{
			ParseLine(line); // paserseamos la linea
		}		
	}

	return true; //retornamos true para indicar la lectura correcta
}

// miramos si la linea es una linea de comentario
bool ComponentDefinitionReader::IsComment(const std::string& line)
{
	size_t pos = line.find_first_not_of(WHITESPACE); // buscamos la posicion del primer caracter que no es espacio en blanco
	if (pos != std::string::npos) // si la linea no esta entera en blanco, es decir lo anterior no retorno no existe posicion (npos)
	{
		if ((line[pos] == '#') || (line[pos] == '*')) return true; //si el primer caracter de la linea es # o * es una linea de comentario
	}
	else return true; // empty line, las lineas vacias se consideran comentarios

	return false; // no es una linea de comentario
}

bool ComponentDefinitionReader::ParseLine(const std::string& inLine)
{
	std::string line; // declaramos line de tipo string
	size_t pos = inLine.find_first_of("#*"); // buscamos la posicion de la primera # o * , ¡¡ojo no se si funcionara o buscara # seguido de *
	if (pos != std::string::npos) line = std::string(inLine, 0, pos); // si ha encontrado alguna es que en la linea hay comentarios, eliminamos esa parte de la linea
	else line = inLine; // no hay comentarios en la linea, la guardamos sin recortar nada

	typedef std::vector<std::string> tTokens; 
	tTokens tokens; // creamos un vector para contener los token (palabras) que contiene la linea

	std::string type = "";
	int numCons = 0;
	bool hasSpecialValue = false;
	bool ignoreValue = false;
	bool canTurn = false;

	Tokenize(line, tokens, WHITESPACE, true); //obtenermos los tokens(palabras) de la linea
	for(size_t i=0;i<tokens.size(); i++) // recorremos el vector con los token obtenidos
	{
		switch(i)
		{
		case 0:
			type = ToUpper(tokens[0]); //convertimos a mayusculas y lo almacenamos en type
			break;
		case 1:
			numCons = ToInt(tokens[1]); //convertimos a entero y lo guardamos como el numero de pines o conexiones
			break;
		case 2: // ahora leemos los flags
			{
				std::string flags = tokens[2];
				for(size_t j=0;j<flags.size(); j++)
				{
					switch(flags[j])
					{
					case 'i':
					case 'I':
						ignoreValue = true; //ignoramos el valor
						break;
					case 's':
					case 'S':
						hasSpecialValue = true; //es un valor especial
						break;
					case 't':
					case 'T':
						canTurn = true; //se puede girar, ejemplo resistencias, condensadores no polarizados, etc..
						break;
					}
				}
			}
			break;
		default:
			return false; //no se ha parseado la linea
		}
	}
	//mCompDefs esta definido en el .h como ListParser::tComponentDefinitions
	mCompDefs.push_back(ComponentTypeDefinition(type, numCons, canTurn, ignoreValue, hasSpecialValue)); //guardamos el componente con sus valores en la pila
	return true; //se ha parseado la linea
}

const ListParser::tComponentDefinitions& ComponentDefinitionReader::GetDefinitions() const
{
	//mCompDefs esta definido en el .h como ListParser::tComponentDefinitions
	return mCompDefs; //retornamos la pila con la definicion de los componentes.
}