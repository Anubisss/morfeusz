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
 *  @brief   Proxy_Socket class definition
 *  @author  raczman <raczman@gmail.com>
 *  @date    2009-11-13
 *
 */
#ifndef PROXY_SOCKET_H
#define PROXY_SOCKET_H

#include <list>
#include <ace/Svc_Handler.h>
#include <ace/SOCK_Stream.h>
#include <ace/Refcounted_Auto_Ptr.h>
#include <openssl/hmac.h>
#include <openssl/sha.h>
#include <openssl/bn.h>

#include "ByteBuffer.h"


namespace Trinity
{
namespace Proxyd
{

#if defined( __GNUC__ )
#pragma pack(1)
#else
#pragma pack(push,1)
#endif

struct ServerPktHeader
{
  uint16 size;
  uint16 cmd;
};

struct ClientPktHeader
{
  uint16 size;
  uint32 cmd;
};

#if defined( __GNUC__ )
#pragma pack()
#else
#pragma pack(pop)
#endif

class Proxy_Crypto;
class Proxy_Socket;
typedef ACE_Refcounted_Auto_Ptr<Trinity::Proxyd::Proxy_Socket, ACE_Recursive_Thread_Mutex> Proxy_Sock_Ptr;
  
  /**
   * @brief Network socket for Proxy Service.
   * @details As this class is based on ACE's handler class,
   *          there are elements that are common with 
   *          Trinity::Realmd::Realm_Socket class.
   * @sa Trinity::Realmd::Realm_Socket          
   */
class Proxy_Socket : public ACE_Svc_Handler<ACE_SOCK_STREAM, ACE_MT_SYNCH>
{
public:
  Proxy_Socket();
  ~Proxy_Socket();
  int open(void*);
  int close(u_long);
  int handle_input(ACE_HANDLE);
  int handle_output(ACE_HANDLE);
  int handle_close(ACE_HANDLE, ACE_Reactor_Mask);
  void send(ByteBuffer*);
private:
  void die();

  /**
   * @brief This function decides wether we forward packet to zone,
   *        or handle it in here.
   */
  void process_incoming();
  Proxy_Sock_Ptr ptr;
  size_t expected_data;
  uint8 raw_buf[4096];
  
  /**
   * @brief Seed used to authenticate with client.
   */
  uint32 seed;

  /**
   * @brief If we receive packet, we keep it in here
   *        so we can either receive rest (partial packet)
   *        or process it with process_incoming.
   */
  ClientPkt* in_packet;
  std::list<ByteBuffer*> packet_queue;
  ACE_Recursive_Thread_Mutex queue_mtx;
  bool out_active;
  /**
   * @brief Encryption interface.
   */
  Proxy_Crypto* crypto;
};

};
};

#endif //PROXY_SOCKET_H
