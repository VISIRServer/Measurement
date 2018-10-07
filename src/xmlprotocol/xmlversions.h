#ifndef __XML_VERSIONS_H__
#define __XML_VERSIONS_H__

// Trick to get macro value string expansion
#define STR(x) STRX(x)
#define STRX(x) #x

#define XML_MIN_VERSION 1.3
#define XML_MAX_VERSION 1.999999

#define XML_PROTOCOL_MAJOR_VERSION 1
#define XML_PROTOCOL_MINOR_VERSION 3

#define XML_MAJOR_STR STR(XML_PROTOCOL_MAJOR_VERSION)
#define XML_MINOR_STR STR(XML_PROTOCOL_MINOR_VERSION)

#define XML_PROTOCOL_VERSION_STR XML_MAJOR_STR "." XML_MINOR_STR

#define XML_OSCILLOSCOPE_VERSION 1

#endif
