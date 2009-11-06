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
 *  @brief   Implementation for Realm_Socket class
 *  @author  raczman <raczman@gmail.com>
 *  @date    2009-10-22
 *  @ingroup Realmd
 */


#include "Realm_Socket.h"
#include "Realm_Service.h"
#include "AuthCodes.h"
#include "Util.h"
#include "ByteBuffer.h"
#include "Configuration.h"

#include <openssl/sha.h>
#include <algorithm>

namespace Trinity
{
enum eAuthCmd
  {
    //AUTH_NO_CMD                 = 0xFF,
    AUTH_LOGON_CHALLENGE        = 0x00,
    AUTH_LOGON_PROOF            = 0x01,
    AUTH_RECONNECT_CHALLENGE    = 0x02,
    AUTH_RECONNECT_PROOF        = 0x03,
    //update srv =4
    REALM_LIST                  = 0x10,
    XFER_INITIATE               = 0x30,
    XFER_DATA                   = 0x31,
    XFER_ACCEPT                 = 0x32,
    XFER_RESUME                 = 0x33,
    XFER_CANCEL                 = 0x34
  };

#if defined( __GNUC__ )
#pragma pack(1)
#else
#pragma pack(push,1)
#endif

typedef struct AUTH_LOGON_CHALLENGE_C
{
  uint8   cmd;
  uint8   error;
  uint16  size;
  uint8   gamename[4];
  uint8   version1;
  uint8   version2;
  uint8   version3;
  uint16  build;
  uint8   platform[4];
  uint8   os[4];
  uint8   country[4];
  uint32  timezone_bias;
  uint32  ip;
  uint8   I_len;
  uint8   I[1];
} sAuthLogonChallenge_C;

typedef struct AUTH_LOGON_PROOF_C
{
  uint8   cmd;
  uint8   A[32];
  uint8   M1[20];
  uint8   crc_hash[20];
  uint8   number_of_keys;
  uint8   unk;
} sAuthLogonProof_C;

typedef struct AUTH_RECONNECT_PROOF_C
{
  uint8 cmd;
  uint8 R1[16];
  uint8 R2[20];
  uint8 R3[20];
  uint8 number_of_keys;
} sAuthReconnectProof_C;

#if defined( __GNUC__ )
#pragma pack()
#else
#pragma pack(pop)
#endif

enum SupportedClientBuilds
{
    BUILD_1_12 = 5875,
    BUILD_2_43 = 8606,
    BUILD_3_20 = 10505
};

bool
isSupportedClientBuild(int build)
{
    switch (build)
    {
        case BUILD_1_12:
        case BUILD_2_43:
        case BUILD_3_20:
            return true;
        default:
            return false;
    }
    return false;
}

Realm_Socket::Realm_Socket()
  :ptr(this), out_active(false), state(STATUS_CONNECTED)
{
  REALM_TRACE;

  ctx = BN_CTX_new();
  N = NULL;
  BN_hex2bn(&N, "894B645E89E1535BBDAD5B8B290650530801B18EBFBF5E8FAB3C82872A3E9BB7");
  g = BN_new();
  k = BN_new();
  s = BN_new();
  v = BN_new();
  b = BN_new();
  B = BN_new();
  reconnect_proof = BN_new();

  BN_set_word(k, 3);
  BN_set_word(g, 7);
  BN_rand(s, 32*8, 0, 1);

}

Realm_Socket::~Realm_Socket()
{
  REALM_TRACE;
  BN_CTX_free(ctx);
  BN_free(g);
  BN_free(k);
  BN_free(s);
  BN_free(v);
  BN_free(b);
  BN_free(B);
  BN_free(N);
  BN_free(reconnect_proof);
}

int
Realm_Socket::open(void*)
{
  REALM_TRACE;
  ACE_INET_Addr addr;
  
  if(this->peer().get_remote_addr(addr) == -1)
    return -1;
  if(sRealm->get_reactor()->register_handler(this,
					     ACE_Event_Handler::READ_MASK) == -1 )
    return -1;
  this->ip.append(addr.get_host_addr());
  REALM_LOG("Got connection from: %s (%s) \n", addr.get_host_name(), addr.get_host_addr());
  
  return 0;
}

int
Realm_Socket::close(u_long flags)
{
  REALM_TRACE;
  this->die();
  return 0;
}

int
Realm_Socket::handle_input(ACE_HANDLE)
{
  REALM_TRACE;

  if(this->state == STATUS_CLOSING)
    return -1;
  int error;
  memset(raw_buf, 0, 4096);
  int bytes_read = this->peer().recv(this->raw_buf, 4096);
  error = errno;

  if(bytes_read == -1 || bytes_read == 0)
    /*    if(error == EWOULDBLOCK && bytes_read < 0)
      return 0;
    else
    */  return -1;

  uint8 cmd = (uint8)raw_buf[0];

  //REALM_LOG("Got command %u from peer, bytes_read = %u\n",cmd, bytes_read);
  switch(cmd)
    {
    case AUTH_LOGON_CHALLENGE:
      this->handle_auth_logon_challenge();
      break;
    case AUTH_LOGON_PROOF:
      this->handle_auth_logon_proof();
      break;
    case AUTH_RECONNECT_CHALLENGE:
      this->handle_auth_reconnect_challenge();
      break;
    case AUTH_RECONNECT_PROOF:
      this->handle_auth_reconnect_proof();
      break;
    case REALM_LIST:
      this->handle_realm_list();
      break;
    default:
      REALM_LOG("Got unknown command %u, disconnecting!\n",cmd);
      return -1;
    }


    return 0;
}


void
Realm_Socket::handle_auth_logon_challenge()
{
  if(this->state != STATUS_CONNECTED)
    {
      this->die();
      return;
    }

  sAuthLogonChallenge_C* ch = (sAuthLogonChallenge_C*)&raw_buf;
  
  Utils::EndianConvert(*((uint32*)(&ch->gamename[0])));
  Utils::EndianConvert(*((uint32*)(&ch->platform[0])));
  Utils::EndianConvert(*((uint32*)(&ch->os[0])));
  Utils::EndianConvert(*((uint32*)(&ch->country[0])));
  Utils::EndianConvert(ch->timezone_bias);
  Utils::EndianConvert(ch->ip);

  this->login.append((char*)ch->I, ch->I_len);
  REALM_LOG("User %s tries to log in\n", this->login.c_str());
  
  this->client_build = ch->build;

  if(!isSupportedClientBuild(this->client_build))
    {
      ByteBuffer *pkt = new ByteBuffer;
      *pkt << (uint8) AUTH_LOGON_CHALLENGE;
      *pkt << (uint8) 0x00;
      *pkt << (uint8) REALM_AUTH_WRONG_BUILD_NUMBER;
      this->send(pkt);
      return;
    }
  //  ACE_OS::memset(raw_buf, 0, 4096);
  sRealm->get_db()->check_ip_ban(this->ptr);
  this->state = STATUS_NEED_PROOF;
}

void
Realm_Socket::handle_auth_logon_proof()
{
  if(this->state != STATUS_NEED_PROOF)
    {
      this->die();
      return;
    }
  REALM_TRACE;
  sAuthLogonProof_C* prf = (sAuthLogonProof_C*)&raw_buf;
  std::reverse(prf->A, (uint8*)prf->A + sizeof(prf->A));
  BIGNUM* A = BN_new();
  BN_bin2bn(prf->A, sizeof(prf->A), A);
  std::reverse(prf->A, (uint8*)prf->A + sizeof(prf->A));
  
  //Again, thanks Derex <3
  BIGNUM *u = BN_new();
  {
    uint8 B_buff[32];
    uint8 u_buff[SHA_DIGEST_LENGTH];
    
    ACE_OS::memset((char*)B_buff, 0, sizeof(B_buff));
    BN_bn2bin(B, B_buff);
    // Convert to little endian.
    std::reverse((uint8*)B_buff, (uint8*)B_buff + sizeof(B_buff));
    
    SHA_CTX sha;
    SHA1_Init(&sha);
    SHA1_Update(&sha, prf->A, sizeof(prf->A));
    SHA1_Update(&sha, B_buff, sizeof(B_buff));
    SHA1_Final(u_buff, &sha);
    
    // Convert to big endian.
    std::reverse((uint8*)u_buff, (uint8*)u_buff + sizeof(u_buff));
    BN_bin2bn(u_buff, sizeof(u_buff), u);
  }
  
  BIGNUM *S = BN_new();
  {
    BIGNUM *temp = BN_new();
    BN_mod_exp(temp, v, u, N, ctx);
    BN_mul(temp, A, temp, ctx);
    BN_mod_exp(S, temp, b, N, ctx);
    BN_free(temp);
  }
  uint8 K_buff[40];
  uint8 S_buff[32];
  uint8 S_buff_half[16];
  uint8 sha_buff[SHA_DIGEST_LENGTH];
  
  BN_bn2bin(S, S_buff);
  std::reverse((uint8*)S_buff, (uint8*)S_buff + sizeof(S_buff));

  for(int i = 0; i < 16; i++)
    S_buff_half[i] = S_buff[i*2];

  SHA1(S_buff_half, sizeof(S_buff_half), sha_buff);

  for(int i = 0; i < 20; i++)
    K_buff[i*2] = sha_buff[i];

  for(int i = 0; i < 16; i++)
    S_buff_half[i] = S_buff[i*2+1];

  SHA1(S_buff_half, sizeof(S_buff_half), sha_buff);

  for(int i = 0; i < 20; i++)
    K_buff[i*2+1] = sha_buff[i];

  uint8 N_buff[32];
  uint8 N_hash[SHA_DIGEST_LENGTH];
  ACE_OS::memset((char*)N_buff, 0, sizeof(N_buff));
  BN_bn2bin(N, N_buff);
  std::reverse((uint8*)N_buff, (uint8*)N_buff+sizeof(N_buff));
  SHA1(N_buff, sizeof(N_buff), N_hash);

  // Load g, and calc H(g)
  uint8 g_buff = 7;
  uint8 g_hash[SHA_DIGEST_LENGTH];
  SHA1(&g_buff, sizeof(g_buff), g_hash);

  // HNxorHg = H(N) xor H(g) :)
  uint8 HNxorHg[SHA_DIGEST_LENGTH];
  for(size_t i = 0; i < sizeof(HNxorHg); i++)
    HNxorHg[i] = N_hash[i] ^ g_hash[i];

  // Caclulate H(I)
  uint8 I_hash[SHA_DIGEST_LENGTH];
  SHA1((const uint8*)(this->login.c_str()), this->login.length(), I_hash);

  // Load s.
  uint8 s_buff[32];
  ACE_OS::memset((char*)s_buff, 0, sizeof(s_buff));
  BN_bn2bin(s, s_buff);
  std::reverse((uint8*)s_buff, (uint8*)s_buff+sizeof(s_buff));

  // We already have A

  // Load B.
  uint8 B_buff[32];
  ACE_OS::memset((char*)B_buff, 0, sizeof(B_buff));
  BN_bn2bin(B, B_buff);
  std::reverse((uint8*)B_buff, (uint8*)B_buff+sizeof(B_buff));
  BN_free(A);
  BN_free(u);
  BN_free(S);
  // We also have K, lets make the SHA1.
  uint8 M_buff[SHA_DIGEST_LENGTH];
  SHA_CTX M_sha_ctx;
  SHA1_Init(&M_sha_ctx);
  SHA1_Update(&M_sha_ctx, HNxorHg, sizeof(HNxorHg));
  SHA1_Update(&M_sha_ctx, I_hash, sizeof(I_hash));
  SHA1_Update(&M_sha_ctx, s_buff, sizeof(s_buff));
  SHA1_Update(&M_sha_ctx, prf->A, sizeof(prf->A));
  SHA1_Update(&M_sha_ctx, B_buff, sizeof(B_buff));
  SHA1_Update(&M_sha_ctx, K_buff, sizeof(K_buff));
  SHA1_Final(M_buff, &M_sha_ctx);

  //End of Derex's code.
  
  if(!ACE_OS::memcmp(prf->M1, M_buff, SHA_DIGEST_LENGTH))
    {
      REALM_LOG("User %s authenticated\n", this->login.c_str());
      
      // H(A|M|K)
      uint8 hamk_fin[SHA_DIGEST_LENGTH] = {0};
      SHA_CTX hamk;
      SHA1_Init(&hamk);
      SHA1_Update(&hamk, prf->A, sizeof(prf->A));
      SHA1_Update(&hamk, M_buff, sizeof(M_buff));
      SHA1_Update(&hamk, K_buff, sizeof(K_buff));
      SHA1_Final(&hamk_fin[0], &hamk);

      SqlOperationRequest* op = new SqlOperationRequest(REALMD_DB_UPDATE_ACCOUNT);
      BIGNUM* K = BN_new();
      std::reverse((uint8*)K_buff, (uint8*)K_buff + sizeof(K_buff));
      BN_bin2bn(K_buff,sizeof(K_buff),K);
      const char* K_hexb = BN_bn2hex(K);
      BN_free(K);
      op->add_string(1, K_hexb);
      free((void*)K_hexb);
      op->add_string(2, this->ip.c_str());
      op->add_uint8(3, 0);
      op->add_string(4, this->login.c_str());
      sRealm->get_db()->enqueue(op);
      
      ByteBuffer* pkt = new ByteBuffer(this->client_build == BUILD_1_12 ? 26 : 32);
      *pkt << (uint8) AUTH_LOGON_PROOF;
      *pkt << (uint8) 0;
      pkt->append(hamk_fin, SHA_DIGEST_LENGTH);
      if(this->client_build != BUILD_1_12)
        *pkt << (uint32) 0x00800000;
      *pkt << (uint32) 0;
      if(this->client_build != BUILD_1_12)
        *pkt << (uint16) 0;
      this->send(pkt);
      this->state = STATUS_AUTHED;
    }
  else
    {
      REALM_LOG("Wrong password for user %s (%s)\n", this->login.c_str(), this->ip.c_str());
      if(sConfig->getBool("realmd","WrongPassBan"))
	{
	  this->acct.failed_logins++;
	  sRealm->get_db()->increment_failed_logins(this->acct.id);
	  if(this->acct.failed_logins > ( sConfig->getInt("realmd","WrongPassAmnt") ) )
	    {
	      if(sConfig->getString("realmd", "WrongPassBanType").compare("ip"))
		sRealm->get_db()->ban_failed_logins(this->acct.id);
	      else
		sRealm->get_db()->ban_failed_logins(this->ip);
	    }
	}
      ByteBuffer buf(4);
      buf << (uint8) AUTH_LOGON_CHALLENGE;
      buf << (uint8) 0;
      buf << (uint8) REALM_AUTH_NO_MATCH;
      this->send(new ByteBuffer(buf));
      this->die();
    }
  
}

void
Realm_Socket::handle_realm_list()
{
  if(this->state != STATUS_AUTHED)
    {
      this->die();
    }
  sRealm->get_db()->get_char_amount(this->ptr);
}

void
Realm_Socket::handle_auth_reconnect_challenge()
{
  if(this->state != STATUS_CONNECTED)
    {
      this->die();
      return;
    }

  sAuthLogonChallenge_C* ch = (sAuthLogonChallenge_C*)&raw_buf;

  this->client_build = ch->build;
  this->login = (const char*)ch->I;
  sRealm->get_db()->get_sessionkey(this->ptr);
  this->state = STATUS_NEED_RPROOF;
}

void
Realm_Socket::handle_auth_reconnect_proof()
{
  if(this->state != STATUS_NEED_RPROOF)
    {
      this->die();
      return;
    }
  sAuthReconnectProof_C* prf = (sAuthReconnectProof_C*)&raw_buf;
  
  SHA_CTX sha;
  uint8 hash[SHA_DIGEST_LENGTH];
  SHA1_Init(&sha);
  SHA1_Update(&sha, this->login.c_str(), this->login.length() );
  SHA1_Update(&sha, prf->R1, 16);
  uint8* tmp = new uint8[16];
  BN_bn2bin(this->reconnect_proof, tmp);
  SHA1_Update(&sha, tmp, 16);
  delete[] tmp;
  tmp = new uint8[40];
  BN_bn2bin(this->k, tmp);
  SHA1_Update(&sha, tmp, 40);
  SHA1_Final(hash, &sha);
  delete[] tmp;

  if(!memcmp(hash, prf->R2, SHA_DIGEST_LENGTH))
    {
      ByteBuffer* pkt = new ByteBuffer();
      *pkt << (uint8)AUTH_RECONNECT_PROOF;
      *pkt << (uint8)0x00;
      *pkt << (uint16)0x00;
      this->send(pkt);
      this->state = STATUS_AUTHED;
    }
  else
    {
      this->die();
    }
}

void
Realm_Socket::get_char_amount(std::map<uint8, uint8> amnt)
{
  REALM_TRACE;
  realm_char_amount = amnt;
  ByteBuffer* pkt = new ByteBuffer;
  std::map<uint8, Realm>* realmlist = sRealm->get_realmlist();

  uint16 listSize=0;
  std::map<uint8, Realm>::const_iterator i;
  for(i = realmlist->begin(); i != realmlist->end(); i++)
      if(i->second.build == this->client_build)
          ++listSize;
      
  
  *pkt << (uint32) 0;
  if(this->client_build == BUILD_1_12)
    *pkt << (uint8) listSize;
  else
    *pkt << (uint16) listSize;
  if (listSize > 0)
  for(i = realmlist->begin(); i != realmlist->end(); i++)
    {
        if (i->second.build != this->client_build)
            continue;

      *pkt << (uint32) i->second.icon;
      if(this->client_build != BUILD_1_12)
          *pkt << (uint8)(i->second.allowedSecurityLevel > this->acct.gmlevel ? 1:0);
      *pkt << (uint8) i->second.color;
      *pkt << i->second.name;
      *pkt << i->second.address;
      *pkt << (float)i->second.population;
      if(this->realm_char_amount.find(i->first) != realm_char_amount.end())
	*pkt << this->realm_char_amount[i->first];
      else
	*pkt << (uint8) 0;
      *pkt << (uint8) i->second.timezone;
      if(this->client_build == BUILD_1_12)
        *pkt << (uint8) 0x00;
      else
        *pkt << (uint8) 0x2C;
    }

  if(this->client_build == BUILD_1_12)
  {
    *pkt << (uint8) 0x00;
    *pkt << (uint8) 0x02;
  }
  else
  {
    *pkt << (uint8) 0x10;
    *pkt << (uint8) 0x00;
  }

  ByteBuffer *data = new ByteBuffer;
  *data << (uint8)REALM_LIST;
  *data << (uint16) pkt->size();
  data->append(*pkt);
  delete pkt;
  this->send(data);
  //  this->send(pkt);
  this->set_vs();
  
}

void
Realm_Socket::ip_ban_checked(bool result)
{
  REALM_TRACE;
  if(result)
    {
      ByteBuffer buf(3);
      buf << (uint8) AUTH_LOGON_CHALLENGE;
      buf << (uint8) 0x00;
      buf << (uint8) REALM_AUTH_ACCOUNT_BANNED;
      this->send(new ByteBuffer(buf));
      return;
    }
  
  sRealm->get_db()->get_account(this->ptr);
  
}

void Realm_Socket::account_checked(AccountState state)
{
  REALM_TRACE;
  switch(state)
    {
    case ACCOUNT_BANNED:
      {
	ByteBuffer buf(3);
	buf << (uint8) AUTH_LOGON_CHALLENGE;
	buf << (uint8) 0x00;
	buf << (uint8) REALM_AUTH_ACCOUNT_BANNED;
	this->send(new ByteBuffer(buf));
	break;
      }
    case ACCOUNT_NOTFOUND:
      {
	ByteBuffer buf(3);
	buf << (uint8) AUTH_LOGON_CHALLENGE;
	buf << (uint8) 0x00;
	buf << (uint8) REALM_AUTH_NO_MATCH;
	this->send(new ByteBuffer(buf));
	break;
      }
    case ACCOUNT_EXISTS:
      {
	if(this->acct.locked == true)
	  if(this->ip != this->acct.last_ip)
	    {
	      REALM_LOG("IP lock kicked in yo!\n");
	      ByteBuffer buf(3);
	      buf << (uint8) AUTH_LOGON_CHALLENGE;
	      buf << (uint8) 0x00;
	      buf << (uint8) REALM_AUTH_ACCOUNT_BANNED;
	      this->send(new ByteBuffer(buf));
	      return;
	    }
	
	ByteBuffer *buf = new ByteBuffer;
	*buf << (uint8) AUTH_LOGON_CHALLENGE;
	*buf << (uint8) 0x00;
	*buf << (uint8) REALM_AUTH_SUCCESS;

	this->set_vs();
	
	BN_rand(b, 19*8,0,1);
	//Following snippet is courtesy of Derex.
	BIGNUM *temp = BN_new(), *temp2 = BN_new();
	BN_mod_exp(temp, g, b, N, ctx);
	BN_mul(temp2, v, k, ctx);
	BN_add(temp2, temp2, temp);
	BN_mod(B, temp2, N, ctx);
	
	BN_free(temp2);
	//</Derex's>

	uint8 tmp[32] = {0};
	BN_bn2bin(B, tmp);
	std::reverse(tmp, (uint8*)tmp+sizeof(tmp));
	buf->append(tmp, 32);
	*buf << (uint8)1;
	*buf << (uint8)7;
	*buf << (uint8)32;
	
	BN_bn2bin(N, tmp);
	std::reverse(tmp, (uint8*)tmp+sizeof(tmp));
	buf->append(tmp, 32);
	BN_bn2bin(s, tmp);
	std::reverse(tmp, (uint8*)tmp+sizeof(tmp));
	buf->append(tmp, 32);
	
	BN_rand(temp, 16*8, 0, 1);
	BN_bn2bin(temp, tmp);
	buf->append(tmp, 16);
	*buf << (uint8)0;
	BN_free(temp);
	this->send(buf);
	break;
      }
    default:
      REALM_LOG("Received unknown AccountState, THIS SHOULDN'T HAPPEN!\n");
      this->die();
      break;
    }

}

void
Realm_Socket::get_sessionkey(bool result)
{
  if(!result)
    {
      this->die();  //Client displays "Session Expired" on its own.
      return;
    }

  ByteBuffer* pkt = new ByteBuffer();
  *pkt << (uint8) AUTH_RECONNECT_CHALLENGE;
  *pkt << (uint8) 0x00;
  BN_rand(this->reconnect_proof, 16*8,0,1);
  uint8* rpr = new uint8[16];
  BN_bn2bin(this->reconnect_proof,rpr);
  pkt->append(rpr, 16);
  delete[] rpr;
  *pkt << (uint64) 0x00;
  *pkt << (uint64) 0x00;
  this->send(pkt);
  this->state = STATUS_NEED_RPROOF;
}

void
Realm_Socket::set_vs()
{
  SqlOperationRequest* fix = new SqlOperationRequest(REALMD_DB_FIX_SV);
  fix->add_string(1, this->login.c_str());
  sRealm->get_db()->enqueue_with_priority(fix, PRIORITY_HIGH);
  BIGNUM* I;
  BIGNUM* x;
  uint8 x_ch[SHA_DIGEST_LENGTH] = {0};
  uint8 p_ch[SHA_DIGEST_LENGTH] = {0};
  uint8 s_ch[32] = {0};
  BN_bn2bin(s, s_ch);
  std::reverse((uint8*)s_ch, (uint8*)s_ch + sizeof(s_ch));
  x = BN_new();
  
  I = NULL;
  BN_hex2bn(&I, this->acct.sha_pass.c_str());
  BN_bn2bin(I, p_ch);
  BN_free(I);
  
  SHA_CTX sha;
  SHA1_Init(&sha);
  SHA1_Update(&sha, s_ch, 32);
  SHA1_Update(&sha, p_ch, SHA_DIGEST_LENGTH);
  SHA1_Final(x_ch, &sha);
  std::reverse((uint8*)x_ch, (uint8*)x_ch + sizeof(x_ch));

  BN_bin2bn(x_ch, SHA_DIGEST_LENGTH, x);
  BN_mod_exp(this->v, this->g, x, this->N, this->ctx);  
  

  SqlOperationRequest* sv = new SqlOperationRequest(REALMD_DB_SET_S_V);
  const char* s_str = BN_bn2hex(s);
  const char* v_str = BN_bn2hex(v);
  
  sv->add_string(1, v_str);
  sv->add_string(2, s_str);
  sv->add_string(3, this->login.c_str());
  sRealm->get_db()->enqueue(sv);
  
  BN_free(x);
  ACE_OS::free((void*)s_str);
  ACE_OS::free((void*)v_str);
}

int
Realm_Socket::handle_output(ACE_HANDLE)
{
  REALM_TRACE;
  if(!this->out_active)
    return 0;
  ACE_Guard<ACE_Recursive_Thread_Mutex> g(this->queue_mtx);
  int error, sent_bytes;

  while (!this->packet_queue.empty())
    {
      ByteBuffer* buffer = this->packet_queue.front();
      sent_bytes = this->peer().send(buffer->contents(), buffer->size());
      //REALM_LOG("Sent %u bytes of %u\n",sent_bytes, buffer->size());
      error = errno;
      if (sent_bytes < buffer->size() && sent_bytes > 0)
        {
	  //  REALM_LOG("Sent %u of %u bytes \n",sent_bytes,buffer->size());
	  buffer->rpos(sent_bytes - 1);
	  buffer->read((uint8*)buffer->contents(), buffer->size() - sent_bytes);
	  buffer->resize(buffer->size() - sent_bytes);
	  break;
        }
      else
	if (sent_bytes == -1)
	  {
	    //REALM_LOG("Couldn't send to peer, bailing out size: %u!\n",buffer->size());
	    return -1;
	  }
	else
	  {
	    delete buffer;
	    packet_queue.pop_front();
	  }
    };

  if (this->packet_queue.empty())
    {
      sRealm->get_reactor()->cancel_wakeup(this, ACE_Event_Handler::WRITE_MASK);
      this->out_active = false;
      return 0;
    }
  else
    return 1;
}

int 
Realm_Socket::handle_close(ACE_HANDLE, ACE_Reactor_Mask mask)
{

  REALM_TRACE;
  if( mask == ACE_Event_Handler::WRITE_MASK )
    return 0;
  this->die();
  return 0;
}

void
Realm_Socket::die()
{
  REALM_TRACE;
  this->state = STATUS_CLOSING;
  ACE_Guard<ACE_Recursive_Thread_Mutex> g(this->queue_mtx);
  sRealm->get_reactor()->remove_handler(this,
					ACE_Event_Handler::READ_MASK |
					ACE_Event_Handler::WRITE_MASK |
					ACE_Event_Handler::DONT_CALL);

  this->peer().close();
  this->ptr.release();

}

void
Realm_Socket::send(ByteBuffer* pkt)
{
  REALM_TRACE;
  ACE_Guard<ACE_Recursive_Thread_Mutex> g(this->queue_mtx);
  packet_queue.push_back(pkt);


  if(!this->out_active)
    {
      sRealm->get_reactor()->register_handler(this,
					     ACE_Event_Handler::WRITE_MASK);
      //  sRealm->get_reactor()->notify(this,
      //			    ACE_Event_Handler::WRITE_MASK);
      this->out_active = true;
    }
  
}
};
