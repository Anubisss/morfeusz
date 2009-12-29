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
 *  @brief   Implementation of Data_Store class.
 *  @author  raczman <raczman@gmail.com>
 *  @date    2009-12-29
 *
 */


#include "DBC_File.h"

namespace Trinity
{

namespace DBC
{

DBC_File::DBC_File(const char* src)
{
  this->file.open(src, std::fstream::binary);
  if(!file)
    throw new DBC_Read_Exception("Could not open file!");

  if(this->read_uint32() != 0x43424457)
    throw new DBC_Read_Exception("File is not a valid DBC store.");
  
  this->records = this->read_uint32();
  this->fields = this->read_uint32();
  this->record_size = this->read_uint32();
  this->string_block_size = this->read_uint32();
  this->field_size = this->record_size / this->fields;

  // We will be using this as a pointer into beggining of string table
  // So, let's get down to math!
  // Complete data size is records times records size. We add header size to it
  // And size of NULL that precedes string block.
  this->data_size = (this->records * this->record_size) + 20 + 1;
  
}
  
uint32
DBC_File::read_uint32()
{
  uint32 ret;
  file.read(reinterpret_cast<char*>(&ret), 4);
  return ret;
}

float
DBC_File::read_float()
{
  float ret;
  file.read(reinterpret_cast<char*>(&ret), 4);
  return ret;
}

std::string
DBC_File::read_string()
{
  if(this->string_block_size <= 1)
    throw new DBC_Read_Exception("DBC doesn't have string block.");
  
  
  uint32 offset = this->read_uint32();
  uint32 get_ptr = this->file.tellg();

  this->file.seekg(this->data_size + offset);

  std::string ret;
  char c;

  while(1)
    {
      file.read(&c, 1);
      if(c == '\00')
	break;
      ret += c;
    }

  this->file.seekg(get_ptr);
  return ret;
}

void
DBC_File::skip_field()
{

  file.seekg(this->field_size, std::ios_base::cur);

}

};
};
