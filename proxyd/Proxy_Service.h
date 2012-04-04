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
 *  @brief   Definition for Proxy_Service class.
 *  @author  raczman <raczman@gmail.com>
 *  @date    2009-11-13
 *  @ingroup Proxy
 */

#ifndef PROXY_SERVICE_H
#define PROXY_SERVICE_H

#include <tao/ORB.h>
#include <ace/Atomic_Op.h>
#include <ace/Singleton.h>
#include <ace/Task.h>
#include <ace/Acceptor.h>
#include <ace/SOCK_Acceptor.h>
#include "Proxy_Socket.h"
#include "Proxy_Database.h"

class ACE_Reactor;
using namespace Morpheus::DatabaseAccess;

namespace Morpheus
{

/**
 * @brief Proxy Service's namespace
 */
namespace Proxyd
{

    typedef ACE_Acceptor<Proxy_Socket, ACE_SOCK_ACCEPTOR> ProxydAcceptor;
    class EC_Communicator;
    typedef ACE_Atomic_Op<ACE_Recursive_Thread_Mutex, uint32> tsafe_uint32;
/**
 * @brief Proxy service main class.
 * @details Most of it is same as in Realm_Service
 * @sa Morpheus::Realmd::Realm_Service
 */
class Proxy_Service : public ACE_Task_Base
{
    friend class ACE_Singleton<Proxy_Service, ACE_Recursive_Thread_Mutex>;  

public:

    void start();
    void stop();
    static Proxy_Service* instance() { return ACE_Singleton<Proxy_Service, ACE_Recursive_Thread_Mutex>::instance(); }
    int svc();
    ACE_Reactor* get_reactor() { return this->reactor; }

    /**
     * @brief Helps keep track of opened connections.
     * @param up true for +1, false for -1
     */
    void update_connections(bool up);
  
    ProxyDB* get_db(){return this->database;}
    EC_Communicator* get_event_channel(){return event_channel;}
    uint8 get_realmid(){return this->realm_id;}
    float load;

private:

    Proxy_Service(){}
    tsafe_uint32 current_connections;
    ACE_Reactor* reactor;
    ProxydAcceptor* acceptor;
    bool is_running;
    CORBA::ORB_var orb;
    EC_Communicator* event_channel;
    ProxyDB* database;
    uint16 connection_limit;
    uint8 realm_id;
};

};
};

//Logging macros
#define PROXY_PREFIX ACE_TEXT("PROXYD: ")
#define PROXY_LOG(...) ACE_DEBUG((LM_INFO, PROXY_PREFIX __VA_ARGS__))

#ifdef _SHOW_TRACE
#define PROXY_TRACE ACE_DEBUG((LM_DEBUG,"%s\n", __PRETTY_FUNCTION__))
#else
#define PROXY_TRACE NULL
#endif

#define sProxy Morpheus::Proxyd::Proxy_Service::instance()
#endif //PROXY_SERVICE_H
