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
 *  @brief   Realmd::EC_Communicator implementation.
 *  @author  raczman <raczman@gmail.com>
 *  @date    2009-11-13
 *
 */
#include "Realm_Service.h"
#include "Realm_EC_Communicator.h"
#include "idl/Proxy_EventsC.h"
#include <orbsvcs/CosNamingC.h>

namespace Morpheus
{

namespace Realmd
{

void EC_Communicator::connect()
{
    try {
        REALM_LOG("Connecting to Event Channel...\n");
        CORBA::Object_var _object =
            orb->resolve_initial_references("NameService");
        CosNaming::NamingContext_var naming_context =
            CosNaming::NamingContext::_narrow (_object.in ());

        CosNaming::Name ec_name;
        ec_name.length(1);
        ec_name[0].id = CORBA::string_dup("CosEventService");

        CosEventChannelAdmin::EventChannel_var channel =
            CosEventChannelAdmin::EventChannel::_narrow(naming_context->resolve(ec_name));

        _object = orb->resolve_initial_references("RootPOA");
        this->poa = PortableServer::POA::_narrow(_object.in());

        PortableServer::ObjectId_var oid = poa->activate_object(this);
        CORBA::Object_var consumer_obj = poa->id_to_reference(oid.in());

        CosEventComm::PushConsumer_var consumer =
            CosEventComm::PushConsumer::_narrow(consumer_obj.in());
        this->supplier_proxy = channel->for_consumers()->obtain_push_supplier();
        this->supplier_proxy->connect_push_consumer(consumer.in());

        this->pusher = channel->for_suppliers()->obtain_push_consumer();
        this->pusher->connect_push_supplier(CosEventComm::PushSupplier::_nil());
        poa->the_POAManager()->activate();

        REALM_LOG("Connected to Event Channel.\n");
    }
    catch (CORBA::Exception &e) {
        REALM_LOG("Couldn't connect to Event Channel! - Exception thrown was of type: %s\n", e._name());
        throw;
        return;
    }
}

void EC_Communicator::request_proxies()
{
    REALM_TRACE;
    sRealm->delete_proxies(); // to make sure it's clean
    std::map<uint8, Realm> const* realms = sRealm->get_realmlist();
    for (std::map<uint8, Realm>::const_iterator itr = realms->begin();
         itr != realms->end();
         ++itr)
        request_proxies_for_realm(itr->first);
}

void EC_Communicator::request_proxies_for_realm(uint8 id)
{
    REALM_TRACE;
    Morpheus::Proxy_Request req;
    req.realm_id = id;

    CORBA::Any any;
    any <<= req;
    this->pusher->push(any);
}

void EC_Communicator::push(const CORBA::Any &data)
{
    REALM_TRACE;
    CORBA::TypeCode_ptr dataType = data._tao_get_typecode();
    if (dataType->equal(Morpheus::_tc_Proxy_Announce))
    {
        Morpheus::Proxy_Announce* ann;
        if (data >>= ann)
        {
#ifdef _MORPHEUS_DEBUG
            REALM_LOG("[EVENT] %s | ID: %u Address: %s Load: %f\n",
                      dataType->name(),
                      ann->realm_id,
                      CORBA::string_dup(ann->address),
                      ann->load);
#endif

            sRealm->process_proxy_announce(ann);
        }
    }
    else if (dataType->equal(Morpheus::_tc_Proxy_Shutdowned))
    {
        Morpheus::Proxy_Shutdowned* proxy;
        if (data >>= proxy)
        {
            std::string address(CORBA::string_dup(proxy->address));

#ifdef _MORPHEUS_DEBUG
            REALM_LOG("[EVENT] %s | Address: %s\n", dataType->name(), address.c_str());
#endif

            sRealm->delete_proxy(address);
        }
    }

#ifdef _MORPHEUS_DEBUG
    else
        REALM_LOG("[EVENT] %s | unhandled\n", dataType->name());
#endif
}

void EC_Communicator::disconnect_push_consumer()
{
    REALM_LOG("WARNING: Disconnected from Event Channel!\n");
    this->supplier_proxy->disconnect_push_supplier ();
}

};
};
