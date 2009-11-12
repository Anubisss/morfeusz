/* -*- C++ -*-
 * Copyright (C) 2009 Trinity Core <http://www.trinitycore.org>
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
 *  @brief   <brief>
 *  @author  <author> <<email>>
 *  @date    <date>
 *
 */
#include "Realm_Service.h"
#include "Realm_EC_Communicator.h"
#include "Proxy_EventsC.h"
#include <orbsvcs/CosNamingC.h>

namespace Trinity
{
namespace Realmd
{
void
EC_Communicator::connect()
{
  REALM_LOG("Connecting to Event Channel...\n");

  try
    {
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
      
    }
  catch(CORBA::Exception &e)
    {
      REALM_LOG("Couldn't connect to Event Channel!\nException thrown was of type: %s\n",e._name());
      return;
    }
  REALM_LOG("Connected to Event Channel.\n");
}

void
EC_Communicator::request_proxies_for_realm(uint8 id)
{
  Trinity::Proxy_Request req;
  req.realm_id = id;

  CORBA::Any any;
  any <<= req;
  this->pusher->push(any);
}

void
EC_Communicator::push(const CORBA::Any &data)
{
  
  Trinity::Proxy_Announce* ann;
  Trinity::Proxy_Load_Report* report;

  if(data >>= ann)
    {
      sRealm->add_proxy(ann->realm_id, std::string(ann->address.out()),
			ann->load);
    }
  else if(data >>= report)
    {
      sRealm->add_proxy_load_report(std::string(report->address.out()),
				    report->load);
    }
  else 
    {
      REALM_LOG("Received unknown event type!\n");
    }
}

void
EC_Communicator::disconnect_push_consumer()
{
  this->supplier_proxy->disconnect_push_supplier ();
}

};
};
