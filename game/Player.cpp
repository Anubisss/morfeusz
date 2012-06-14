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

#include "Player.h"
#include "ObjectMgr.h"
#include "Database.h"
#include "Proxy_Service.h"

namespace Morpheus
{

namespace Entities
{

bool Player::create(uint32 low_guid, uint32 account, const std::string& name, uint8 race, uint8 pclass, uint8 gender, uint8 skin, uint8 face, uint8 hair_style, uint8 hair_color, uint8 facial_hair, uint8 outfit_id, Morpheus::DatabaseAccess::SqlOperationTransaction* trans)
{
    SqlOperationRequest* op = new SqlOperationRequest(PROXYD_DB_INS_CHAR);
    op->add_uint32(1, low_guid);
    op->add_uint32(2, account);
    op->add_string(3, name.c_str());
    op->add_uint8(4, race);
    op->add_uint8(5, pclass);
    op->add_uint8(6, gender);
    op->add_uint32(7, uint32(1));

    uint32 playerBytes = (skin | (face << 8) | (hair_style << 16) | hair_color << 24);
    op->add_uint32(8, playerBytes);

    uint32 playerBytes2 = (facial_hair | (0x00 << 8) | (0x00 << 16) | (0x02 << 24)); // Magic numbers from TC
    op->add_uint32(9, playerBytes2);

    Morpheus::Managers::PlayerCreateInfo pci = sObjectMgr->get_player_create_info(race, pclass);
    printf("Data: %u %u %f %f %f\n", pci.zone_id, pci.map_id, pci.position_x, pci.position_y, pci.position_z);
    op->add_uint32(10, pci.zone_id);
    op->add_uint32(11, pci.map_id);
    op->add_float(12, pci.position_x);
    op->add_float(13, pci.position_y);
    op->add_float(14, pci.position_z);

    trans->append(op);

    return true;
}

};
};
