// Code borrowed from Howard Jeng's article on gamedev
// http://www.gamedev.net/reference/programming/features/vcppexceptmodel/

//Copyright (c) 2007 Howard Jeng
//
//Permission is hereby granted, free of charge, to any person obtaining a copy
//of this software and associated documentation files (the "Software"), to deal
//in the Software without restriction, including without limitation the rights
//to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//copies of the Software, and to permit persons to whom the Software is
//furnished to do so, subject to the following condition:
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR

//IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//THE SOFTWARE.

// Code borrowed from googles v8
// http://src.chromium.org/viewvc/chrome/branches/chrome_official_branch/src/v8/src/platform-win32.cc?revision=1757&pathrev=1763

// Copyright 2006-2008 the V8 project authors. All rights reserved.
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
//       copyright notice, this list of conditions and the following
//       disclaimer in the documentation and/or other materials provided
//       with the distribution.
//     * Neither the name of Google Inc. nor the names of its
//       contributors may be used to endorse or promote products derived
//       from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// With modifications by Johan Zackrisson (c) 2009

#include "crashinfo.h"

#include <iostream>
#include <sstream>

#include <fstream>

#include <windows.h>
#include <dbghelp.h>
#include <Tlhelp32.h>

// Code from google's V8

// The following code loads functions defined in DbhHelp.h and TlHelp32.h
// dynamically. This is to avoid being depending on dbghelp.dll and
// tlhelp32.dll when running (the functions in tlhelp32.dll have been moved to
// kernel32.dll at some point so loading functions defines in TlHelp32.h
// dynamically might not be necessary any more - for some versions of Windows?).

// Function pointers to functions dynamically loaded from dbghelp.dll.
#define DBGHELP_FUNCTION_LIST(V)  \
  V(SymInitialize)                \
  V(SymGetOptions)                \
  V(SymSetOptions)                \
  V(SymGetSearchPath)             \
  V(SymLoadModule64)              \
  V(StackWalk64)                  \
  V(SymGetSymFromAddr64)          \
  V(SymGetLineFromAddr64)         \
  V(SymFunctionTableAccess64)     \
  V(SymGetModuleBase64)

// Function pointers to functions dynamically loaded from dbghelp.dll.
#define TLHELP32_FUNCTION_LIST(V)  \
  V(CreateToolhelp32Snapshot)      \
  V(Module32FirstW)                \
  V(Module32NextW)

// Define the decoration to use for the type and variable name used for
// dynamically loaded DLL function..
#define DLL_FUNC_TYPE(name) _##name##_
#define DLL_FUNC_VAR(name) _##name

// Define the type for each dynamically loaded DLL function. The function
// definitions are copied from DbgHelp.h and TlHelp32.h. The IN and VOID macros
// from the Windows include files are redefined here to have the function
// definitions to be as close to the ones in the original .h files as possible.
#ifndef IN
#define IN
#endif
#ifndef VOID
#define VOID void
#endif

// DbgHelp.h functions.
typedef BOOL (__stdcall *DLL_FUNC_TYPE(SymInitialize))(IN HANDLE hProcess,
                                                       IN PSTR UserSearchPath,
                                                       IN BOOL fInvadeProcess);
typedef DWORD (__stdcall *DLL_FUNC_TYPE(SymGetOptions))(VOID);
typedef DWORD (__stdcall *DLL_FUNC_TYPE(SymSetOptions))(IN DWORD SymOptions);
typedef BOOL (__stdcall *DLL_FUNC_TYPE(SymGetSearchPath))(
    IN HANDLE hProcess,
    OUT PSTR SearchPath,
    IN DWORD SearchPathLength);
typedef DWORD64 (__stdcall *DLL_FUNC_TYPE(SymLoadModule64))(
    IN HANDLE hProcess,
    IN HANDLE hFile,
    IN PSTR ImageName,
    IN PSTR ModuleName,
    IN DWORD64 BaseOfDll,
    IN DWORD SizeOfDll);
typedef BOOL (__stdcall *DLL_FUNC_TYPE(StackWalk64))(
    DWORD MachineType,
    HANDLE hProcess,
    HANDLE hThread,
    LPSTACKFRAME64 StackFrame,
    PVOID ContextRecord,
    PREAD_PROCESS_MEMORY_ROUTINE64 ReadMemoryRoutine,
    PFUNCTION_TABLE_ACCESS_ROUTINE64 FunctionTableAccessRoutine,
    PGET_MODULE_BASE_ROUTINE64 GetModuleBaseRoutine,
    PTRANSLATE_ADDRESS_ROUTINE64 TranslateAddress);
