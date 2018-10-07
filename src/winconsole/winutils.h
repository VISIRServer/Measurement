#pragma once
#ifndef __WINDOWS_UTILS_H__
#define __WINDOWS_UTILS_H__

#include <string>

std::wstring WideConvertNLtoCRNL(const std::wstring& data);
std::wstring ConvertToUnicode(const std::string& data);

#endif
