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

#ifndef sServiceManager
#include <ace/Singleton.h>
#include <map>
#include <ace/ARGV.h>

enum MorpheusServices
{
    LOGINSERVER = 0,
    GAMESERVER,
    ZONESERVER,
    SOCIALSERVER,
    SERVICES_MAX
};

enum ServiceStatus
{
    OFF = 0,
    ON
};

struct ServiceInfo
{
    ServiceInfo() : status(OFF), pid(0) {}
    ServiceInfo(pid_t pid) : pid(pid), status(ON) {}

    ServiceStatus status;
    pid_t pid;
};

class Service_Manager
{
    friend class ACE_Singleton<Service_Manager, ACE_Thread_Mutex>;

public:

    ACE_ARGV args;
    bool run_realmd();
    bool run_proxyd();
    void update_services();
    static Service_Manager* instance() { return ACE_Singleton<Service_Manager, ACE_Thread_Mutex>::instance(); }

private:

    Service_Manager() {};
    std::map<MorpheusServices, ServiceInfo*> svcs;
};

#define sServiceManager Service_Manager::instance()
#else
#endif
