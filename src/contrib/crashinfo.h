#pragma once
#ifndef __CRASH_INFO_H__
#define __CRASH_INFO_H__

// warning, includes windows
#include <windows.h>
#include <string>
 
DWORD seh_filter(EXCEPTION_POINTERS * eps, std::string & buffer);

#endif
