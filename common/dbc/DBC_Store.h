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
 *  @brief   Definition of DBC_Store singleton, as well as data-holding structures. 
 *  @author  raczman <raczman@gmail.com>
 *  @date    2009-12-29
 *
 */

#ifndef _DBC_STORE_H
#define _DBC_STORE_H

#include "Common.h"
#include "Unordered_Map.h"
#include <string>
#include <map>
#include <ace/Singleton.h>
#include <ace/Null_Mutex.h>

namespace Trinity
{

namespace DBC
{

struct ItemEntry
{
  uint32 display_id;
  uint32 inventory_type;
  uint32 sheath;
};

typedef UNORDERED_MAP<uint32, ItemEntry> item_map;

struct SpellItemEnchantmentEntry
{
  uint32 type[3];
  uint32 amount[3];
  //uint32[3]
  uint32 spell_id[3];
  std::string desc;
  //std::string description[16];
  uint32 aura_id;
  uint32 slot;
  uint32 gem_id;
  uint32 enchantment_condition;
};

typedef UNORDERED_MAP<uint32, SpellItemEnchantmentEntry> spell_item_ench_map;

class DBC_Store
{
  friend class ACE_Singleton<DBC_Store, ACE_Null_Mutex>;
 public:
  static DBC_Store* instance(){ACE_Singleton<DBC_Store, ACE_Null_Mutex>::instance();}
  void open();
  void load_item_dbc();
  void load_spell_item_enchantments_dbc();
  item_map* get_item_map(){return &items;}
 private:
  DBC_Store(){}
  std::string path;
  item_map items;
  spell_item_ench_map spell_item_enchantments;
};

};

};
#define sDBC Trinity::DBC::DBC_Store::instance()
#endif
