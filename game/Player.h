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

#ifndef _PLAYER_H
#define _PLAYER_H

#include "Common.h"
#include "Database.h"
#include "SharedDefines.h"
#include <string>

namespace Morpheus
{
    
namespace Entities
{

class SqlOperationTransaction;

class Player
{

public:

    static bool create(uint32 low_guid, uint32 account, const std::string& name, uint8 race, uint8 pclass, uint8 gender, uint8 skin, uint8 face, uint8 hair_style, uint8 hair_color, uint8 facial_hair, uint8 outfit_id, Morpheus::DatabaseAccess::SqlOperationTransaction* trans);

private:


};

};
};

#endif // _PLAYER_H
