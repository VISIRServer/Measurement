#ifndef __BASE_64_H__
#define __BASE_64_H__

#include <string>

namespace base64
{
	std::string base64_encode(unsigned char const* in , unsigned int len);
	std::string base64_decode(const std::string & in);
} // end of namespace

#endif
