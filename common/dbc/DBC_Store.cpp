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

#include <ace/OS_Dirent.h>
#include "DBC_Store.h"
#include "DBC_File.h"
#include "Configuration.h"

namespace Trinity
{

namespace DBC
{

void
DBC_Store::open()
{

  this->path = sConfig->getString("data","dbc"); 
  
  if(!ACE_OS::opendir(this->path.c_str()))
    {
      ACE_DEBUG((LM_ERROR,"Couldn't open DBC directory: %s !\n",path.c_str()));
    }

}

void
DBC_Store::load_item_dbc()
{
  std::string file = this->path;
  file += "Item.dbc";

  DBC_File* dbc = new DBC_File(file.c_str());

  for(int i = 0; i < dbc->get_records(); i++)
    {
      uint32 entry = dbc->read_uint32();
      items_map[entry].display_id = dbc->read_uint32();
      items_map[entry].inventory_type = dbc->read_uint32();
      items_map[entry].sheath = dbc->read_uint32();
    }

  ACE_DEBUG((LM_ERROR,"Loaded %u Item.dbc entries\n",items_map.size()));

  delete dbc;
}

};
};