typedef BOOL (__stdcall *DLL_FUNC_TYPE(SymGetSymFromAddr64))(
    IN HANDLE hProcess,
    IN DWORD64 qwAddr,
    OUT PDWORD64 pdwDisplacement,
    OUT PIMAGEHLP_SYMBOL64 Symbol);
typedef BOOL (__stdcall *DLL_FUNC_TYPE(SymGetLineFromAddr64))(
    IN HANDLE hProcess,
    IN DWORD64 qwAddr,
    OUT PDWORD pdwDisplacement,
    OUT PIMAGEHLP_LINE64 Line64);
// DbgHelp.h typedefs. Implementation found in dbghelp.dll.
typedef PVOID (__stdcall *DLL_FUNC_TYPE(SymFunctionTableAccess64))(
    HANDLE hProcess,
    DWORD64 AddrBase);  // DbgHelp.h typedef PFUNCTION_TABLE_ACCESS_ROUTINE64
typedef DWORD64 (__stdcall *DLL_FUNC_TYPE(SymGetModuleBase64))(
    HANDLE hProcess,
    DWORD64 AddrBase);  // DbgHelp.h typedef PGET_MODULE_BASE_ROUTINE64

// TlHelp32.h functions.
typedef HANDLE (__stdcall *DLL_FUNC_TYPE(CreateToolhelp32Snapshot))(
    DWORD dwFlags,
    DWORD th32ProcessID);
typedef BOOL (__stdcall *DLL_FUNC_TYPE(Module32FirstW))(HANDLE hSnapshot,
                                                        LPMODULEENTRY32W lpme);
typedef BOOL (__stdcall *DLL_FUNC_TYPE(Module32NextW))(HANDLE hSnapshot,
                                                       LPMODULEENTRY32W lpme);

#undef IN
#undef VOID

// Declare a variable for each dynamically loaded DLL function.
#define DEF_DLL_FUNCTION(name) DLL_FUNC_TYPE(name) DLL_FUNC_VAR(name) = NULL;
DBGHELP_FUNCTION_LIST(DEF_DLL_FUNCTION)
TLHELP32_FUNCTION_LIST(DEF_DLL_FUNCTION)
#undef DEF_DLL_FUNCTION

// Load the functions. This function has a lot of "ugly" macros in order to
// keep down code duplication.

