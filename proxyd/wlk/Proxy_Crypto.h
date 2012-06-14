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
 *  @brief   Crypto implementation for WLK client version.
 *  @author  thenecromancer <jaquboss@yahoo.com>
 *  @date    2009-11-22
 *
 */

#include "Proxy_Crypto_Base.h"
#include "Crypto_Keys.h"

#include <openssl/hmac.h>
#include <openssl/sha.h>

namespace Morpheus
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
     * @brief We use this variable to check if key was set.
     */
    bool is_initialised;

    EVP_CIPHER_CTX    in_ctx;
    EVP_CIPHER_CTX    out_ctx;
};

};
};
