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

namespace Morpheus
{

namespace Realmd
{

void Realm_Service::start()
{
    REALM_TRACE;
    REALM_LOG("Starting realmd\n");
    
    sLog->initialize(LOG_TYPE_REALMD);
    sLog->outDetail(LOG_FILTER_SYSTEM, "Log system initialized.");

#if defined (ACE_HAS_EVENT_POLL) || defined (ACE_HAS_DEV_POLL)
    this->reactor = new ACE_Reactor(new ACE_Dev_Poll_Reactor());
    this->reactor->restart(1);
#else
    this->reactor = new ACE_Reactor(new ACE_TP_Reactor(), 1);
#endif

    this->acceptor = new RealmdAcceptor();
  
    if (this->acceptor->open(ACE_INET_Addr(sConfig->getString("realmd", "BindAddr").c_str()), this->reactor) == -1) {
        REALM_LOG("Couldn't bind to interface!\n");
        delete reactor;
        delete acceptor;
        return;
    }
 
    this->database = new RealmDB(sConfig->getInt("realmd", "DBThreads"));
    this->database->open(sConfig->getString("realmd", "DBengine"),sConfig->getString("realmd", "DBUrl") );

    ACE_ARGV *orb_args = new ACE_ARGV;
    try
    {
        orb_args->add("");
        orb_args->add("-ORBInitRef");
        std::string str = "NameService=";
        str += sConfig->getString("corba","NSLocation");
        str += "/NameService";
        orb_args->add(str.c_str());
        int argc = orb_args->argc();
        REALM_LOG("Using %s\n", str.c_str());
        this->orb = CORBA::ORB_init(argc, orb_args->argv(),NULL);
        event_channel = new EC_Communicator(this->orb.in());
        event_channel->connect();
    }
    catch (CORBA::Exception &e)
    {
        ACE_DEBUG((LM_ERROR, "CORBA exception!\n"));
        delete orb_args;
        return;
    }

    delete orb_args;

    this->is_running = true;
    this->activate(THR_NEW_LWP | THR_JOINABLE, sConfig->getInt("realmd", "NetThreads"));
    this->database->get_realmlist();
    this->reactor->schedule_timer(new Unban_Timer(), 0, ACE_Time_Value(1), ACE_Time_Value(60));
    REALM_LOG("Started\n");
    ACE_Thread_Manager::instance()->wait();
    return;
}

void Realm_Service::update_realms(Morpheus::SQL::ResultSet* res)
{
    REALM_TRACE;
    REALM_LOG("Updating realms\n");

    if (!res)
        return;

    Realm rlm;
    uint32 id;
    while (res->next()) {
        rlm.name = res->getString(2);
        rlm.icon = res->getUint8(3);
        rlm.color = res->getUint8(4);
        rlm.timezone = res->getUint8(5);
        rlm.allowedSecurityLevel = res->getUint8(6);
        rlm.population = res->getFloat(7);
        rlm.build = res->getUint16(8);
        id = res->getUint32(1);
        this->realm_map[id] = rlm;

        REALM_LOG("Added realm %s (ID: %u) %f\n", rlm.name.c_str(), id, rlm.population);
    }
}

int Realm_Service::svc()
{
    REALM_TRACE;
    ACE_Time_Value tm;
  
    while (this->is_running) {
        tm.msec(100);
        this->reactor->handle_events(tm);
        tm.msec(100);
        if (this->orb->work_pending())
            this->orb->run(tm);
    }
    
    return 0;
}

void Realm_Service::stop()
{
    REALM_TRACE;
    this->is_running = false;
    this->reactor->end_reactor_event_loop();
    this->database->close();
}

void Realm_Service::add_proxy(uint8 realm, std::string ip, float load)
{
    REALM_TRACE;
    if (realm_map.find(realm) == realm_map.end())
        return;

    std::pair<std::multimap<uint8, Proxy_Info>::iterator, 
        std::multimap<uint8, Proxy_Info>::iterator> ret;  //Fuck you stl.
  
    std::multimap<uint8, Proxy_Info>::iterator itr;
    ret = proxies.equal_range(realm);
  
    //Ignore nodes we know about.
    for (itr = ret.first;itr != ret.second; ++itr) {
        if (!itr->second.ip.compare(ip)) {
            itr->second.load = load;
            return;
        }
    }
  
    Proxy_Info info;
    info.ip = ip;
    info.load = 0;
    proxies.insert(std::pair<uint8, Proxy_Info>(realm,info));
    REALM_LOG("Received new proxy server for realm %u: %s\n",realm,ip.c_str());
}

void Realm_Service::add_proxy_load_report(std::string ip, float load)
{
    REALM_TRACE;
    std::multimap<uint8, Proxy_Info>::iterator itr;
    for (itr = proxies.begin();itr != proxies.end(); ++itr) {
        if(!itr->second.ip.compare(ip)) {
            itr->second.load = load;
            return;
        }
    }

    REALM_LOG("Received load report for node (%s) that is not registered as proxy server. This shouldn't happen.\n");
}

std::string Realm_Service::get_proxy_for_realm(uint8 id)
{
    // get valid proxys for the realm id
    std::list<Proxy_Info> proxies_for_realm;
    for (std::multimap<uint8, Proxy_Info>::const_iterator itr = proxies.begin();
         itr != proxies.end();
         ++itr)
        if (itr->first == id)
            proxies_for_realm.push_back(itr->second);

    // there's no proxy for that realm
    if (proxies_for_realm.empty())
        return "";

    // select the most lowest load
    std::list<Proxy_Info>::const_iterator lowestLoad = proxies_for_realm.begin();
    for (std::list<Proxy_Info>::const_iterator itr = lowestLoad;
         itr != proxies_for_realm.end();
         ++itr)
        if (itr->load < lowestLoad->load)
            lowestLoad = itr;

    return lowestLoad->ip;
}

};
};
