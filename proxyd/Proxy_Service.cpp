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
 *  @brief   Proxy_Service implementation
 *  @author  raczman <raczman@gmail.com>
 *  @date    2009-11-13
 *
 */

#include <ace/Thread_Manager.h>
#include <ace/Dev_Poll_Reactor.h>
#include "Configuration.h"
#include "Proxy_Service.h"

namespace Trinity
{
namespace Proxyd
{
void
Proxy_Service::start()
{
  //Boilerplate code goes in here.
  PROXY_LOG("Starting...\n");
  
  #if defined (ACE_HAS_EVENT_POLL) || defined (ACE_HAS_DEV_POLL)
  this->reactor = new ACE_Reactor(new ACE_Dev_Poll_Reactor());
  this->reactor->restart(1);
#else
  this->reactor = new ACE_Reactor(new ACE_TP_Reactor(), 1);
#endif
  this->acceptor = new ProxydAcceptor();

  if(this->acceptor->
     open(ACE_INET_Addr(sConfig->getString("proxyd","BindAddr").c_str()),
	  this->reactor) == -1)
    {
      PROXY_LOG("Couldn't bind to interface!\n");
      delete this->reactor;
      delete this->acceptor;
      return;
    }

  this->is_running = true;
  this->activate(THR_NEW_LWP | THR_JOINABLE,
		 sConfig->getInt("proxyd","NetThreads"));

  ACE_Thread_Manager::instance()->wait();
  return;
}

int
Proxy_Service::svc()
{

  while(this->is_running)
    {
      if(this->reactor->work_pending())
	this->reactor->handle_events();
    }
  return 0;
}

};
};
