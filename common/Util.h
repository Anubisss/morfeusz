/*
 * Copyright (C) 2009 Dawn of Reckoning project <http://www.dorproject.net/>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */

#pragma once
#ifndef _UTIL_H
#define _UTIL_H

#include "Common.h"
#include "MTRand.h"
#include <ace/Log_Msg.h>
#include <sstream>
#include <string.h>
#include <vector>
#include <algorithm>


namespace Utils
{

inline uint32 urand32()
{

  return (uint32)sRand->genrand_int32();

}

inline std::vector<std::string> StrSplit(const std::string &src, const std::string &sep)
{
    std::vector<std::string> r;
    std::string s;
    for (std::string::const_iterator i = src.begin(); i != src.end(); i++)
    {
        if (sep.find(*i) != std::string::npos)
        {
            if (s.length())
                r.push_back(s);
            s = "";
        }
        else
        {
            s += *i;
        }
    }
    if (s.length())
        r.push_back(s);
    return r;
}

inline void hexdump(void *pAddressIn, long  lSize)
{
    std::ostringstream stm;
    stm << std::endl;
    char szBuf[100];
    long lIndent = 1;
    long lOutLen, lIndex, lIndex2, lOutLen2;
    long lRelPos;
    struct
    {
        char *pData;
        unsigned long lSize;
    } buf;
    unsigned char *pTmp,ucTmp;
    unsigned char *pAddress = (unsigned char *)pAddressIn;

    buf.pData   = (char *)pAddress;
    buf.lSize   = lSize;

    while (buf.lSize > 0)
    {
        pTmp     = (unsigned char *)buf.pData;
        lOutLen  = (int)buf.lSize;
        if (lOutLen > 16)
            lOutLen = 16;

        // create a 64-character formatted output line:
        sprintf(szBuf, " >                            "
                "                      "
                "    %08lX", (long unsigned int)(pTmp-pAddress));
        lOutLen2 = lOutLen;

        for (lIndex = 1+lIndent, lIndex2 = 53-15+lIndent, lRelPos = 0;
                lOutLen2;
                lOutLen2--, lIndex += 2, lIndex2++
            )
        {
            ucTmp = *pTmp++;

            sprintf(szBuf + lIndex,"%02X ", (unsigned short)ucTmp);
            if (!isprint(ucTmp))
                ucTmp = '.'; // nonprintable char
            szBuf[lIndex2] = ucTmp;

            if (!(++lRelPos & 3))     // extra blank after 4 bytes
            {
                lIndex++;
                szBuf[lIndex+2] = ' ';
            }
        }

        if (!(lRelPos & 3))
            lIndex--;

        szBuf[lIndex  ]   = '<';
        szBuf[lIndex+1]   = ' ';

        stm << szBuf << std::endl;


        buf.pData   += lOutLen;
        buf.lSize   -= lOutLen;
    }
    ACE_TRACE((LM_TRACE,stm.str().c_str()));
}
inline std::string strToLowerCase(std::string inString)
{
    std::transform(inString.begin(), inString.end(), inString.begin(),static_cast < int(*)(int) > (tolower));
    return inString;
}
inline std::string strToUpperCase(std::string inString)
{
    std::transform(inString.begin(), inString.end(), inString.begin(),static_cast < int(*)(int) > (toupper));
    return inString;
}

/// \todo Re-write this HORRID piece of code. Written in a hurry.
inline std::string byteArrayToAsciiString(const unsigned char *inArray, const int inLen)
{
    std::string outString;
    char tmp[255];
    for (int i=0; i< inLen; i++)
    {
        sprintf(&tmp[0], "%02X", inArray[i]);
        outString.push_back(tmp[0]);
        outString.push_back(tmp[1]);
    }
    return outString;
}
inline void asciiHexStringtoByteArray(std::string inString, unsigned char* outArray)
{
    int length = inString.length();
    // make sure the input string has an even digit numbers
    if (length%2 == 1)
    {
        return;
    }

    int size = length/2;

    std::stringstream sstr(inString);
    for (int i=0; i < size; i++)
    {
        char ch1, ch2;
        sstr >> ch1 >> ch2;
        int dig1, dig2;
        if (isdigit(ch1))
            dig1 = ch1 - '0';
        else
            if (ch1>='A' && ch1<='F')
                dig1 = ch1 - 'A' + 10;
            else
                if (ch1>='a' && ch1<='f')
                    dig1 = ch1 - 'a' + 10;
                else	//this is a failure case, if we get here we've been given a malformed Hex string!
                    dig1 = 0;

        if (isdigit(ch2))
            dig2 = ch2 - '0';
        else
            if (ch2>='A' && ch2<='F')
                dig2 = ch2 - 'A' + 10;
            else
                if (ch2>='a' && ch2<='f')
                    dig2 = ch2 - 'a' + 10;
                else	//this is a failure case, if we get here we've been given a malformed Hex string!
                    dig2 = 0;
        outArray[i] = dig1*16 + dig2;
    }
}


inline std::string makeSalt(const char* noun)
{
    uint8 salt[8];
    memset(salt,0,8);
    SNPRINTF((char*)salt,8,"%s",noun);
    return byteArrayToAsciiString(salt,8);
}

#include<algorithm>

namespace ByteConverter
{
    template<size_t T>
      inline void convert(char *val)
      {
	std::swap(*val, *(val + T - 1));
        convert<T - 2>(val + 1);
      }

    template<> inline void convert<0>(char *) {}
    template<> inline void convert<1>(char *) {}            // ignore central byte

    template<typename T> inline void apply(T *val)
      {
        convert<sizeof(T)>((char *)(val));
      }
}

#if TRINITY_ENDIAN == TRINITY_BIGENDIAN
template<typename T> inline void EndianConvert(T& val) { ByteConverter::apply<T>(&val); }
template<typename T> inline void EndianConvertReverse(T&) { }
#else
template<typename T> inline void EndianConvert(T&) { }
template<typename T> inline void EndianConvertReverse(T& val) { ByteConverter::apply<T>(&val); }
#endif
 
template<typename T> void EndianConvert(T*);         // will generate link error
template<typename T> void EndianConvertReverse(T*);  // will generate link error


}//namespace Utils
#endif
