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
 *  @brief   Implementation of Realm_Service
 *  @author  raczman <raczman@gmail.com>
 *  @date    2009-10-20
 *  @ingroup Realmd
 */


#include "Realm_Service.h"
#include "Realm_Socket.h"
#include "Configuration.h"
#include "Realm_Database.h"
#include "Realm_Timer.h"
#include "Realm_EC_Communicator.h"
#include <sstream>
#include <ace/TP_Reactor.h>
#include <ace/Dev_Poll_Reactor.h>
#include <ace/ARGV.h>

namespace Trinity
{
namespace Realmd
{
void
Realm_Service::start()
{

  REALM_LOG("Starting realmd\n");

#if defined (ACE_HAS_EVENT_POLL) || defined (ACE_HAS_DEV_POLL)
  this->reactor = new ACE_Reactor(new ACE_Dev_Poll_Reactor());
  this->reactor->restart(1);
#else
  this->reactor = new ACE_Reactor(new ACE_TP_Reactor(), 1);
#endif

  this->acceptor = new RealmdAcceptor();
  
  if(this->acceptor->open(ACE_INET_Addr(sConfig->getString("realmd", "BindAddr").c_str()), this->reactor) == -1)
    {
      REALM_LOG("Couldn't bind to interface!\n");
      delete reactor;
      delete acceptor;
      return;
    }
  ACE_Time_Value tm(1 * 60 * sConfig->getInt("realmd","UpdateInterval"));
 
  this->database = new RealmDB(sConfig->getInt("realmd", "DBThreads"));
  this->database->open(sConfig->getString("realmd", "DBengine"),sConfig->getString("realmd", "DBUrl") );
  this->database->get_realmlist();

  ACE_ARGV *orb_args = new ACE_ARGV;
  orb_args->add("");
  orb_args->add("-ORBInitRef");
  std::string str = "NameService=";
  str += sConfig->getString("corba","NSLocation");
  str += "/NameService";
  orb_args->add(str.c_str());
  int argc = orb_args->argc();
  REALM_LOG("%s\n", str.c_str());
  this->orb = CORBA::ORB_init(argc, orb_args->argv(),NULL);
  delete orb_args;
  event_channel = new EC_Communicator(this->orb.in());
  event_channel->connect();

  this->is_running = true;
  this->activate(THR_NEW_LWP | THR_JOINABLE, sConfig->getInt("realmd", "NetThreads"));
  this->reactor->schedule_timer(new Realm_Timer(), 0, tm, tm);
  this->reactor->schedule_timer(new Unban_Timer(), 0, ACE_Time_Value(1), ACE_Time_Value(60));
  REALM_LOG("Started\n");
  ACE_Thread_Manager::instance()->wait();
  return;
}

void
Realm_Service::update_realms(Trinity::SQL::ResultSet* res)
{
  REALM_LOG("Updating realms\n");

  if(!res)
    return;
  Realm rlm;
  uint32 id;
  while(res->next())
    {
      std::ostringstream oss;
      oss.clear();
      rlm.name = res->getString(2); 
      oss << res->getString(3) << ":" << res->getUint16(4);
      rlm.address = oss.str();
      rlm.icon = res->getUint8(5);
      rlm.color = res->getUint8(6);
      rlm.timezone = res->getUint8(7);
      rlm.allowedSecurityLevel = res->getUint8(8);
      rlm.population = res->getFloat(9);
      rlm.build = res->getUint16(10);
      id = res->getUint32(1);
      this->realm_map[id] = rlm;
      REALM_LOG("Added realm %s (ID: %u) %f\n",rlm.name.c_str(), res->getUint8(1), rlm.population);
    }

}

int 
Realm_Service::svc()
{
  while(this->is_running)
    {
      if(this->reactor->work_pending())
	this->reactor->handle_events();
      if(this->orb->work_pending())
	this->orb->perform_work();
    }
  return 0;
}

void
Realm_Service::stop()
{
  this->is_running = false;
  this->reactor->end_reactor_event_loop();
  this->database->close();

}

void
Realm_Service::add_proxy(uint8 realm, std::string ip, float load)
{

  std::pair<std::multimap<uint8, Proxy_Info>::iterator, 
    std::multimap<uint8, Proxy_Info>::iterator> ret;  //Fuck you stl.
  
  std::multimap<uint8, Proxy_Info>::iterator itr;
  ret = proxies.equal_range(realm);
  
  //Ignore nodes we know about.
  for(itr = ret.first;itr != ret.second; ++itr)
    if(!itr->second.ip.compare(ip))
      {
	itr->second.load = load;
	return;
      }
  
  Proxy_Info info;
  info.ip = ip;
  info.load = 0;
  
}

void
Realm_Service::add_proxy_load_report(std::string ip, float load)
{

  std::multimap<uint8, Proxy_Info>::iterator itr;
  for(itr = proxies.begin();itr != proxies.end(); ++itr)
    {
      if(!itr->second.ip.compare(ip))
	{
	  itr->second.load = load;
	  return;
	}
    }

}

};
};
