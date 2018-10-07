#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <algorithm>

#include <instruments/circuitlist.h>
#include <instruments/circuitsolver3.h>
#include <instruments/listparser.h>
#include <instruments/compdefreader.h>
#include <instruments/listproducer.h>
#include <instruments/netlist2.h>

#include <util/timer.h>

#include <contrib/md5.h>

#ifdef _WIN32
#include <windows.h>
#pragma comment(lib, "winmm.lib")
#define DIR_SEPARATOR "\\"
#else
#include <glob.h>
#include <libgen.h>
#define DIR_SEPARATOR "/"
#endif

using namespace std;

#define COMPONENT_DEFINITION "component.types"

typedef pair<string, ListComponent::tComponentList> tMaxListNamePair;
typedef vector< tMaxListNamePair > tMaxLists;

// Muestra un menu con las opciones del programa y sale del mismo
void usage(char* cmdname)
{
	cout << cmdname << " <flags> <filelist>" << endl;
	cout << " Flags:" << endl;
	cout << "  -d <compdef>" << endl;
	cout << "  -o <output>" << endl;
	cout << "  -m <maxlistconf>" << endl;
	cout << "  -v" << endl;

	exit(1); // sale del programa, para que puedas reintentar el enviar datos en formato correcto.
}

void NormalizeWinPath(string& path)
{
#ifdef _WIN32 //nuestro caso, se compila para windows 32
	for(size_t i = 0;i<path.size(); i++)
	{
		if (path[i] == '/') path[i] = '\\'; //cambiamos las barras / por doble barra \\
	}
#else
	string out;
	for (char c : path) {
		if (c == '\r') continue;
		out += c;
	}
	path = out;
#endif
}

string BaseName(const string& path)
{
#ifdef _WIN32
	size_t last = path.find_last_of("\\");
	if (last != string::npos) return string(path, last+1);
	return path;
#else
	char buffer[path.size()];
	strlcpy(buffer, path.c_str(), path.size());
	return ::basename(buffer);
#endif
}

string DirName(string& path)
{
#ifdef _WIN32
	size_t last = path.find_last_not_of("\\"); // buscamos la posicion de lacadena que hay a continacion de las ultimas dos barras
	size_t pathend = path.find_last_of("\\", last); // obtenermos la posicion del directorio que contiene al archivo
	if (pathend != string::npos) return string(path, 0, pathend); // retornamos la posicion donde indica el directorio.
	else return path; // el archivo no esta contenido en un directorio, se encuentra en el mismo dir que el programa
#else
	char buffer[path.size()];
	strlcpy(buffer, path.c_str(), path.size());
	return ::dirname(buffer);
#endif
}

// crea un fileList a partir de los argumentos que se le van pasado
// estos argumentos son rutas a las netlist de los circuitos a chequear
void BuildFileList(string arg, vector<string>& fileList)
{
#ifdef _WIN32
	WIN32_FIND_DATAA filedata; // Creamos una estructura tipo WIN32_FIND_DATAA, en esta estructura se guarda:
	// fecha creacion del archivo, si esta comprimido, si esta oculto, si es un directorio...

	NormalizeWinPath(arg); // sustituimos la barra / por doble barra \\

	string path = DirName(arg); // nos devuelve el nombre del directorio donde esta contenido nuestro archivo
	if (!path.empty()) path += DIR_SEPARATOR; // si esta vacio es que esta en el mismo que el programa le añadimos barra

	HANDLE fh = FindFirstFileA(arg.c_str(), &filedata); //crea el manejador del archivo
	if (INVALID_HANDLE_VALUE != fh) // se ha podido crear el manejador?
	{
		fileList.push_back(path + filedata.cFileName);

		while(FindNextFileA(fh, &filedata) != 0)
		{
			fileList.push_back(path + filedata.cFileName);
		}

		FindClose(fh); //cierra la busqueda
	}
#else
	glob_t globblob;
	if (0 != glob(arg.c_str(), 0, NULL, &globblob)) return;
	
	for(int i=0;i<globblob.gl_pathc;i++)
	{
		fileList.push_back(globblob.gl_pathv[i]);
	}
	
	globfree(&globblob);
	
#endif
}

bool ReadMaxList(string file, tMaxLists& maxLists, const ListParser::tComponentDefinitions& compdefs)
{
	NormalizeWinPath(file); // Cambiamos la barra / por doble barra //

	ListParser parser(compdefs); // creamos un objeto lipo parser y le pasamos la definicion de los componentes
	if (!parser.ParseFile(file)) // Ha podido parsear el archivo .max?
	{
		cerr << "failed to read maxlist: " << file << endl; //imprimimos que fallo la lectura del archivo .max
		return false; // retornamos false
	}

	maxLists.push_back(tMaxListNamePair(file, parser.GetList())); 
	return true;
}

