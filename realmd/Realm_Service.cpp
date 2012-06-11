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

    if (!res)
        return;

    REALM_LOG("Updating realms\n");

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
    this->event_channel->request_proxies();
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

void Realm_Service::process_proxy_announce(Morpheus::Proxy_Announce const* announce)
{
    REALM_TRACE;

    uint8 realm_id = announce->realm_id;
    std::string address = CORBA::string_dup(announce->address);
    float load = announce->load;

    // invalid realm ID
    if (realm_map.find(realm_id) == realm_map.end())
    {
        REALM_LOG("Realm doesn't exist: %u\n", realm_id);
        return;
    }

    bool newProxy = false;
    // definitely new proxy
    if (proxies.find(realm_id) == proxies.end())
        newProxy = true;
    // maybe new maybe not
    else
    {
        newProxy = true;
        for (std::multimap<uint8, Proxy_Info>::const_iterator itr = proxies.begin();
             itr != proxies.end();
             ++itr)
        {
            // realm ID doesn't match
            if (itr->first != realm_id)
            {
                // have another proxy with the same address but not same realm ID
                // all these 2 proxies should be deleted
                // and request new ones for these realm
                if (itr->second.ip == address)
                {

#ifdef _MORPHEUS_DEBUG
                    REALM_LOG("Proxy (%s) duplication, both proxies deleted and requesting new ones for the realms: %s (%u) - %s (%u)\n",
                              address.c_str(),
                              realm_map[itr->first].name.c_str(),
                              itr->first,
                              realm_map[realm_id].name.c_str(),
                              realm_id);
#endif

                    delete_proxy(address);
                    event_channel->request_proxies_for_realm(itr->first);
                    event_channel->request_proxies_for_realm(realm_id);
                    return;
                }
                continue;
            }
            if (itr->second.ip == address)
            {
                newProxy = false;
                break;
            }
        }
    }

    if (newProxy)
        add_proxy(realm_id, address, load);
    else
        update_proxy_load(address, load);
}

void Realm_Service::add_proxy(uint8 realm, std::string ip, float load)
{
    REALM_TRACE;
    Proxy_Info info;
    info.ip = ip;
    info.load = load;
    proxies.insert(std::pair<uint8, Proxy_Info>(realm, info));
    REALM_LOG("Received new proxy server for realm %s (%u): %s\n",
              realm_map[realm].name.c_str(),
              realm,
              ip.c_str());
}

void Realm_Service::update_proxy_load(std::string ip, float load)
{
    REALM_TRACE;
    for (std::multimap<uint8, Proxy_Info>::iterator itr = proxies.begin();
         itr != proxies.end();
         ++itr)
    {
        if (itr->second.ip == ip)
        {

#ifdef _MORPHEUS_DEBUG
            REALM_LOG("%s | Proxy load updated: %f -> %f\n",
                      ip.c_str(),
                      itr->second.load,
                      load);
#endif

            itr->second.load = load;
            break;
        }
    }
}

void Realm_Service::delete_proxy(std::string ip)
{
    REALM_TRACE;
    bool deleted = false;
    for (std::multimap<uint8, Proxy_Info>::iterator itr = proxies.begin();
         itr != proxies.end();)
    {
        if (itr->second.ip == ip)
        {
            proxies.erase(itr++);
            deleted = true;
        }
        else
            ++itr;
    }
    ASSERT(deleted); // shouldn't happen
    REALM_LOG("Proxy %s deleted\n", ip.c_str());
}

std::string Realm_Service::get_proxy_for_realm(uint8 id)
{
    // get proxies for the same realm id
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
