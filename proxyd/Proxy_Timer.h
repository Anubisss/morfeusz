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
 *  @brief   <brief>
 *  @author  <author> <<email>>
 *  @date    <date>
 *
 */

#ifndef _PROXY_TIMER_H
#define _PROXY_TIMER_H

#include <ace/Event_Handler.h>

namespace Trinity
{
namespace Proxyd
{

  class Proxy_Timer : public ACE_Event_Handler
  {
    int handle_timeout(const ACE_Time_Value &, const void*)
    {
      PROXY_LOG("Sending load report...\n");
      sProxy->get_event_channel()->report_load();
      return 0;
    }
  
  };

};
};
#endif