bool LoadMaxlists(string maxlistConf, tMaxLists& maxLists, const ListParser::tComponentDefinitions& compdefs)
{
	NormalizeWinPath(maxlistConf); // cambiamos la barra / por doble barra //

	string path = DirName(maxlistConf); // nos devuelve el nombre del directorio donde esta contenido nuestro archivo
	if (!path.empty()) path += DIR_SEPARATOR; // Si esta vacio le añadimos un barra

	fstream file(maxlistConf.c_str()); // abrimos el archivo
	if (!file.is_open()) // ¿se ha podido abrir el archivo?
	{
		cerr << "Can't find maxlist config: " << maxlistConf << endl; //No, imprimimos error 
		return false; //retornamos false
	}

	while(!file.eof()) //leeemos linas del archivo hasta el final
	{
		string str; // lo definimos como string
		getline(file,str); //leemos una linea y la guardamos en str

		if (str.empty()) continue; // si la linea esta vacia continuamos
		if ((str[0] == '#') || (str[0] == '*')) continue; //¿es una linea de comentario?

		NormalizeWinPath(str); // cambiamos la barra / por dos barras \\

		if (!ReadMaxList(path + str, maxLists, compdefs)) //leemos el archivo que viene el el maxconf, ¿ha podido leerlo?
		{
			return false; // no puede leerlo, retornamos false
		}
	}

	return true; //lectura correcta, retornamos true
}

