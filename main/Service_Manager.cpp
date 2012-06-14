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

#include "Service_Manager.h"
#include <ace/Process_Manager.h>

bool Service_Manager::run_realmd()
{
    ACE_ARGV realmd_args;
    realmd_args.add(this->args.argv()[0]);
    realmd_args.add("runrealmd");

    ACE_Process_Manager* pmgr = ACE_Process_Manager::instance();
    ACE_Process_Options pop;
    pop.command_line(realmd_args.argv());
    pid_t realmpid = pmgr->spawn(pop);
    if (realmpid == ACE_INVALID_PID)
        return false;

    ServiceInfo *si = new ServiceInfo(realmpid);
    this->svcs.insert(std::pair<MorpheusServices, ServiceInfo*>(LOGINSERVER, si));
    ACE_DEBUG((LM_DEBUG,"Realmd runs at pid %u\n", realmpid));
    return true;
}

bool Service_Manager::run_proxyd()
{
    ACE_ARGV proxyd_args;
    proxyd_args.add(this->args.argv()[0]);
    proxyd_args.add("rungamed");

    ACE_Process_Manager* pmgr = ACE_Process_Manager::instance();
    ACE_Process_Options pop;
    pop.command_line(proxyd_args.argv());
    pid_t proxyd_pid = pmgr->spawn(pop);
    if (proxyd_pid == ACE_INVALID_PID)
        return false;

    ServiceInfo* si = new ServiceInfo(proxyd_pid);
    this->svcs.insert(std::pair<MorpheusServices, ServiceInfo*>(GAMESERVER, si));
    ACE_DEBUG((LM_DEBUG,"Proxyd runs at pid %u\n", proxyd_pid));
    return true;
}

void Service_Manager::update_services()
{
    std::map<MorpheusServices, ServiceInfo*>::iterator iter;

    for (iter = svcs.begin();iter != svcs.end(); iter++) {
        if (iter->second->status == OFF)
            continue;

        bool is_dead = false;
        char path[ 6 + 6 + 5];
        ACE_OS::sprintf(path,"/proc/%u/stat", iter->second->pid);
        ACE_HANDLE status_file = ACE_OS::open(path, GENERIC_READ);

        if (status_file == ACE_INVALID_HANDLE) {
            ACE_DEBUG((LM_DEBUG,"Service has crashed.\n"));
            is_dead = true;
        }

        if (!is_dead) {
            char *buf = new char[100]; //please excuse me this waste of bytes.
            char delim[] = " ";
            ACE_OS::read(status_file,buf,99);
            strtok(buf,delim);
            strtok(NULL,delim);
            char * status = strtok(NULL, delim);
            if (*status == 'Z') {
                ACE_DEBUG((LM_DEBUG, "Service is stopped.\n"));
                is_dead = true;
                ACE_OS::kill(iter->second->pid, SIGKILL);
            }
            delete[] buf;
        }

        if (is_dead) {
            switch(iter->first) {
            case LOGINSERVER:
                ACE_DEBUG((LM_DEBUG,"Restarting Realm Service\n"));
                this->run_realmd();
                break;
            case GAMESERVER:
                ACE_DEBUG((LM_DEBUG,"Restarting Proxy Service\n"));
                this->run_proxyd();
                break;
            default:
                break;
            }

            delete iter->second;
            this->svcs.erase(iter++); // I'm not sure this is correct - it might skip the next service

            continue;
        }

        ACE_OS::close(status_file);
    }
}
