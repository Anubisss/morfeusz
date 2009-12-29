/* -*- C++ -*-
 * Copyright (C) 2009 Trinity Core <http://www.trinitycore.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/**
 *  @file
 *  @brief   
 *  @author  raczman <raczman@gmail.com>
 *  @date    2009-12-29
 *
 */

#ifndef _DBC_FILE_H
#define _DBC_FILE_H

#include <iostream>
#include <fstream>

#include "Common.h"

namespace Trinity
{

  /**
   * @brief Contains classes used for accessing data inside of .dbc files.
   */
namespace DBC
{

  /**
   * @brief Exception thrown when there are any problems reading DBC.
   */
class DBC_Read_Exception
{
public:
  DBC_Read_Exception(char* m){msg += m;}
  std::string msg;
};

  /**
   * @brief DBC files live here. Each DBC_File symbols a .dbc opened.
   *        The class itself serves as an abstraction layer for reading .dbc's.
   *        At the same time it will try to ensure it is as idiot proof as needed.
   *        DBC files are binary storage format used by Blizzard to hold certain data
   *        used by client. A common DBC starts with 20 byte header, with contents:
   *        - "WDBC" Signature
   *        - number of records
   *        - number of fields in each record
   *        - size of record
   *        - string block size.
   *        
   *        After header, comes data section, of size record_size*records. 
   *        Last section is string block, which contains null terminated strings.
   *        When referencing to a string, DBC just marks an offset into string bblock from which to read.
   * 
   */
class DBC_File
{
public:

  /**
   * @brief Opens up .dbc, reads header data, and if something goes wrong, throws.
   * @param src File location.
   */
  DBC_File(const char* src);
  
  /**
   * @brief Hardly ever field size in DBC is something other than 4.
   *        So we only read data that may possibly be there.
   *        There are of course exceptions to the "Rule of 4".
   */
  uint32 read_uint32();
  float read_float();
  std::string read_string();
  uint32 get_records(){return this->records;}
  void skip_field();

private:
  uint32 records;
  uint32 record_size;
  uint32 field_size;
  uint32 fields;
  
  std::ifstream file;

  /**
   * @brief size of dbc - string_block_size
   */
  uint32 data_size;
  uint32 string_block_size;

  /**
   * @brief I dont like default constructor for a reason.
   */
  DBC_File(){}
};

};
};
#endif //_DBC_FILE_H
