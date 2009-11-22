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
#include "Opcodes.h"

using namespace Utils;
using namespace Utils::ByteConverter;

namespace Trinity
{
namespace Proxyd
{

Proxy_Socket::Proxy_Socket()
  :ptr(this), crypto(new Proxy_Crypto),expected_data(0),
   in_packet(NULL), seed(urand32()), out_active(false),
   continue_send(false)
{
  PROXY_TRACE; 

  ACE_OS::memset(this->raw_buf, 0, 4096);
}

Proxy_Socket::~Proxy_Socket()
{
  PROXY_TRACE; 
  delete crypto;
}

int
Proxy_Socket::open(void*)
{
  PROXY_TRACE; 
  if(sProxy->load == 1)
    return -1;

  sProxy->update_connections(true);
  if(sProxy->get_reactor()->
     register_handler(this, ACE_Event_Handler::READ_MASK) == -1)
    return -1;
  
  ByteBuffer* pkt = new ByteBuffer(8);
  *pkt << (uint8)0x00;
  *pkt << (uint8)0x06;
  *pkt << (uint16)SMSG_AUTH_CHALLENGE;
  *pkt << seed;
  this->send(pkt);

  return 0;
}

void
Proxy_Socket::send(ByteBuffer* pkt)
{
  ACE_Guard<ACE_Recursive_Thread_Mutex> g(this->queue_mtx);
  packet_queue.push_back(pkt);
  if(!this->out_active)
    {
      sProxy->get_reactor()->register_handler(this,
					    ACE_Event_Handler::WRITE_MASK);
      this->out_active = true;
    }
}

int
Proxy_Socket::close(u_long)
{
  PROXY_TRACE; 
  this->die();
  return 0;
}

int
Proxy_Socket::handle_input(ACE_HANDLE)
{
  PROXY_TRACE; 

  size_t bytes_read = this->peer().recv(this->raw_buf, 4096);
  
  if(bytes_read == -1 || bytes_read == 0)
    return -1;

  if(!expected_data)  // We are not expecting new data.
    {
      Utils::EndianConvertReverse(*((int16*)&raw_buf[0]));
      Utils::EndianConvert(*((int32*)&raw_buf[2]));
  
      this->crypto->decrypt(this->raw_buf, sizeof(ClientPktHeader));
      this->in_packet = new ClientPkt(bytes_read);
      this->in_packet->append(raw_buf, bytes_read);
      
      if(bytes_read != (this->in_packet->PeekSize() + 2 ) )
	{
	  this->expected_data = this->in_packet->PeekSize() - bytes_read - 6;
	  return 0;
	}
    }
  else
    {
     
      this->in_packet->append(raw_buf, bytes_read);
      if(this->in_packet->size() != this->in_packet->PeekSize() + 2)
	return 0;
    }
  this->process_incoming();
  return 0;
}

void
Proxy_Socket::process_incoming()
{

  if(!in_packet)
    return;
  PROXY_LOG("Received packet %u, size: %u\n",this->in_packet->PeekOpcode(), this->in_packet->PeekSize());
  delete in_packet;
}

int
Proxy_Socket::handle_output(ACE_HANDLE)
{
  PROXY_TRACE; 
  ACE_Guard<ACE_Recursive_Thread_Mutex> g(this->queue_mtx);
  int error;
  size_t sent_bytes = 0;

  while(!this->packet_queue.empty())
    {
      ByteBuffer* buffer = this->packet_queue.front();
      
      if(!this->continue_send)
	this->crypto->encrypt(const_cast<uint8*>(buffer->contents()), 4);
      this->continue_send = false;
      sent_bytes = this->peer().send(buffer->contents(), buffer->size());
      error = errno;
      if(sent_bytes < buffer->size() && sent_bytes > 0)
	{
	  buffer->rpos(sent_bytes - 1);
	  buffer->read((uint8*)buffer->contents(),buffer->size() - sent_bytes);
	  buffer->resize(buffer->size() - sent_bytes);
	  this->continue_send = true;
	  break;
	}
      else
	if(sent_bytes == -1)
	  {
	    return -1;
	  }
	else
	  {
	    delete buffer;
	    packet_queue.pop_front();
	  }

    };
  
  if(this->packet_queue.empty())
    {
      sProxy->get_reactor()->cancel_wakeup(this,
					   ACE_Event_Handler::WRITE_MASK);
      this->out_active = false;
      return 0;
    }
  else
    return 1;

}

int
Proxy_Socket::handle_close(ACE_HANDLE, ACE_Reactor_Mask mask)
{
  PROXY_TRACE; 
  if(mask == ACE_Event_Handler::WRITE_MASK)
    return 0;
  this->die();
  return 0;
}

void
Proxy_Socket::die()
{
  PROXY_TRACE; 
  sProxy->update_connections(false);
  this->ptr.release();
}

};
};
