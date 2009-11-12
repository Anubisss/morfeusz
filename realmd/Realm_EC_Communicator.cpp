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
      PortableServer::POA_var poa = 
	PortableServer::POA::_narrow(_object.in());
      
      PortableServer::ObjectId_var oid = poa->activate_object(this);
      CORBA::Object_var consumer_obj = poa->id_to_reference(oid.in());
      
      CosEventComm::PushConsumer_var consumer = 
	CosEventComm::PushConsumer::_unchecked_narrow(consumer_obj.in());
      channel->for_consumers()->obtain_push_supplier()->connect_push_consumer(consumer);
    
    }
  catch(CORBA::Exception &e)
    {
      REALM_LOG("Couldn't connect to Event Channel!\nException thrown was of type: %s\n",e._name());
      return;
    }
  REALM_LOG("Connected to Event Channel.\n");
}

};
};
