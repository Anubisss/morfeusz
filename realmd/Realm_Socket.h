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
 *  @brief   Header file for Realmd socket.
 *  @author  raczman <raczman@gmail.com>
 *  @date    2009-10-22
 *  @ingroup Realmd
 */


#pragma once
#ifndef REALM_SOCKET_H
#define REALM_SOCKET_H

#include <ace/Svc_Handler.h>
#include <ace/SOCK_Stream.h>
#include <ace/Refcounted_Auto_Ptr.h>
#include <string>
#include <list>
#include <map>
#include <openssl/bn.h>


#include "Common.h"

class ByteBuffer;

namespace Trinity
{

namespace Realmd
{
/**
 * @brief Holds account information retrieved from database.
 * @see RealmDB::get_account(Realm_Sock_Ptr)
 * @see Realm_Socket::account_checked(AccountState)
 */
class Account
{
 public:
  std::string sha_pass;
  uint64 id;
  bool locked;
  std::string last_ip;
  uint8 gmlevel;
  uint8 failed_logins;
};
 
/**
 *  @brief This enum is used by callback that fetches accounts from db.
 *  @see void Realm_Socket::account_checked(AccountState)
 */
enum AccountState
{
  ACCOUNT_EXISTS,
  ACCOUNT_NOTFOUND,
  ACCOUNT_BANNED
};

/**
 * @brief Used to mark state of connection, 
 *        to permit certain operations only to authed clients.
 */
enum ConnectionState
{
  STATUS_CONNECTED,
  STATUS_NEED_PROOF,
  STATUS_AUTHED,
  STATUS_NEED_RPROOF,
  STATUS_CLOSING
};

/**
 * @brief Networking socket used to communicate with game client
 * @details Inherited from ACE_Svc_Handler this class follows 
 *          Acceptor/Reactor design pattern. 
 * @see ACE_Svc_Handler
 */
class Realm_Socket : public ACE_Svc_Handler<ACE_SOCK_STREAM, ACE_MT_SYNCH>
{

 public:
  /**
   * @brief Initialises recounted ptr, and bignumbers used for SRP6.
   */
  Realm_Socket();

  /**
   * @brief Frees resources.
   */
  ~Realm_Socket();
  
  /**
   * @brief Called by reactor when new socket is opened.
   */
  int open(void*);

  /**
   * @brief Called by reactor when connection is closed, 
   *        or handler is deregistered for event type.
   */
  int close(u_long flags);
  const std::string& get_ip(){return ip;}
  const std::string& get_login(){return login;}

  /**
   * @brief %Callback called from checkIpBanObsv 
   * @param result Indicates wether IP is banned.
   */
  void ip_ban_checked(bool result);

  /**
   * @brief %Callback from checkAcctObsv
   *        by the time this callback is received, 
   *        Realm_Socket::acct should be set in case 
   *        account exists and is usable.
   * @param state Contains value from AccountState 
   *              enum, to indicate account status
   */
  void account_checked(AccountState state);
  /**
   * @brief Inserts new packet into output queue.
   *        This function is thread safe, and it's
   *        practically the only way to send to client.
   * @param pkt Payload to send.
   */
  void send(ByteBuffer* pkt);
  /**
   * @brief Stores account information for connection
   */
  Account acct;

  /**
   * @brief %Callback from getCharAmntObsv 
   * @param amnt Contains information about character amount
   */
  void get_char_amount(std::map<uint8, uint8> amnt);

  /**
   * @brief %Callback from getSessionKeyObsv
   * @param result True means we have sessionkey, false means we do not.
   */
  void get_sessionkey(bool result);

  /**
   * @brief Sessionkey. It is public because it is set by callback.
   */
  BIGNUM* k;
 private:

  /**
   * @brief Map keyed with realm id, value is number of characters.
   */
  std::map<uint8, uint8> realm_char_amount;

  /**
   * @brief This variable is set when we have any packets in
   *        socket's packet_queue
   */
  bool out_active;

  /**
   * @brief Called when we want to close the socket.
   *        This function releases internal refcounted pointer
   *        which will cause object to expire as soon as
   *        other references to it are removed.
   */
  void die();
  
  /**
   * @brief Handles packet 0x00, fills initial information about account.
   */
  void handle_auth_logon_challenge();

  /**
   * @brief Handles 0x01, handles authentication using SRP6.
   */
  void handle_auth_logon_proof();

  /**
   * @brief Handles 0x10, sends realmlist that matches client's build
   */
  void handle_realm_list();

  /**
   * @brief Handles packet 0x02
   */
  void handle_auth_reconnect_challenge();

  /**
   * @brief Handles packet 0x03
   */
  void handle_auth_reconnect_proof();
  
  /**
   * @brief Sets verificator and seed, used by SRP6 authentication.
   */
  void set_vs();

  /**
   * @brief Called by Reactor when socket is available for read.
   */
  int handle_input(ACE_HANDLE);

  /**
   * @brief Called by Reactor when socket is available for write.
   */
  int handle_output(ACE_HANDLE);

  /**
   * @brief Handles closing, or deregisters for write events
   *        when we do not want to write to socket for some time.
   */
  int handle_close(ACE_HANDLE, ACE_Reactor_Mask);

  /**
   * @brief Internal reference counted pointer, which helps us 
   *        with cleaning memory.
   */
  ACE_Refcounted_Auto_Ptr<Realm_Socket, ACE_Recursive_Thread_Mutex> ptr;

  /**
   * @brief Buffer that receives incoming data.
   */
  char raw_buf[4096];
  ConnectionState state;
  uint16 client_build;
  std::string login;
  std::string ip;
  /**
   * @brief FIFO queue for output packets.
   */
  std::list<ByteBuffer*> packet_queue;

  /**
   * @brief Lock for synchronising access into packet_queue
   */
  ACE_Recursive_Thread_Mutex queue_mtx;
  BN_CTX* ctx;

  /**
   * @brief Variables used when calculating SRP6
   *        Seed, verificator, sessionkey, N prime and others. 
   */
  BIGNUM* s, *v, *g, *N, *b, *B, *reconnect_proof;

  /**
  * @brief Called when password doesn't match
  */
  void handle_failed_login();

  /**
  * @brief Builds REALM_LIST data packet for 1.12 build
  */
  ByteBuffer* build_realm_packet();

  /**
  * @brief Builds REALM_LIST data packet for 2.4.3 and 3.2.0 builds (till they have same structure)
  */
  ByteBuffer* build_expansion_realm_packet();

  /**
  * @brief Builds AUTH_LOGON_PROOF data packet for 1.12 build
  */
  ByteBuffer* build_logon_proof_packet(uint8* hamk_fin);

  /**
  * @brief Builds AUTH_LOGON_PROOF data packet for 2.4.3 and 3.2.0 builds (till they have same structure)
  */
  ByteBuffer* build_expansion_logon_proof_packet(uint8* hamk_fin);
};

/**
 *  @brief Use this type when refering to refcounted pointer to Realm_Socket
 */
typedef ACE_Refcounted_Auto_Ptr<Trinity::Realmd::Realm_Socket, ACE_Recursive_Thread_Mutex> Realm_Sock_Ptr;
};
};
#else
#endif
