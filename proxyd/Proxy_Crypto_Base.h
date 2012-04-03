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
 *  @brief   Abstract class for packet encryption.
 *  @author  raczman <raczman@gmail.com>
 *  @date    2009-11-14
 *  @ingroup Proxy
 */

#include "Common.h"
#include <openssl/bn.h>

namespace Morpheus
{
namespace Proxyd
{

/**
 * @brief Base class for encryption.
 * @details Encryption method varies across versions,
 *          so to preserve flexibility we use abstract
 *          class that will have to be implemented in 
 *          version specific encryption classes.
 */
class Proxy_Crypto_Base
{

public:

  /**
   * @brief Encrypts data sent to client.
   */
  virtual void encrypt(uint8* data, size_t len) = 0;
 
  /**
   * @brief Decrypts received data.
   */
  virtual void decrypt(uint8* data, size_t len) = 0;

  /**
   * @brief Sets key for encryption algorithm.
   */
  virtual void set_key(BIGNUM* key) = 0;
};

};
};
