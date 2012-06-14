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
 *  @brief   Event Channel communication for Proxy Service
 *  @author  raczman <raczman@gmail.com>
 *  @date    2009-11-17
 *  @ingroup Proxy
 */

#ifndef _PROXY_EC_COMMUNICATOR_H
#define _PROXY_EC_COMMUNICATOR_H
#include <orbsvcs/CosEventCommS.h>
#include <orbsvcs/CosEventChannelAdminC.h>

namespace Morpheus
{

namespace Proxyd
{
/**
 * @brief Communicates with Event Channel
 * @sa Morpheus::Realmd::EC_Communicator
 * @todo Make a base class for Event Channel pushers and just derive from it.
 */
class EC_Communicator : private virtual POA_CosEventComm::PushConsumer
{

public:

    EC_Communicator(CORBA::ORB_ptr _orb):
        orb(CORBA::ORB::_duplicate(_orb)){};
    void connect();
    virtual void push( const CORBA::Any &data);
    virtual void disconnect_push_consumer();

    /**
     *  @brief  Announces proxy node in cluster.
     */
    void announce();

private:

    CosEventChannelAdmin::ProxyPushConsumer_var pusher;
    CosEventChannelAdmin::ProxyPushSupplier_var supplier_proxy;
    PortableServer::POA_var poa;
    CORBA::ORB_var orb;
};


};
};
#endif
