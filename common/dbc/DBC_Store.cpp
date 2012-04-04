/* -*- C++ -*-
 * Copyright (C) 2009 Trinity Core <http://www.trinitycore.org>
 * Copyright (C) 2012 Morpheus
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

namespace Morpheus
{

namespace DBC
{

void DBC_Store::open()
{
    this->path = sConfig->getString("data", "dbc"); 

    if (!ACE_OS::opendir(this->path.c_str()))
        ACE_DEBUG((LM_ERROR, "Couldn't open DBC directory: %s !\n", path.c_str()));
}

void DBC_Store::load_item_dbc()
{
    std::string file = this->path;
    file += "Item.dbc";

    DBC_File* dbc = new DBC_File(file.c_str());

    for (int i = 0; i < dbc->get_records(); i++) {
        uint32 entry = dbc->read_uint32();
        ItemEntry& item = this->items[entry];
        item.display_id = dbc->read_uint32();
        item.inventory_type = dbc->read_uint32();
        item.sheath = dbc->read_uint32();
    }

    ACE_DEBUG((LM_ERROR, "Loaded %u Item.dbc entries.\n", items.size()));

    delete dbc;
}

void DBC_Store::load_spell_item_enchantments_dbc()
{
    std::string file = this->path;
    file += "SpellItemEnchantment.dbc";

    DBC_File* dbc = new DBC_File(file.c_str());

    for (int i = 0; i < dbc->get_records(); i++) {
        uint32 entry = dbc->read_uint32();
        SpellItemEnchantmentEntry& spell = this->spell_item_enchantments[entry];
        spell.type[0] = dbc->read_uint32();
        spell.type[1] = dbc->read_uint32();
        spell.type[2] = dbc->read_uint32();

        spell.amount[0] = dbc->read_uint32();
        spell.amount[1] = dbc->read_uint32();
        spell.amount[2] = dbc->read_uint32();

        dbc->skip_field();
        dbc->skip_field();
        dbc->skip_field();

        spell.spell_id[0] = dbc->read_uint32();
        spell.spell_id[1] = dbc->read_uint32();
        spell.spell_id[2] = dbc->read_uint32();

        spell.desc = dbc->read_string();

        for (int j = 0; j < 16; j++)
            dbc->skip_field();

        spell.aura_id = dbc->read_uint32();
        spell.slot = dbc->read_uint32();
        spell.gem_id = dbc->read_uint32();
        spell.enchantment_condition = dbc->read_uint32();
    }

    ACE_DEBUG((LM_ERROR, "Loaded %u SpellItemEnchantment.dbc entries.\n", this->spell_item_enchantments.size()));

    delete dbc;
}

void DBC_Store::load_chr_races_dbc()
{
    std::string file = this->path;
    file += "ChrRaces.dbc";
    
    DBC_File* dbc = new DBC_File(file.c_str());
    
    for (int i = 0; i < dbc->get_records(); i++) {
        uint32 entry = dbc->read_uint32();
        ChrRacesEntry& crentry = this->chr_races[entry];
        crentry.race_id = entry;
        dbc->skip_field();
        crentry.faction_id = dbc->read_uint32();
        dbc->skip_field();
        crentry.model_m = dbc->read_uint32();
        crentry.model_f = dbc->read_uint32();
        dbc->skip_field();
        dbc->skip_field();
        crentry.team_id = dbc->read_uint32();
        dbc->skip_field();
        dbc->skip_field();
        dbc->skip_field();
        dbc->skip_field();
        crentry.startmovie = dbc->read_uint32();
        for (int j = 0; j < 16; j++)
            crentry.name[j] = const_cast<char*>(dbc->read_string().c_str());
        for (int j = 0; j < 38; j++)
            dbc->skip_field();
        crentry.addon = dbc->read_uint32();
    }
    
    ACE_DEBUG((LM_ERROR, "Loaded %u ChrRaces.dbc entries.\n", this->chr_races.size()));
    
    delete dbc;
}

void DBC_Store::load_chr_classes_dbc()
{
    std::string file = this->path;
    file += "ChrClasses.dbc";
    
    DBC_File* dbc = new DBC_File(file.c_str());
    
    for (int i = 0; i < dbc->get_records(); i++) {
        uint32 entry = dbc->read_uint32();
        ChrClassesEntry& clentry = this->chr_classes[entry];
        clentry.class_id = entry;
        dbc->skip_field();
        clentry.powertype = dbc->read_uint32();
        dbc->skip_field();
        for (int j = 0; j < 16; j++)
            clentry.name[j] = const_cast<char*>(dbc->read_string().c_str());
        for (int j = 0; j < 36; j++)
            dbc->skip_field();
        clentry.spellfamily = dbc->read_uint32();
        dbc->skip_field();
    }
    
    ACE_DEBUG((LM_ERROR, "Loaded %u ChrClasses.dbc entries.\n", this->chr_classes.size()));
    
    delete dbc;
}

};
};
