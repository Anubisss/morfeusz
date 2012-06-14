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
 *  @brief   Definition of EC_Communicator class.
 *  @author  raczman <raczman@gmail.com>
 *  @date    2009-11-12
 *  @ingroup Realmd
 */

#ifndef REALM_EC_COMMUNICATOR_H
#define REALM_EC_COMMUNICATOR_H
#include <orbsvcs/CosEventCommS.h>
#include <orbsvcs/CosEventChannelAdminC.h>

namespace Morpheus
{

namespace Realmd
{

/**
 * @brief This class is responsible for communicating with Event Channel
 */
class EC_Communicator : public POA_CosEventComm::PushConsumer
{

public:

    EC_Communicator(CORBA::ORB_ptr _orb):
        orb(CORBA::ORB::_duplicate(_orb)){};

    /**
     * @brief Connects to event channel.
     */
    void connect();

    /**
     * @brief This function receives data pushed by other nodes
     */
    void push( const CORBA::Any &data);

    /**
     * @brief called by event channel when we are disconnected from channel
     */
    void disconnect_push_consumer();

    /**
     *  @brief  Request proxies for all realms.
     */
    void request_proxies();

    /**
     * @brief Realm Service calls this function to poll for proxy nodes
     */
    void request_proxies_for_realm(uint8 id);

private:

    CORBA::ORB_var orb;
    CosEventChannelAdmin::ProxyPushConsumer_var pusher;
    CosEventChannelAdmin::ProxyPushSupplier_var supplier_proxy;
    PortableServer::POA_var poa;
};

};
};

#endif //REALM_EC_COMMUNICATOR_H
