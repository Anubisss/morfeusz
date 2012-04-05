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
 *  @brief   TBC specific opcode handlers for proxyd
 *  @author  raczman <raczman@gmail.com>
 *  @date    2009-11-29
 *
 */

#include <openssl/bn.h>
#include <openssl/sha.h>

#include "Update_Fields.h"
#include "dbc/DBC_Store.h"
#include "Proxy_Service.h"
#include "Proxy_Socket.h"
#include "Proxy_Crypto.h"
#include "Opcodes.h"

// TODO: move this
enum ResponseCodes
{
    RESPONSE_SUCCESS                                       = 0x00,
    RESPONSE_FAILURE                                       = 0x01,
    RESPONSE_CANCELLED                                     = 0x02,
    RESPONSE_DISCONNECTED                                  = 0x03,
    RESPONSE_FAILED_TO_CONNECT                             = 0x04,
    RESPONSE_CONNECTED                                     = 0x05,
    RESPONSE_VERSION_MISMATCH                              = 0x06,

    CSTATUS_CONNECTING                                     = 0x07,
    CSTATUS_NEGOTIATING_SECURITY                           = 0x08,
    CSTATUS_NEGOTIATION_COMPLETE                           = 0x09,
    CSTATUS_NEGOTIATION_FAILED                             = 0x0A,
    CSTATUS_AUTHENTICATING                                 = 0x0B,

    AUTH_OK                                                = 0x0C,
    AUTH_FAILED                                            = 0x0D,
    AUTH_REJECT                                            = 0x0E,
    AUTH_BAD_SERVER_PROOF                                  = 0x0F,
    AUTH_UNAVAILABLE                                       = 0x10,
    AUTH_SYSTEM_ERROR                                      = 0x11,
    AUTH_BILLING_ERROR                                     = 0x12,
    AUTH_BILLING_EXPIRED                                   = 0x13,
    AUTH_VERSION_MISMATCH                                  = 0x14,
    AUTH_UNKNOWN_ACCOUNT                                   = 0x15,
    AUTH_INCORRECT_PASSWORD                                = 0x16,
    AUTH_SESSION_EXPIRED                                   = 0x17,
    AUTH_SERVER_SHUTTING_DOWN                              = 0x18,
    AUTH_ALREADY_LOGGING_IN                                = 0x19,
    AUTH_LOGIN_SERVER_NOT_FOUND                            = 0x1A,
    AUTH_WAIT_QUEUE                                        = 0x1B,
    AUTH_BANNED                                            = 0x1C,
    AUTH_ALREADY_ONLINE                                    = 0x1D,
    AUTH_NO_TIME                                           = 0x1E,
    AUTH_DB_BUSY                                           = 0x1F,
    AUTH_SUSPENDED                                         = 0x20,
    AUTH_PARENTAL_CONTROL                                  = 0x21,
    AUTH_LOCKED_ENFORCED                                   = 0x22,

    REALM_LIST_IN_PROGRESS                                 = 0x23,
    REALM_LIST_SUCCESS                                     = 0x24,
    REALM_LIST_FAILED                                      = 0x25,
    REALM_LIST_INVALID                                     = 0x26,
    REALM_LIST_REALM_NOT_FOUND                             = 0x27,

    ACCOUNT_CREATE_IN_PROGRESS                             = 0x28,
    ACCOUNT_CREATE_SUCCESS                                 = 0x29,
    ACCOUNT_CREATE_FAILED                                  = 0x2A,

    CHAR_LIST_RETRIEVING                                   = 0x2B,
    CHAR_LIST_RETRIEVED                                    = 0x2C,
    CHAR_LIST_FAILED                                       = 0x2D,

