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
 *  @brief   Declaration of Realm_Service class, and it's friends.
 *  @author  raczman <raczman@gmail.com>
 *  @date    2009-10-20
 *  @ingroup Realmd
 */

/**
 *  @defgroup Realmd Realm Service
 * 
 */

#ifndef sRealm
#include <map>

#include <ace/Singleton.h>
#include <ace/Acceptor.h>
#include <ace/SOCK_Acceptor.h>
#include <tao/corba.h>
#include "Realm_Database.h"

class ACE_Reactor;

using namespace Morpheus::DatabaseAccess;

namespace Morpheus
{

/**
 * @brief %Realm service's namespace
 * @details Contains classes and interfaces used by %Realm Service.
 */
namespace Realmd
{

typedef ACE_Acceptor<Realm_Socket, ACE_SOCK_ACCEPTOR> RealmdAcceptor;

class EC_Communicator;

/**
 * @brief This structure is used to pass information
 *        about realms around.
 */
struct Realm
{
    std::string name;
    uint8 icon;
    uint8 color;
    uint8 timezone;
    uint8 allowedSecurityLevel;
    float population;
    uint16 build;
};

struct Proxy_Info
{
    std::string ip;
    float load;
};

/**
 * @brief Realm Daemon service
 * @details This class is providing logon server
 *          functionality. It starts and shuts down networking,
 *          database connections and is responsible for 
 *          load balancing players across Game Servers.
 */
class Realm_Service : public ACE_Task_Base
{
    friend class ACE_Singleton<Realm_Service, ACE_Recursive_Thread_Mutex>;

public:

    /**
     * @brief Starts service.
     */
    void start();

    /**
     * @brief Stops service.
     */
    void stop();

    /**
     * @brief Realm_Service is a singleton, we are accessing it using mutexed instance call.
     */
    static Realm_Service* instance() { return ACE_Singleton<Realm_Service, ACE_Recursive_Thread_Mutex>::instance(); }
    ACE_Reactor* get_reactor() { return reactor; }

    /**
     * @brief Threadbody for network handling threads.
     */
    int svc();
    RealmDB* get_db() { return database; }
    std::map<uint8, Realm>* get_realmlist() { return &realm_map; }

    /**
     * @brief This is callbacked function, used to set realmlist.
     * @see RealmDB::get_realmlist()
     * @see getRealmListObsv
     */
    void update_realms(Morpheus::SQL::ResultSet*);

    void add_proxy(uint8 realm, std::string ip, float load);

    void add_proxy_load_report(std::string ip, float load);

    /**
     *  @brief    Get a proxy ip for a specific realm.
     *  @details  Select the most lowest loaded proxy and
     *            return with its address.
     *            Return empty string if there is no proxy for that realm.
     */
    std::string get_proxy_for_realm(uint8 id);

private:

    std::map<uint8, Realm> realm_map;
    std::multimap<uint8, Proxy_Info> proxies;
    Realm_Service(){}
    ACE_Reactor* reactor;
    RealmdAcceptor* acceptor;
    RealmDB* database;

    CORBA::ORB_var orb;
    EC_Communicator* event_channel;

    /**
     * @brief Used to indicate service's status.
     */
    bool is_running;
};

};
};

/**
 * @brief Prefix for log macro.
 */
#define REALM_PREFIX ACE_TEXT("REALMD: ")

/**
 * @brief Logging macro. Outputs messages in "REALMD: %s" format.
 */
#define REALM_LOG(...) ACE_DEBUG((LM_INFO, REALM_PREFIX __VA_ARGS__))

#ifdef _SHOW_TRACE
#define REALM_TRACE ACE_DEBUG((LM_DEBUG,"%s\n", __PRETTY_FUNCTION__))
#else
#define REALM_TRACE NULL
#endif
/**
 * @brief For clarity, rest of the code references realm service
 *        singleton as sRealm.
 */
#define sRealm Morpheus::Realmd::Realm_Service::instance()
#else
#endif
