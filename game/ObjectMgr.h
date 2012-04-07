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
 
#ifndef _OBJECTMGR_H
#define _OBJECTMGR_H

#include "Database.h"
#include "SharedDefines.h"
#include <ace/Singleton.h>

namespace Morpheus
{
    
namespace Managers
{

struct PlayerCreateInfo
{
    uint32 map_id;
    uint32 zone_id;
    float position_x;
    float position_y;
    float position_z;
};

class ObjectMgr
{
    friend class ACE_Singleton<ObjectMgr, ACE_Recursive_Thread_Mutex>;
    
public:

    static ObjectMgr* instance() { return ACE_Singleton<ObjectMgr, ACE_Recursive_Thread_Mutex>::instance(); }
    void player_create_info_loaded(uint8 loaded);
    void set_player_create_info(uint8 race, uint8 pclass, PlayerCreateInfo info) { playerCreateInfo[race - 1][pclass - 1] = info; }
    const PlayerCreateInfo& get_player_create_info(uint8 race, uint8 pclass) { return playerCreateInfo[race - 1][pclass - 1]; }
    virtual ~ObjectMgr()
    {
        /*for (int i = 0; i < RACE_MAX; i++) {
            for (int j = 0; j < CLASS_MAX; j++)
                delete playerCreateInfo[i][j];
        }*/
    }
    
private:

    ObjectMgr()
    {
        /*playerCreateInfo[RACE_MAX][CLASS_MAX] = new PlayerCreateInfo[RACE_MAX * CLASS_MAX];
        for (int i = 0; i < RACE_MAX; i++) {
            for (int j = 0; j < CLASS_MAX; j++)
                playerCreateInfo[i][j] = new PlayerCreateInfo;
        }*/
    }
    
    PlayerCreateInfo playerCreateInfo[RACE_MAX][CLASS_MAX];
    
};
    
};
};

#define sObjectMgr Morpheus::Managers::ObjectMgr::instance()
#endif // _OBJECTMGR_H