static bool LoadDbgHelpAndTlHelp32() {
  static bool dbghelp_loaded = false;

  if (dbghelp_loaded) return true;

  HMODULE module;

  // Load functions from the dbghelp.dll module.
  module = LoadLibrary(TEXT("dbghelp.dll"));
  if (module == NULL) {
    return false;
  }

#define LOAD_DLL_FUNC(name)                                                 \
  DLL_FUNC_VAR(name) =                                                      \
      reinterpret_cast<DLL_FUNC_TYPE(name)>(GetProcAddress(module, #name));

DBGHELP_FUNCTION_LIST(LOAD_DLL_FUNC)

#undef LOAD_DLL_FUNC

  // Load functions from the kernel32.dll module (the TlHelp32.h function used
  // to be in tlhelp32.dll but are now moved to kernel32.dll).
  module = LoadLibrary(TEXT("kernel32.dll"));
  if (module == NULL) {
    return false;
  }

#define LOAD_DLL_FUNC(name)                                                 \
  DLL_FUNC_VAR(name) =                                                      \
      reinterpret_cast<DLL_FUNC_TYPE(name)>(GetProcAddress(module, #name));

TLHELP32_FUNCTION_LIST(LOAD_DLL_FUNC)

#undef LOAD_DLL_FUNC

  // Check that all functions where loaded.
  bool result =
#define DLL_FUNC_LOADED(name) (DLL_FUNC_VAR(name) != NULL) &&

DBGHELP_FUNCTION_LIST(DLL_FUNC_LOADED)
TLHELP32_FUNCTION_LIST(DLL_FUNC_LOADED)

#undef DLL_FUNC_LOADED
  true;

  dbghelp_loaded = result;
  return result;
  // NOTE: The modules are never unloaded and will stay around until the
  // application is closed.
}

bool LoadSymbols()
{
	LoadDbgHelpAndTlHelp32();
	HANDLE process_handle = GetCurrentProcess();

	_SymInitialize(process_handle, 0, FALSE);

	HANDLE snapshot = _CreateToolhelp32Snapshot(
		TH32CS_SNAPMODULE,       // dwFlags
		GetCurrentProcessId());  // th32ProcessId

	if (snapshot == INVALID_HANDLE_VALUE) return false;

	MODULEENTRY32W module_entry;
	module_entry.dwSize = sizeof(module_entry);  // Set the size of the structure.
	BOOL cont = _Module32FirstW(snapshot, &module_entry);
	while (cont)
	{
		DWORD64 base;
		base = _SymLoadModule64(
		process_handle,                                       // hProcess
		0,                                                    // hFile
		reinterpret_cast<PSTR>(module_entry.szExePath),       // ImageName
		reinterpret_cast<PSTR>(module_entry.szModule),        // ModuleName
		reinterpret_cast<DWORD64>(module_entry.modBaseAddr),  // BaseOfDll
		module_entry.modBaseSize);                            // SizeOfDll
		if (base == 0)
		{
			int err = GetLastError();
			if (err != ERROR_SUCCESS && err != ERROR_MOD_NOT_FOUND && err != ERROR_INVALID_HANDLE) return false;
		}
	cont = _Module32NextW(snapshot, &module_entry);
	}
	
	CloseHandle(snapshot);

	return true;
}

// code from Howard Jeng

std::string get_module_path(HMODULE module = 0) {
	char path_name[MAX_PATH] = {};
	DWORD size = GetModuleFileNameA(module, path_name, MAX_PATH);
	return std::string(path_name, size);
}

void write_module_name(std::ostream & os, HANDLE process, DWORD64 program_counter) {
	DWORD64 module_base = _SymGetModuleBase64(process, program_counter);
	if (module_base) {
		std::string module_name = get_module_path(reinterpret_cast<HMODULE>(module_base));
		if (!module_name.empty())
			os << module_name << "|";
		else 
			os << "Unknown module|";
	} else {
		os << "Unknown module|";
	}
}

void write_function_name(std::ostream & os, HANDLE process, DWORD64 program_counter) {
	/*
	SYMBOL_INFO_PACKAGE sym = { sizeof(sym) };
	sym.si.MaxNameLen = MAX_SYM_NAME;
	if (SymFromAddr(process, program_counter, 0, &sym.si)) {
		os << sym.si.Name << "()";
	} else {
		os << "Unknown function";
	}
	*/

	IMAGEHLP_SYMBOL64* sym = reinterpret_cast<IMAGEHLP_SYMBOL64*>(calloc(1, sizeof(IMAGEHLP_SYMBOL64) + MAX_SYM_NAME));
	if (sym == NULL) return; // out of memory, what to do?

    sym->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL64);
    sym->MaxNameLength = MAX_SYM_NAME;
	if (_SymGetSymFromAddr64(process, program_counter, 0, sym)) {
		os << sym->Name << "()";
	} else {
		os << "Unknown function";
	}

	free(sym);
}

void write_file_and_line(std::ostream & os, HANDLE process, DWORD64 program_counter) {
	IMAGEHLP_LINE64 ih_line = { sizeof(IMAGEHLP_LINE64) };
	DWORD dummy = 0;
	if (_SymGetLineFromAddr64(process, program_counter, &dummy, &ih_line)) {
		os << "|" << ih_line.FileName
			<< ":" << ih_line.LineNumber;
	}
}

void generate_stack_trace(std::ostream & os, CONTEXT ctx, int skip) {

	// Here is where we need the symbols, hopefully we have time to load it before things to bananas
	LoadSymbols();

	STACKFRAME64 sf = {};
	sf.AddrPC.Offset    = ctx.Eip;
	sf.AddrPC.Mode      = AddrModeFlat;
	sf.AddrStack.Offset = ctx.Esp;
	sf.AddrStack.Mode   = AddrModeFlat;
	sf.AddrFrame.Offset = ctx.Ebp;
	sf.AddrFrame.Mode   = AddrModeFlat;

	HANDLE process = GetCurrentProcess();
	HANDLE thread = GetCurrentThread();

	os << std::uppercase;
	for (;;) {
		SetLastError(0);
		BOOL stack_walk_ok = _StackWalk64(IMAGE_FILE_MACHINE_I386, process, thread, &sf,
			&ctx, 0, _SymFunctionTableAccess64, 
			_SymGetModuleBase64, 0);
		if (!stack_walk_ok || !sf.AddrFrame.Offset) return;

		if (skip) {
			--skip;
		} else {
			// write the address

			os << std::hex << reinterpret_cast<void *>(sf.AddrPC.Offset) << "|" << std::dec;

			write_module_name(os, process, sf.AddrPC.Offset);
			write_function_name(os, process, sf.AddrPC.Offset);
			write_file_and_line(os, process, sf.AddrPC.Offset);

			os << "\n";
		}
	}
}

struct UntypedException {
	UntypedException(const EXCEPTION_RECORD & er)
		: exception_object(reinterpret_cast<void *>(er.ExceptionInformation[1])),
		type_array(reinterpret_cast<_ThrowInfo *>(er.ExceptionInformation[2])->pCatchableTypeArray)
	{}
	void * exception_object;
	_CatchableTypeArray * type_array;
};

void * exception_cast_worker(const UntypedException & e, const type_info & ti) {
	for (int i = 0; i < e.type_array->nCatchableTypes; ++i) {
		_CatchableType & type_i = *e.type_array->arrayOfCatchableTypes[i];
		const std::type_info & ti_i = *reinterpret_cast<std::type_info *>(type_i.pType);
		if (ti_i == ti) {
			char * base_address = reinterpret_cast<char *>(e.exception_object);
			base_address += type_i.thisDisplacement.mdisp;
			return base_address;
		}
	}
	return 0;
}

void get_exception_types(std::ostream & os, const UntypedException & e) {
	for (int i = 0; i < e.type_array->nCatchableTypes; ++i) {
		_CatchableType & type_i = *e.type_array->arrayOfCatchableTypes[i];
		const std::type_info & ti_i = *reinterpret_cast<std::type_info *>(type_i.pType);
		os << ti_i.name() << "\n";
	}
}

template <typename T>
T * exception_cast(const UntypedException & e) {
	const std::type_info & ti = typeid(T);
	return reinterpret_cast<T *>(exception_cast_worker(e, ti));
}

DWORD do_filter(EXCEPTION_POINTERS * eps, std::string & buffer) {
	std::stringstream sstr;
	const EXCEPTION_RECORD & er = *eps->ExceptionRecord;
	int skip = 0;

	sstr << "FATAL EXCEPTION:\n";

	switch (er.ExceptionCode) {
	case 0xE06D7363: { // C++ exception

		UntypedException ue(er);
		if (std::exception * e = exception_cast<std::exception>(ue)) {
			const std::type_info & ti = typeid(*e);
			sstr << ti.name() << ":" << e->what();
		} else {
			sstr << "Unknown C++ exception thrown.\n";
			get_exception_types(sstr, ue);
		}
		skip = 2; // skip RaiseException and _CxxThrowException

					 } break;
	case EXCEPTION_ACCESS_VIOLATION: {
		sstr << "Access violation. Illegal "
			<< (er.ExceptionInformation[0] ? "write" : "read")
			<< " by "
			<< er.ExceptionAddress
			<< " at "
			<< reinterpret_cast<void *>(er.ExceptionInformation[1]);
									 } break;
	default: {
		sstr << "SEH exception thrown. Exception code: "
			<< std::hex << std::uppercase << er.ExceptionCode
			<< " at "
			<< er.ExceptionAddress;
			 }
	}
	sstr << "\n\nStack Trace:\n";
	generate_stack_trace(sstr, *eps->ContextRecord, skip);
	buffer = sstr.str();

	return EXCEPTION_EXECUTE_HANDLER;
}

DWORD seh_filter(EXCEPTION_POINTERS * eps, std::string & buffer) {
	__try {
		return do_filter(eps, buffer);
	} __except (EXCEPTION_EXECUTE_HANDLER) {
		return EXCEPTION_CONTINUE_SEARCH;
	}
}

#if 0

int actual_main(int, char **) {
	// do stuff

	// cause an access violation
	//char * ptr = 0; *ptr = 0;

	// divide by zero

	//int x = 5; x = x / (x - x);

	//throw x;

	// C++ exception
	throw std::runtime_error("I'm an exception!");
	//throw 5;

	return 0;
}

void save_buffer(const std::string & buffer) {
	std::ofstream ofs("err_log.txt");
	if (ofs) ofs << buffer;
}


int seh_helper(int argc, char ** argv, std::string & buffer) {
	__try {
		return actual_main(argc, argv);
	} __except (seh_filter(GetExceptionInformation(), buffer)) {
		if (!buffer.empty()) {
			save_buffer(buffer);
			MessageBoxA(0, buffer.c_str(), "Abnormal Termination", MB_OK);
		}
		return -1;
	}
} 


/*int main(int argc, char ** argv) {
SymInit sym;
std::string buffer;
return seh_helper(argc, argv, buffer);
}*/

#endif
