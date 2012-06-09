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
 *  @brief   Event channel communicator impl.
 *  @author  raczman <raczman@gmail.com>
 *  @date    2009-11-10
 *
 */

#include "Proxy_Service.h"
#include "Configuration.h"
#include "Proxyd_EC_Communicator.h"
#include "idl/Proxy_EventsC.h"
#include <orbsvcs/CosNamingC.h>

namespace Morpheus
{

namespace Proxyd
{

void EC_Communicator::connect()
{
    try {
        PROXY_LOG("Connecting to Event Channel...\n");
        CORBA::Object_var _object = 
            orb->resolve_initial_references("NameService");
        CosNaming::NamingContext_var naming_context = 
            CosNaming::NamingContext::_narrow (_object.in ());

        CosNaming::Name ec_name;
        ec_name.length(1);
        ec_name[0].id = CORBA::string_dup("CosEventService");

        CosEventChannelAdmin::EventChannel_var channel = 
            CosEventChannelAdmin::EventChannel::_unchecked_narrow(naming_context->resolve(ec_name));

        _object = orb->resolve_initial_references("RootPOA");
        this->poa = PortableServer::POA::_narrow(_object.in());
      
        PortableServer::ObjectId_var oid = poa->activate_object(this);
        CORBA::Object_var consumer_obj = poa->id_to_reference(oid.in());
      
        CosEventComm::PushConsumer_var consumer = 
            CosEventComm::PushConsumer::_unchecked_narrow(consumer_obj.in());
        this->supplier_proxy = channel->for_consumers()->obtain_push_supplier();
        this->supplier_proxy->connect_push_consumer(consumer);

        this->pusher = channel->for_suppliers()->obtain_push_consumer();
        this->pusher->connect_push_supplier(CosEventComm::PushSupplier::_nil());
        poa->the_POAManager()->activate();

        PROXY_LOG("Connected to Event Channel.\n");
    }
    catch (CORBA::Exception &e) {
        PROXY_LOG("Couldn't connect to Event Channel! - Exception thrown was of type: %s\n", e._name());
        throw;
        return;
    }
}

void EC_Communicator::disconnect_push_consumer()
{
    try {
        PROXY_LOG("WARNING: Disconnected from Event Channel!\n");
        this->supplier_proxy->disconnect_push_supplier ();
    }
    catch (CORBA::Exception &e) {
        return;
    }
}

void EC_Communicator::announce()
{
    try {
        Morpheus::Proxy_Announce ann;
        ann.realm_id = sProxy->get_realmid();
        ann.address = CORBA::string_dup(sConfig->getString("proxyd","BindAddr").c_str());
        ann.load = sProxy->load;

        CORBA::Any any;
        any <<= ann;
        this->pusher->push(any);
    }
    catch (CORBA::Exception &e) {
        PROXY_LOG("Exception thrown!\n");
        return;
    }
}

void EC_Communicator::push(const CORBA::Any &data)
{
    PROXY_TRACE;
    CORBA::TypeCode_ptr dataType = data._tao_get_typecode();
    if (dataType->equal(_tc_Proxy_Request))
    {
        Morpheus::Proxy_Request* req;
        if (data >>= req)
        {
            PROXY_LOG("[EVENT] %s | ID: %u\n", dataType->name(), req->realm_id);
            if (req->realm_id == sProxy->get_realmid())
                this->announce();
        }
    }
    else
        PROXY_LOG("[EVENT] %s | unhandled\n", dataType->name());
}

void EC_Communicator::report_load()
{
    try {
        Morpheus::Proxy_Load_Report rep;
        rep.address = CORBA::string_dup(sConfig->getString("proxyd","BindAddr").c_str());
        rep.load = sProxy->load;

        CORBA::Any any;
        any <<= rep;
        this->pusher->push(any);
    }
    catch (CORBA::Exception &e) {
      return;
    }
}

};
};
