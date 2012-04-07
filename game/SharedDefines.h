/* -*- C++ -*-
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
 
#ifndef _SHARED_DEF_H
#define _SHARED_DEF_H

enum DBCRaces
{
    RACE_HUMAN      = 1,
    RACE_ORC        = 2,
    RACE_DWARF      = 3,
    RACE_NIGHTELF   = 4,
    RACE_UNDEAD     = 5,
    RACE_TAUREN     = 6,
    RACE_GNOME      = 7,
    RACE_TROLL      = 8,
    //RACE_GOBLIN     = 9,
    RACE_BLOODELF   = 10,
    RACE_DRAENEI    = 11,
    RACE_MAX
};

enum DBCClasses
{
    CLASS_WARRIOR   = 1,
    CLASS_PALADIN   = 2,
    CLASS_HUNTER    = 3,
    CLASS_ROGUE     = 4,
    CLASS_PRIEST    = 5,
    CLASS_SHAMAN    = 7,
    CLASS_MAGE      = 8,
    CLASS_WARLOCK   = 9,
    CLASS_DRUID     = 11,
    CLASS_MAX
};

enum GuidTypes
{
    GUID_CHAR       = 0,
    GUID_MAX
};

#endif // _SHARED_DEF_H
