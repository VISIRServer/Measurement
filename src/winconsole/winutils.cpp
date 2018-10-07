#include "stdafx.h"
#include "winutils.h"

#include <vector>

std::wstring WideConvertNLtoCRNL(const std::wstring& data)
{
	std::wstring out;

	for(std::wstring::const_iterator it = data.begin(); it != data.end(); it++)
	{
		if (*it == L'\r')
		{
			out.push_back(*it);
			it++;
			if (it != data.end())
			{
				if (*it == L'\n')
				{
					out.push_back(*it);
					continue; // found L"\r\n"
				}
			}
			//else we should insert a final \n i guess
		}
		else if (*it == L'\n')
		{
			out.push_back(L'\r');
			out.push_back(*it);
		}
		else
		{
			out.push_back(*it);
		}
	}

	return out;
}

std::wstring ConvertToUnicode(const std::string& data)
{
	// returns the length or 0 if unsuccessful
	int rv = MultiByteToWideChar(CP_ACP, 0, data.c_str(), data.size(), NULL, 0);
	if (rv == 0) return L"";

	std::vector<TCHAR> wcdata(rv);
	rv = MultiByteToWideChar(CP_ACP, 0, data.c_str(), data.size(), &wcdata[0], wcdata.size());
	if (rv == 0) return L"";

	std::wstring out;
	out.insert(out.end(), wcdata.begin(), wcdata.end());
	return out; // copy out
}
