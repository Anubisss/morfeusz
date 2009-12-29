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
 *  @brief   Crypto implementation for TBC client version.
 *  @author  raczman <raczman@gmail.com>
 *  @date    2009-11-14
 *  @ingroup Proxy
 */

#include "Proxy_Crypto_Base.h"
#include "Crypto_Keys.h"

#include <openssl/hmac.h>
#include <openssl/sha.h>

namespace Trinity
{
namespace Proxyd
{

class Proxy_Crypto : public Proxy_Crypto_Base
{
 public:
  Proxy_Crypto();
  void encrypt(uint8* data, size_t len);
  void decrypt(uint8* data, size_t len);
  void set_key(BIGNUM* key);
 private:
  /**
   * @brief Header size for TBC server->client packets.
   */
  const static uint8 CRYPTED_SEND_LEN = 4;
  
  /**
   * @brief Header size for TBC client->server packets.
   */
  const static uint8 CRYPTED_RECV_LEN = 6;

  /**
   * @brief Encryption key.
   */
  uint8 key[SHA_DIGEST_LENGTH];

  /**
   * @brief We use this variable to check if key was set.
   */
  bool is_initialised;
  uint8 send_i, send_j;
  uint8 recv_i, recv_j;
};

};
};
