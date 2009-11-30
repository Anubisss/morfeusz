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
 *  @brief   TBC specific opcode handlers for proxyd
 *  @author  raczman <raczman@gmail.com>
 *  @date    2009-11-29
 *
 */

#include <openssl/bn.h>
#include <openssl/sha.h>

#include "Proxy_Service.h"
#include "Proxy_Socket.h"
#include "Proxy_Crypto.h"
#include "Opcodes.h"

namespace Trinity
{
namespace Proxyd
{

void
Proxy_Socket::handle_cmsg_auth_session()
{
  PROXY_TRACE;
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

  sProxy->get_db()->get_account(this->ptr);
}


void 
Proxy_Socket::account_retrieved(bool state)
{
  PROXY_TRACE;

  // The moment someone writes a hack that bypasses realmd 
  // and logs in somehow straight into game
  // Call me so i can give a beer to that wonderful hacker.
  // But still, we need to check if the account exists.
  // Client doesn't even react to that opcode, but lets send it 
  // For sake of being...
  // 0x15 is AUTH_STATE_ACCOUNT_UNKNOWN btw.
  if(!state)
    {
      ServerPkt* pkt = new ServerPkt(SMSG_AUTH_RESPONSE, 1);
      *pkt << uint8(0x15);
      this->send(pkt);
      this->die();
      return;
    }

  //Prepare vars
  BIGNUM* N = BN_new();
  BIGNUM* g = BN_new();
  BIGNUM* I = BN_new();
  BIGNUM* s = BN_new();
  BIGNUM* v = BN_new();
  BIGNUM* x = BN_new();
  BIGNUM* K = BN_new();
  BN_CTX* ctx = BN_CTX_new();
  uint8 pass_hash[SHA_DIGEST_LENGTH] = {0};
  uint8 digest[SHA_DIGEST_LENGTH] = {0};
  uint8* s_bin;

  //Loading data
  BN_hex2bn(&N, "894B645E89E1535BBDAD5B8B290650530801B18EBFBF5E8FAB3C82872A3E9BB7");
  BN_set_word(g, 7);
  BN_hex2bn(&I, this->acct.sha_pass_hash.c_str());
  BN_hex2bn(&s, this->acct.s.c_str());

  s_bin = new uint8[BN_num_bytes(s)];
  BN_bn2bin(s, s_bin); 
  BN_bn2bin(I, pass_hash);

  std::reverse(s_bin, (uint8*)s_bin + BN_num_bytes(s) );
  
  //  std::reverse(pass_hash, pass_hash + SHA_DIGEST_LENGTH);

  SHA_CTX* sha = new SHA_CTX();
  SHA1_Init(sha);
  SHA1_Update(sha, s_bin, BN_num_bytes(s) );
  SHA1_Update(sha, pass_hash, SHA_DIGEST_LENGTH);
  SHA1_Final(digest, sha);
  delete sha;
  std::reverse(digest, digest + SHA_DIGEST_LENGTH);
  BN_bin2bn(digest, SHA_DIGEST_LENGTH, x);
  BN_mod_exp(v, g, x, N, ctx);

  char* s_char = BN_bn2hex(s);
  char* v_char = BN_bn2hex(v);

  PROXY_LOG("s: %s \n\told v:%s\n\t v=%s\n", this->acct.s.c_str(),this->acct.v.c_str(),v_char);

  if(ACE_OS::strcmp(v_char, this->acct.v.c_str()))
    {
      BN_free(K);
      BN_free(N);
      BN_free(g);
      BN_free(I);
      BN_free(s);
      BN_free(v);
      BN_free(x);
      BN_CTX_free(ctx);
      ACE_OS::free(s_char);
      ACE_OS::free(v_char);
      delete[] s_bin;
      this->die();
      return;
    }
  

 
  
  uint8 check_digest[SHA_DIGEST_LENGTH];
  uint32 trailer = 0x00;
  BN_hex2bn(&K, this->acct.sessionkey.c_str());

  uint8* k_char = new uint8[BN_num_bytes(K)];
  BN_bn2bin(K, k_char);
  std::reverse(k_char, k_char + 40);
  sha = new SHA_CTX();

  SHA1_Init(sha);
  SHA1_Update(sha, this->login.c_str(), this->login.length());
  SHA1_Update(sha, &trailer, 4);
  SHA1_Update(sha, &this->client_seed, 4);
  SHA1_Update(sha, &this->seed, 4);
  SHA1_Update(sha, k_char, BN_num_bytes(K));
  SHA1_Final(check_digest, sha);
  delete sha;

  //std::reverse(check_digest, check_digest + SHA_DIGEST_LENGTH);

  PROXY_LOG("memcmp %u\n",memcmp(check_digest, this->client_digest,20));

  if(memcmp(check_digest, this->client_digest, 20))
    {
      BN_free(K);
      BN_free(N);
      BN_free(g);
      BN_free(I);
      BN_free(s);
      BN_free(v);
      BN_free(x);
      BN_CTX_free(ctx);
      ACE_OS::free(s_char);
      ACE_OS::free(v_char);

      this->die();
      return;
    }

  this->crypto->set_key(K);

  ServerPkt* pkt = new ServerPkt(SMSG_AUTH_RESPONSE, 1 + 4 + 1 + 4 + 1);
  *pkt << (uint8)0x0C;  //AUTH_OK
  *pkt << (uint32)0x00;
  *pkt << (uint8)0x00;
  *pkt << (uint32)0x00;
  *pkt << (uint8)this->acct.expansion;
  this->send(pkt);

  BN_free(K);
  BN_free(N);
  BN_free(g);
  BN_free(I);
  BN_free(s);
  BN_free(v);
  BN_free(x);
  BN_CTX_free(ctx);
  ACE_OS::free(s_char);
  ACE_OS::free(v_char);
  

}

};
};
