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

namespace Morpheus
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

int Proxy_Socket::open(void*)
{
    PROXY_TRACE; 
    if(sProxy->load == 1)
        return -1;

    sProxy->update_connections(true);
    if (sProxy->get_reactor()->register_handler(this, ACE_Event_Handler::READ_MASK) == -1)
        return -1;

    ServerPkt* pkt = new ServerPkt(SMSG_AUTH_CHALLENGE, 4);
    *pkt << seed;
    this->send(pkt);

    return 0;
}

void Proxy_Socket::send(ByteBuffer* pkt)
{
    ACE_Guard<ACE_Recursive_Thread_Mutex> g(this->queue_mtx);
    packet_queue.push_back(pkt);
    if (!this->out_active) {
        sProxy->get_reactor()->register_handler(this,
                                    ACE_Event_Handler::WRITE_MASK);
        this->out_active = true;
    }
}

int Proxy_Socket::close(u_long)
{
    PROXY_TRACE; 
    this->die();
    return 0;
}

int Proxy_Socket::handle_input(ACE_HANDLE)
{
    PROXY_TRACE; 

    size_t bytes_read = 0;

    if (!expected_data) {  // We are not expecting new data.
        bytes_read = this->peer().recv(this->raw_buf, 6);

        // I'm sorry, but something's wrong. Please try again.
        if (bytes_read < 6)
            return -1;

        // header's read, lets decrypt it.
        this->crypto->decrypt(this->raw_buf, sizeof(ClientPktHeader));
      
        // Endian switch.
        Utils::EndianConvertReverse(*((int16*)&raw_buf[0]));
        Utils::EndianConvert(*((int32*)&raw_buf[2]));

        // Now let's receive rest of data.
        // Actually size field in packet excludes size of that field in itself...
        // So real packet size is reported size+2
        // *((uint16*)raw_buf) - 4 because we have received whole header.
        bytes_read = this->peer().recv(this->raw_buf + 6, *((uint16*)raw_buf) - 4);
    
        //+6 since we have header already.
        this->in_packet = new ClientPkt(bytes_read + 6);
        this->in_packet->append(raw_buf, bytes_read + 6);

        // If the size of received data + 4 (opcode size) isnt same as size declared in packet,
        // We need to get rest before we proceed.
        if (bytes_read + 4 != (this->in_packet->PeekSize()) ) {
            this->expected_data = this->in_packet->PeekSize() - bytes_read - 4;
            return 0;
        }
    }
    else {
        // Try to read remaining data
        bytes_read = this->peer().recv(this->raw_buf, expected_data);

        if (bytes_read == -1 || bytes_read == 0)
            return -1;
     
        // Add the data into waiting packet.
        this->in_packet->append(raw_buf, bytes_read);
      
        // If we still did not read whole packet at once,
        // Give it another chance.
        if (bytes_read != expected_data) {
            expected_data -= bytes_read;
            return 0;
        }
    }

    this->process_incoming();
    return 0;
}

void Proxy_Socket::process_incoming()
{
    if (!in_packet)
        return;

#ifdef _MORPHEUS_DEBUG
    PROXY_LOG("Received packet 0x%X, size: %u\n",this->in_packet->PeekOpcode(), this->in_packet->PeekSize());
    this->in_packet->hexlike();
#endif

    switch (this->in_packet->PeekOpcode()) {
    case CMSG_AUTH_SESSION:
        this->handle_cmsg_auth_session();
        break;
    case CMSG_PING:
        this->handle_cmsg_ping();
        break;
    case CMSG_REALM_SPLIT:
        this->handle_cmsg_realm_split();
        break;
    case CMSG_CHAR_ENUM:
        this->handle_cmsg_char_enum();
        break;
    case CMSG_CHAR_CREATE:
        this->handle_cmsg_char_create();
        break;
    default:
        break;
    }
    
    delete in_packet;
}

int Proxy_Socket::handle_output(ACE_HANDLE)
{
    PROXY_TRACE; 
    ACE_Guard<ACE_Recursive_Thread_Mutex> g(this->queue_mtx);
    int error;
    size_t sent_bytes = 0;

    while (!this->packet_queue.empty()) {
        ByteBuffer* buffer = this->packet_queue.front();
        ((ServerPkt*)buffer)->SetSize(buffer->size() - 4);
#ifdef _MORPHEUS_DEBUG
        PROXY_LOG("Sending packet:\n");
        buffer->hexlike();
#endif      
        if (!this->continue_send)
            this->crypto->encrypt(const_cast<uint8*>(buffer->contents()), 4);
        this->continue_send = false;
        sent_bytes = this->peer().send(buffer->contents(), buffer->size());
        error = errno;
        if (sent_bytes < buffer->size() && sent_bytes > 0) {
            buffer->rpos(sent_bytes - 1);
            buffer->read((uint8*)buffer->contents(),buffer->size() - sent_bytes);
            buffer->resize(buffer->size() - sent_bytes);
            this->continue_send = true;
            break;
        }
        else {
            if (sent_bytes == -1)
                return -1;
            else {
                delete buffer;
                packet_queue.pop_front();
            }
        }

    };
  
    if (this->packet_queue.empty()) {
        sProxy->get_reactor()->cancel_wakeup(this,
					   ACE_Event_Handler::WRITE_MASK);
        this->out_active = false;
        return 0;
    }
    else
        return 1;
}

int Proxy_Socket::handle_close(ACE_HANDLE, ACE_Reactor_Mask mask)
{
    PROXY_TRACE; 
    if(mask == ACE_Event_Handler::WRITE_MASK)
        return 0;
    this->die();
    return 0;
}

void Proxy_Socket::die()
{
    PROXY_TRACE; 
    sProxy->update_connections(false);
    this->ptr.release();
}

void Proxy_Socket::handle_cmsg_ping()
{
    PROXY_TRACE;
    uint32 seq;
    *this->in_packet >> seq;
    PROXY_LOG("PING: %u\n",seq);
    ServerPkt* pkt = new ServerPkt(SMSG_PONG, 4);
    *pkt << seq;
    this->send(pkt);
}

void Proxy_Socket::handle_cmsg_realm_split()
{
    PROXY_TRACE;
    uint32 unk;
    std::string date = "01/01/01"; //Magic taken from tc1.

    *this->in_packet >> unk;
  
    ServerPkt* pkt = new ServerPkt(SMSG_REALM_SPLIT, 4+4+date.size()+1);
    *pkt << unk;
    *pkt << (uint32)0x00;
    *pkt << date;
    this->send(pkt);
}

};
};
