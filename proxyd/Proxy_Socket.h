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
 *  @brief   Proxy_Socket class definition
 *  @author  raczman <raczman@gmail.com>
 *  @date    2009-11-13
 *  @ingroup Proxy
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

namespace Morpheus
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

struct Account
{
    uint64 id;
    uint8 gmlevel;
    std::string sessionkey;
    std::string sha_pass_hash;
    std::string v;
    std::string s;
    uint8 expansion;
};

struct Character_Pet
{
    uint32 entry;
    uint32 displayid;
    uint32 level;
    uint32 family;
    Character_Pet()
    {
        entry = 0;
        displayid = 0;
        level = 0;
        family = 0;
    }
};

struct Character
{
    uint64 guid;
    uint8 race;
    uint8 pclass;  //"player class" - class is a keyword hence the "p"
    uint8 gender;
    uint32 bytes;
    uint8 bytes2;
    uint8 level;
    std::string name;
    float x;
    float y;
    float z;
    uint32 map;
    uint32 zone;
    uint32 login_flags;
    uint32 player_flags;
    Character_Pet pet;
    std::vector<uint32> update_fields;
    uint32 guild;
};

#if defined( __GNUC__ )
#pragma pack()
#else
#pragma pack(pop)
#endif

class Proxy_Crypto;
class Proxy_Socket;
typedef ACE_Refcounted_Auto_Ptr<Morpheus::Proxyd::Proxy_Socket, ACE_Recursive_Thread_Mutex> Proxy_Sock_Ptr;
  
/**
 * @brief Network socket for Proxy Service.
 * @details As this class is based on ACE's handler class,
 *          there are elements that are common with 
 *          Morpheus::Realmd::Realm_Socket class.
 * @sa Morpheus::Realmd::Realm_Socket          
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
    std::string& get_login(){return this->login;}
    void set_account(Account act) { this->acct = act; }
    void set_characters(std::list<Character> list) { this->characters = list; }
    uint64 get_acct_id() { return this->acct.id; }
    
    /**
     * @brief Callback from ProxyDB::get_account
     * @sa handle_cmsg_auth_session
     */
    void account_retrieved(bool state);

    /**
     * @brief Callback from ProxyDB::get_chars
     * @sa handle_cmsg_char_enum
     */
    void characters_retrieved(bool chars_present);

private:

    void die();

    /**
     * @brief This function decides wether we forward packet to zone,
     *        or handle it in here.
     */
    void process_incoming();

    /**
     * @brief Handles CMSG_AUTH_SESSION
     *        This function verifies that client has
     *        indeed authenticated via realmd.
     */
    void handle_cmsg_auth_session();

    /**
     * @brief Handles CMSG_PING
     *        which makes client know its latency.
     */
    void handle_cmsg_ping();

    /**
     * @brief Handles CMSG_REALM_SPLIT
     *        realm splits were a way of offy
     *        to divide overpopulated realms.
     *        The handler just sends back data
     *        that indicates no split. 
     */
    void handle_cmsg_realm_split();

    /**
     * @brief Handles CMSG_CHAR_ENUM
     *
     */
    void handle_cmsg_char_enum();
    
    /**
     * @brief Handles CMSG_CHAR_CREATE
     * 
     */
    void handle_cmsg_char_create();

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
  
    Account acct;
    std::string login;

    uint8* client_digest;
    uint32 client_seed;
    std::list<Character> characters;
    std::list<ByteBuffer*> packet_queue;
    ACE_Recursive_Thread_Mutex queue_mtx;
    bool out_active;
    bool continue_send;
    /**
     * @brief Encryption interface.
     */
    Proxy_Crypto* crypto;
};

};
};

#endif //PROXY_SOCKET_H
