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
 *  @brief   Holds implementation of 
 *           Realm_Timer class that performs
 *           timed update to realmd.
 *  @author  raczman <raczman@gmail.com>
 *  @date    2009-11-02
 *  @ingroup Realmd
 */

#include <ace/Event_Handler.h>

namespace Trinity
{
  /**
   * @brief Timer that performs tasks for realmd
   * @details This class, being an inherit from ACE_Event_Handler
   *          contains only one function, which goes off at a time
   *          set in configuration file.
   */
class Realm_Timer : public ACE_Event_Handler
{
 private:
  /**
   * @brief Inherited from ACE_Event_Handler
   * @details If there are any tasks that need to go at scheduled time,
   *          put them in here.
   */
  int handle_timeout(const ACE_Time_Value &, const void *act)
  {
    sRealm->get_db()->get_realmlist();
    sRealm->get_db()->enqueue(new SqlOperationRequest(REALMD_DB_PRUNE_BANS));
    return 0;

  }
};
}