    CHAR_CREATE_IN_PROGRESS                                = 0x2E,
    CHAR_CREATE_SUCCESS                                    = 0x2F,
    CHAR_CREATE_ERROR                                      = 0x30,
    CHAR_CREATE_FAILED                                     = 0x31,
    CHAR_CREATE_NAME_IN_USE                                = 0x32,
    CHAR_CREATE_DISABLED                                   = 0x33,
    CHAR_CREATE_PVP_TEAMS_VIOLATION                        = 0x34,
    CHAR_CREATE_SERVER_LIMIT                               = 0x35,
    CHAR_CREATE_ACCOUNT_LIMIT                              = 0x36,
    CHAR_CREATE_SERVER_QUEUE                               = 0x37,
    CHAR_CREATE_ONLY_EXISTING                              = 0x38,
    CHAR_CREATE_EXPANSION                                  = 0x39,

    CHAR_DELETE_IN_PROGRESS                                = 0x3A,
    CHAR_DELETE_SUCCESS                                    = 0x3B,
    CHAR_DELETE_FAILED                                     = 0x3C,
    CHAR_DELETE_FAILED_LOCKED_FOR_TRANSFER                 = 0x3D,
    CHAR_DELETE_FAILED_GUILD_LEADER                        = 0x3E,
    CHAR_DELETE_FAILED_ARENA_CAPTAIN                       = 0x3F,

    CHAR_LOGIN_IN_PROGRESS                                 = 0x40,
    CHAR_LOGIN_SUCCESS                                     = 0x41,
    CHAR_LOGIN_NO_WORLD                                    = 0x42,
    CHAR_LOGIN_DUPLICATE_CHARACTER                         = 0x43,
    CHAR_LOGIN_NO_INSTANCES                                = 0x44,
    CHAR_LOGIN_FAILED                                      = 0x45,
    CHAR_LOGIN_DISABLED                                    = 0x46,
    CHAR_LOGIN_NO_CHARACTER                                = 0x47,
    CHAR_LOGIN_LOCKED_FOR_TRANSFER                         = 0x48,
    CHAR_LOGIN_LOCKED_BY_BILLING                           = 0x49,

    CHAR_NAME_SUCCESS                                      = 0x4A,
    CHAR_NAME_FAILURE                                      = 0x4B,
    CHAR_NAME_NO_NAME                                      = 0x4C,
    CHAR_NAME_TOO_SHORT                                    = 0x4D,
    CHAR_NAME_TOO_LONG                                     = 0x4E,
    CHAR_NAME_INVALID_CHARACTER                            = 0x4F,
    CHAR_NAME_MIXED_LANGUAGES                              = 0x50,
    CHAR_NAME_PROFANE                                      = 0x51,
    CHAR_NAME_RESERVED                                     = 0x52,
    CHAR_NAME_INVALID_APOSTROPHE                           = 0x53,
    CHAR_NAME_MULTIPLE_APOSTROPHES                         = 0x54,
    CHAR_NAME_THREE_CONSECUTIVE                            = 0x55,
    CHAR_NAME_INVALID_SPACE                                = 0x56,
    CHAR_NAME_CONSECUTIVE_SPACES                           = 0x57,
    CHAR_NAME_RUSSIAN_CONSECUTIVE_SILENT_CHARACTERS        = 0x58,
    CHAR_NAME_RUSSIAN_SILENT_CHARACTER_AT_BEGINNING_OR_END = 0x59,
    CHAR_NAME_DECLENSION_DOESNT_MATCH_BASE_NAME            = 0x5A,
};

