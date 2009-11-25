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
 *  @brief   
 *  @author  <author> <<email>>
 *  @date    <date>
 *
 */

#include "Proxy_Service.h"
#include "Proxy_Socket.h"

namespace Trinity
{
namespace Proxyd
{

void
Proxy_Socket::handle_cmsg_auth_session()
{
  uint32 tmp;
  if(!this->in_packet->CheckSize(4 + 4+ 1 + 4 + 20))
    return;

  *this->in_packet >> tmp;
  *this->in_packet >> tmp;
  *this->in_packet >> this->login;
  *this->in_packet >> this->client_seed;
  this->client_digest = new uint8[20];
  this->in_packet->read(this->client_digest, 20);

  PROXY_LOG("User %s tries to authenticate (seed: 0x%X)\n",
	    this->login.c_str(),
	    this->client_seed);
}

};
};
