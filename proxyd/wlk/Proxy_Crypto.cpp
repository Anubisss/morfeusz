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
 *  @brief   WLK specific cryptography implementation.
 *  @author  thenecromancer <jaquboss@yahoo.com>
 *  @date    2009-11-22
 *
 */

#include "Proxy_Crypto.h"

namespace Morpheus
{
namespace Proxyd
{

Proxy_Crypto::Proxy_Crypto()
    :is_initialised(false)
{
}

Proxy_Crypto::~Proxy_Crypto()
{
    if (this->is_initialised) {
        EVP_CIPHER_CTX_cleanup(&this->out_ctx);
        EVP_CIPHER_CTX_cleanup(&this->in_ctx);
    }
}

void Proxy_Crypto::set_key(BIGNUM* key)
{
    // Init CTX
    HMAC_CTX ctx;
    HMAC_CTX_init(&ctx);

    uint8* hmac_key = new uint8[SEED_KEY_SIZE];
    uint8* outputKey = new uint8[SHA_DIGEST_LENGHT];

    // Encryption Key
    memcpy(hmac_key, &wlk_encryption_key, SEED_KEY_SIZE);
    HMAC_Init_ex(&ctx, hmac_key, SEED_KEY_SIZE, EVP_sha1(), NULL);
    uint8* tmp = new uint8[BN_num_bytes(key)];
    BN_bn2bin(key, tmp);
    HMAC_Update(&ctx, tmp, BN_num_bytes(key));
    memset(&outputKey, 0, SHA_DIGEST_LENGTH);
    HMAC_Final(&ctx, &outputKey, NULL);

    EVP_CIPHER_CTX_init(&this->out_ctx);
    EVP_CIPHER_CTX_set_key_length(&this->out_ctx, SHA_DIGEST_LENGTH);
    EVP_EncryptInit_ex(&this->out_ctx, EVP_rc4(), NULL, outputKey, NULL);

    // Decryption Key
    memcpy(hmac_key, &wlk_decryption_key, SEED_KEY_SIZE);
    HMAC_Init_ex(&ctx, hmac_key, SEED_KEY_SIZE, NULL, NULL);
    BN_bn2bin(key, tmp);
    HMAC_Update(&ctx, tmp, BN_num_bytes(key));
    memset(&outputKey, 0, SHA_DIGEST_LENGTH);
    HMAC_Final(&ctx, &outputKey, NULL);

    EVP_CIPHER_CTX_init(&this->in_ctx);
    EVP_CIPHER_CTX_set_key_length(&this->in_ctx, SHA_DIGEST_LENGTH);
    EVP_EncryptInit_ex(&this->in_ctx, EVP_rc4(), NULL, outputKey, NULL);

    // Finalize
    delete[] tmp;
    delete[] hmac_key;
    HMAC_CTX_cleanup(&ctx);
    this->is_initialised = true;
}

void Proxy_Crypto::encrypt(uint8* data, size_t len)
{
    if (!this->is_initialised)
        return;

    int outlen = 0;
    EVP_EncryptUpdate(&out_ctx, data, &outlen, data, len);
    EVP_EncryptFinal_ex(&out_ctx, data, &outlen);
}

void Proxy_Crypto::decrypt(uint8* data, size_t len)
{
    if (!this->is_initialised)
        return;

    int outlen = 0;
    EVP_EncryptUpdate(&in_ctx, data, &outlen, data, len);
    EVP_EncryptFinal_ex(&in_ctx, data, &outlen);
}

};
};
