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
 *  @brief   Proxy_Socket class implementation.
 *  @author  raczman <raczman@gmail.com>
 *  @date    2009-11-13
 *
 */
#include "Proxy_Socket.h"
#include "Proxy_Service.h"
#include "Proxy_Crypto.h"
#include "Util.h"

using namespace Utils::ByteConverter;

namespace Trinity
{
namespace Proxyd
{

Proxy_Socket::Proxy_Socket()
  :ptr(this), crypto(new Proxy_Crypto),expected_data(0)
{
  ACE_OS::memset(this->raw_buf, 0, 4096);
}

Proxy_Socket::~Proxy_Socket()
{
  delete crypto;
}

int
Proxy_Socket::open(void*)
{
  
  return sProxy->get_reactor()->
    register_handler(this, ACE_Event_Handler::READ_MASK);
						
}

int
Proxy_Socket::close(u_long)
{
  this->die();
  return 0;
}

int
Proxy_Socket::handle_input(ACE_HANDLE)
{
  if(!expected_data)  // We are not expecting new data.
    {
      size_t bytes_read = this->peer().recv(this->raw_buf, 4096);
      ClientPktHeader* header= ((ClientPktHeader*)this->raw_buf);

      this->crypto->decrypt(this->raw_buf, sizeof(ClientPktHeader));

      Utils::EndianConvertReverse(header->size);
      Utils::EndianConvert(header->cmd);
      PROXY_LOG("CMD: %u SIZE: %u \n",header->cmd, header->size);
      if(bytes_read == ((ClientPktHeader*)this->raw_buf)->size + 6)
	{
	  PROXY_LOG("Received packet at once\n");
	}
    }
  else
    {

    }

  return 0;
}

int
Proxy_Socket::handle_output(ACE_HANDLE)
{
  return 0;

}

int
Proxy_Socket::handle_close(ACE_HANDLE, ACE_Reactor_Mask mask)
{
  if(mask == ACE_Event_Handler::WRITE_MASK)
    return 0;
  this->die();
  return 0;
}

void
Proxy_Socket::die()
{
  this->ptr.release();
}

};
};
