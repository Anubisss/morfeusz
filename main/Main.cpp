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

#include <ace/OS.h>
#include <ace/Arg_Shifter.h>
#include <ace/ARGV.h>
#include <ace/Process_Manager.h>
#include <ace/Time_Value.h>

#include "Service_Manager.h"
#include "Realm_Service.h"
#include "Proxy_Service.h"

#if PLATFORM == PLATFORM_WIN32
extern "C"
{
    #ifdef _DEBUG
        #pragma  comment (lib,"ACEd.lib")
    #else
        #pragma  comment (lib,"ACE.lib")
    #endif
}
#endif

int
ACE_TMAIN(int argc, char* argv[])
{
    if (argc < 2)
        return -1;

    ACE_ARGV args(argc, argv);

    if(ACE_OS::strcmp(args.argv()[1], "runrealmd") == 0) {
        sRealm->start();
        return 0;
        }
    else if (ACE_OS::strcmp(args.argv()[1], "rungamed") == 0) {
        sProxy->start();
        return 0;
    }

    //If we got to this point, it means we are not in slave mode. Yay!
    printf(" __  __                  _                    \n");
    printf("|  \\/  | ___  _ __ _ __ | |__   ___ _   _  ___ \n");
    printf("| |\\/| |/ _ \\| '__| '_ \\| '_ \\ / _ \\ | | |/ __|\n");
    printf("| |  | | (_) | |  | |_) | | | |  __/ |_| |\\__ \\\n");
    printf("|_|  |_|\\___/|_|  | .__/|_| |_|\\___|\\__,_|\\___/\n");
    printf("                  |_|                         \n");

    sServiceManager->args.add(args.argv());
    ACE_Arg_Shifter shifty(argc, argv); // This class ruins our argv, but we have copy in args.

    shifty.consume_arg();

    while (shifty.is_anything_left()) {
        if (ACE_OS::strcmp(shifty.get_current(), "-realm") == 0)
            sServiceManager->run_realmd();
        else if(ACE_OS::strcmp(shifty.get_current(), "-proxy") == 0)
            sServiceManager->run_proxyd();

        shifty.consume_arg();
    }

    ACE_Time_Value time;
    time.sec(1);
    while (1) {
        ACE_Process_Manager::instance()->wait(time); // for now.
        sServiceManager->update_services();
    }
}
