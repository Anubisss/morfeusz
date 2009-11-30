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
 *  @brief   TBC specific cryptography implementation.
 *  @author  raczman <raczman@gmail.com>
 *  @date    2009-11-14
 *
 */

#include <algorithm>
#include "Proxy_Crypto.h"

namespace Trinity
{
namespace Proxyd
{

Proxy_Crypto::Proxy_Crypto()
  :is_initialised(false)
{
  memset(&this->key, 0, SHA_DIGEST_LENGTH);
  send_i = send_j = recv_i = recv_j = 0;
}

void
Proxy_Crypto::set_key(BIGNUM* key)
{
  HMAC_CTX ctx;
  uint8* hmac_key = new uint8[SEED_KEY_SIZE];
  memcpy(hmac_key, &tbc_encryption_key, SEED_KEY_SIZE);
  
  uint8* tmp = new uint8[BN_num_bytes(key)];
  BN_bn2bin(key, tmp);
  std::reverse(tmp, tmp + BN_num_bytes(key));

  HMAC_CTX_init(&ctx);
  HMAC_Init_ex(&ctx, hmac_key, SEED_KEY_SIZE, EVP_sha1(), NULL);
  HMAC_Update(&ctx, tmp, BN_num_bytes(key));
  HMAC_Final(&ctx, this->key, NULL);
  HMAC_CTX_cleanup(&ctx); 
  
  delete[] hmac_key;
  delete[] tmp;
  this->is_initialised = true;
}

void
Proxy_Crypto::encrypt(uint8* data, size_t)
{
  if(!this->is_initialised)
    return;

  for(uint8 t = 0; t < CRYPTED_SEND_LEN; t++)
    {
      send_i %= SHA_DIGEST_LENGTH;
      uint8 x = (data[t] ^ (this->key[send_i])) + send_j;
      ++send_i;
      data[t] = send_j = x;
    }
}

void
Proxy_Crypto::decrypt(uint8* data, size_t)
{
  if(!this->is_initialised)
    return;

  for(uint8 t = 0; t < CRYPTED_RECV_LEN; t++)
    {
      recv_i %= SHA_DIGEST_LENGTH;
      uint8 x = (data[t] - recv_j) ^ (this->key[recv_i]);
      ++recv_i;
      recv_j = data[t];
      data[t] = x;
    }

}

};
};