// argc contiene el numero de argumentos pasados por consola y argv los argumentos 
// (argv[0] siempre es el nombre del programa
// cada espacio en blanco cuenta un argumento 
// nuestros argumentos de opcion se pasan asi:
// -letra_opcion value 
// de forma que el primer arguemento es la opción y el siguiente argumento es su valor si tiene
// despues de los argumentos de opción se pasa la ruta a la netlist del circuito a testear como argumentos.
int main(int argc, char** argv) 								
{
	string aCompDefFile = COMPONENT_DEFINITION;
	string aOutDir = "";
	string aMaxListConf = "";
	int		aVerbose = 0;
	int		aSilent = 0;

	struct sOptions {
		string option;
		string* sValue;
		int*	iValue;
	} aOptions[] = {
		{ "d:", &aCompDefFile, NULL }
		, { "o:", &aOutDir, NULL }
		, { "m:", &aMaxListConf, NULL }
		, { "v", NULL, &aVerbose }
		, { "s", NULL, &aSilent }
	};	

	if (argc < 2) usage(argv[0]);

	vector<string>	fileList;
	tMaxLists		aMaxLists;

	bool parseFlags = true;

	// parse options
	int i=1;
	while(parseFlags && i<argc)
	{
		string option = argv[i];
		if (option[0] == '-') // debe empezar por -
		{
			if (option.size() > 1) // el arguemento debe contener algo mas que -
			{
				char optletter = option[1]; // despues de - va la letra que indica la opcion
				for(size_t optind = 0; optind < (sizeof(aOptions) / sizeof(aOptions[0])); optind++) //recorremos los diferentes aoption de la estructura anterior
				{
					if (optletter == aOptions[optind].option[0]) //comparamos si nuestra letra de opcion se encuentra en la estructura anterior
					{
						if (aOptions[optind].option.size() > 1 && aOptions[optind].option[1] == ':') // ¿se trata de opcion "d:", "o:" o "m:"?
						{
							if (i+1 < argc) // ¿hay mas arguementos a continuación? Si, incrementamos al final i++
							{
								if (aOptions[optind].sValue != NULL) *aOptions[optind].sValue = argv[i+1]; // nuestra opcion debe contener valor? si es si el siguiente argumento es el valor
								i++;
							}
						}
						else //Era el ultimo arguemento
						{
							if (aOptions[optind].iValue != NULL) *aOptions[optind].iValue = 1; //Tiene que tener valor? como era el ultimo argumento cargamos su valor con 1
						}
					}
				}
			}

			i++; // solo contenia - no habia mas valores, por lo que se intenta con el siguiente argumento
		}
		else
		{
			parseFlags = false; // no contiene el identificador de argumento salimos
		}
	}

	NormalizeWinPath(aOutDir); // cambia la barra de / a doble barra \\
	//aOutDir = DirName(aOutDir);

	// a continuacion de los argumentos de opciones vienen los argumentos que contienen la netlist del circuito a testear.
	for(;i<argc;i++)
	{
		BuildFileList(argv[i], fileList); // creamos un fileList con esta netlist de circuito a testear
	}

	if (fileList.empty()) // ¿no se le ha pasado ninguna netlist de circuito a testear?
	{
		cerr << "No input files" << endl; //imprimimos error indicandolo
		usage(argv[0]); // mostramos menu con opciones
	}

	ComponentDefinitionReader compdef; // declaramos de tipo definicion de componentes
	if (!compdef.ReadFile(aCompDefFile)) // intentamos leer el archivo de definicion de componentes, ¿se puede?
	{
		cerr << "Can't find component definitions file: " << aCompDefFile << endl; //mostramos mensaje de que no se ha podido leer
		exit(1); // salimos porque no se a podido leer
	}

	if (!aMaxListConf.empty()) // tenemos archivo de maxlist?
	{
		if (!LoadMaxlists(aMaxListConf, aMaxLists, compdef.GetDefinitions())) //leemos el archivo de maxlist, ¿se puede leer?
		{
			cerr << "Failed to read all maxlists" << endl; // no se ha podido leer mostramos mensaje
		}
	}

	timer total_timer;

	// statistics
	int processed = 0;
	int solved = 0;
	int unsolved = 0;
	double totalTime = 0;
	double totalSolvedTime = 0;
	double totalUnsolvedTime = 0;

	for(vector<string>::const_iterator it = fileList.begin(); it != fileList.end(); it++)
	{
		ListParser parser(compdef.GetDefinitions());
		//cout << "Parsing: " << *it << endl;
		if (!parser.ParseFile(*it))
		{
			cerr << "Unable to parse: " << *it << endl;
			continue;
		}

		bool found = false;
		tMaxLists::const_iterator maxit = aMaxLists.begin();

		CircuitList solver;
		if (aVerbose) solver.EnableLogging();
		
		timer solvetimer;

		while(!found && maxit != aMaxLists.end())
		{
			//cout << "Testing against maxlist: " << maxit->first << endl;
			if (solver.Solve(parser.GetList(), maxit->second))
			{
				//cout << "Found solution against: " << maxit->first << endl;
				found = true;

				if (aVerbose)
				{
					cout << "Solution:" << endl;
					NetList2 solvednetlist;
					solvednetlist.SetNodeList(solver.GetSolution());
					cout << solvednetlist.GetNetListAsString() << endl;
				}
			}
			else
			{
				maxit++;
			}
		}

		double solvetime = solvetimer.elapsed();
		totalTime += solvetime;
		processed++;

		if (found)
		{
			solved++;
			totalSolvedTime += solvetime;

			if (aSilent) continue;

			cout << *it << " matches " << maxit->first;

			CircuitList::tCircuitList solution = solver.GetSolution();
			sort(solution.begin(), solution.end());
			string solutionstr = ListProducer::Produce(solution);			

			unsigned char md5sum[16];
			md5((unsigned char*)solutionstr.c_str(), solutionstr.size(), md5sum);

			char output[33];
			for( int i = 0; i < 16; i++ )
			{
				sprintf(&output[i*2] , "%02x", md5sum[i]);
			}
			output[32] = '\0';

			cout << " " << output << endl;

			if (aOutDir != "")
			{
				string filename = string(aOutDir) + "\\" + BaseName(*it) + ".solved";
				fstream ofile(filename.c_str(), fstream::out);
				if (!ofile.is_open()) cerr << "Failed to write output file: " << filename << endl;
				//ofile << "# Input:   " << *it << endl;
				//ofile << "# Maxlist: " << maxit->first << endl;
				//ofile << "# Solution: " << endl;
				ofile << solutionstr << endl;
			}
		}
		else
		{
			unsolved++;
			totalUnsolvedTime += solvetime;
			
			if (aSilent) continue;
			cout << *it << " has no solution" << endl;
		}
	}

	cerr << "Time: " << total_timer.elapsed() << endl;

	double avgtot = totalTime / processed;
	double avgsolved = totalSolvedTime / processed;
	double avgunsolved = totalUnsolvedTime / unsolved;
	avgtot *= 1000.0;
	avgsolved *= 1000.0;
	avgunsolved *= 1000.0;

	cerr << "Total processed: " << processed << " avg time: " << std::fixed << avgtot << "ms" << endl;
	cerr << "Solved: " << solved << " avg time: " << std::fixed << avgsolved << "ms" << endl;
	cerr << "Unsolved: " << unsolved << " avg time: " << std::fixed << avgunsolved << "ms" << endl;

	return 0;
}