namespace Morpheus
{

namespace Proxyd
{

void Proxy_Socket::handle_cmsg_auth_session()
{
    PROXY_TRACE;
    uint32 tmp;
    if (!this->in_packet->CheckSize(4 + 4+ 1 + 4 + 20))
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

void Proxy_Socket::account_retrieved(bool state)
{
    PROXY_TRACE;

    // The moment someone writes a hack that bypasses realmd 
    // and logs in somehow straight into game
    // Call me so i can give a beer to that wonderful hacker.
    // But still, we need to check if the account exists.
    // Client doesn't even react to that opcode, but lets send it 
    // For sake of being...
    // 0x15 is AUTH_STATE_ACCOUNT_UNKNOWN btw.
    if (!state) {
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

    if (ACE_OS::strcmp(v_char, this->acct.v.c_str())) {
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

    if (memcmp(check_digest, this->client_digest, 20)) {
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

void Proxy_Socket::handle_cmsg_char_enum()
{
    sProxy->get_db()->get_chars(this->ptr);
}

void Proxy_Socket::characters_retrieved(bool state)
{
    ServerPkt* pkt = new ServerPkt(SMSG_CHAR_ENUM, 1);
  
    if (!state) {
        *pkt << uint8(0);
        this->send(pkt);
        return;
    }
  
    PROXY_LOG("%u characters\n", this->characters.size());

    *pkt << (uint8)this->characters.size();
  
    for (std::list<Character>::const_iterator iter = this->characters.begin();iter != this->characters.end(); iter++) {
        PROXY_LOG("%s \n", iter->name.c_str());
        *pkt << uint64(uint64(iter->guid) | (uint64(0) <<24) | ( uint64(0) << 48) );
        *pkt << iter->name;
        *pkt << iter->race;
        *pkt << iter->pclass;
        *pkt << iter->gender;
        *pkt << uint8(iter->bytes);
        *pkt << uint8(iter->bytes >> 8);
        *pkt << uint8(iter->bytes >> 16);
        *pkt << uint8(iter->bytes >> 24);
        *pkt << iter->bytes2;
        *pkt << iter->level;
        *pkt << iter->zone;
        *pkt << iter->map;
        *pkt << iter->x;
        *pkt << iter->y;
        *pkt << iter->z;
        *pkt << iter->guild;

        /** @todo player & atlogin flags; */
        *pkt << uint32(0x00);//2002000); //iter->player_flags;

        *pkt << (uint8)0x01; //UNK
      
        /**
         * @todo Load dbc's and get pets family info...*/
	
        /* *pkt << iter->pet.modelid;
        *pkt << iter->pet.level;
        *pkt << family;*/
      
        *pkt << (uint32)0;
        *pkt << (uint32)0;
        *pkt << (uint32)0;
      
        for (uint8 slot = 0; slot < 19; slot++) {
            /**
             * @todo
             * *pkt << (uint32)enchant_id;
             */
            uint32 item_base = PLAYER_VISIBLE_ITEM_1_0 + (slot * 16);
            uint32 item_id = iter->update_fields[item_base];
            if (sDBC->get_item_map()->find(item_id) != sDBC->get_item_map()->end()) {
                const Morpheus::DBC::ItemEntry& item = sDBC->get_item_map()->find(item_id)->second;
                const Morpheus::DBC::SpellItemEnchantmentEntry* spell = NULL;
	      
                *pkt << (uint32)item.display_id;
                *pkt << (uint8)item.inventory_type;
	
                for (uint8 i = 0; i <= 1; i++) {
                    uint32 enchant_id = iter->update_fields[item_base + 1 + i];
              
                    if (sDBC->get_spell_item_ench_map()->find(enchant_id) 
                        != sDBC->get_spell_item_ench_map()->end())
                    {
                        spell = &sDBC->get_spell_item_ench_map()->find(enchant_id)->second;
                    }
                }

                *pkt << (uint32)(spell? spell->aura_id :0);

            }
            else {
                *pkt << (uint32)0;
                *pkt << (uint8)0;
                *pkt << (uint32)0;
            }
        }
        
        *pkt << (uint32)0;
        *pkt << (uint8)0;
        *pkt << (uint32)0;
    }
  
    this->send(pkt);
}

void Proxy_Socket::handle_cmsg_char_create()
{
    PROXY_TRACE;
    std::string name;
    uint8 race, pclass;
    uint8 gender, skin, face, hairStyle, hairColor, facialHair, outfitId;

    if (!this->in_packet->CheckSize(1+1+1+1+1+1+1+1+1+1))
        return;
    
    *this->in_packet >> name;
    
    // Recheck with name size
    if (!this->in_packet->CheckSize((name.size()+1)+1+1+1+1+1+1+1+1+1))
        return;
        
    *this->in_packet >> race;
    *this->in_packet >> pclass;
    
    ServerPkt* pkt = new ServerPkt(SMSG_CHAR_CREATE, 1);
    
    // TODO: Implement race/class creation disable system based on config
    
    const Morpheus::DBC::ChrRacesEntry* race_entry = NULL;
    if (sDBC->get_chr_races_map()->find(race) != sDBC->get_chr_races_map()->end())
        race_entry = &sDBC->get_chr_races_map()->find(race)->second;
        
    const Morpheus::DBC::ChrClassesEntry* class_entry = NULL;
    if (sDBC->get_chr_classes_map()->find(pclass) != sDBC->get_chr_classes_map()->end())
        class_entry = &sDBC->get_chr_classes_map()->find(pclass)->second;
        
    if (!race_entry || !class_entry) {
        *pkt << uint8(CHAR_CREATE_FAILED);
        this->send(pkt);
        PROXY_LOG("handle_cmsg_char_create: race %u or class %u not found in DBC.", race, pclass);
        return;
    }
    
    if (race_entry->addon > this->acct.expansion) {
        *pkt << uint8(CHAR_CREATE_EXPANSION);
        this->send(pkt);
        return;
    }
    
    if (!Utils::normalizePlayerName(name)) {
        *pkt << uint8(CHAR_NAME_INVALID_CHARACTER);
        this->send(pkt);
        return;
    }
    
    if (!Utils::isValidName(name)) {
        *pkt << uint8(CHAR_NAME_INVALID_CHARACTER);
        this->send(pkt);
        return;
    }

    DatabaseAccess::SqlOperationRequest* op = new SqlOperationRequest(PROXYD_DB_GET_PLR_GUID_FROM_NAME);
    op->add_string(1, name.c_str());
    SQL::ResultSet* res = sProxy->get_db()->enqueue_synch_query(op);
    
    if (res->rowsCount() != 0) {
        *pkt << uint8(CHAR_CREATE_NAME_IN_USE);
        this->send(pkt);
        return;
    }

    op = new SqlOperationRequest(PROXYD_DB_GET_NUMCHAR);
    op->add_uint32(1, this->acct.id);
    res = sProxy->get_db()->enqueue_synch_query(op);

    res->next();
    if (res->rowsCount() > 0 && res->getUint8(1) >= 50) { // TODO: config option
        *pkt << uint8(CHAR_CREATE_ACCOUNT_LIMIT);
        this->send(pkt);
        return;
    }
    
    op = new SqlOperationRequest(PROXYD_DB_GET_CHAR_COUNT);
    op->add_uint32(1, this->acct.id);
    res = sProxy->get_db()->enqueue_synch_query(op);
    
    res->next();
    if (res->rowsCount() > 0 && res->getUint8(1) >= 10) { // TODO: config option
        *pkt << uint8(CHAR_CREATE_SERVER_LIMIT);
        this->send(pkt);
        return;
    }
    
    // TODO: Check on AllowTwoSideAccounts
    
    *this->in_packet >> gender;
    *this->in_packet >> skin;
    *this->in_packet >> face;
    *this->in_packet >> hairStyle;
    *this->in_packet >> hairColor;
    *this->in_packet >> facialHair;
    *this->in_packet >> outfitId;
    
    /*PROXY_LOG("1: %s\n", sProxy->get_db()->getAutoCommit() ? "autocommit 1" : "autocommit 0");
    sProxy->get_db()->init_transaction();
    op = new SqlOperationRequest(PROXYD_DB_INCR_NUMCHAR);
    op->add_uint32(1, sProxy->get_realmid());
    op->add_uint32(2, this->acct.id);
    sProxy->get_db()->enqueue(op);
    sProxy->rollback_transaction();
    PROXY_LOG("2: %s\n", sProxy->get_db()->getAutoCommit() ? "autocommit 1" : "autocommit 0");*/
    
    SqlOperationTransaction* trans = new SqlOperationTransaction();
    op = new SqlOperationRequest(PROXYD_DB_INCR_NUMCHAR);
    op->add_uint32(1, sProxy->get_realmid());
    op->add_uint32(2, this->acct.id);
    trans->append(op);
    sProxy->get_db()->enqueue(trans);

    *pkt << uint8(CHAR_CREATE_SUCCESS);
    this->send(pkt);
}

};
};
