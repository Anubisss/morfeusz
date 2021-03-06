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
 *  @brief   Proxies' database access classes.
 *  @author  raczman <raczman@gmail.com>
 *  @date    2009-11-24
 *  @ingroup Proxy
 */

#ifndef _PROXY_DATABASE_H
#define _PROXY_DATABASE_H
#include "Database.h"
#include "Callback.h"
#include "Proxy_Socket.h"

using namespace Morpheus::Proxyd;

namespace Morpheus
{

namespace DatabaseAccess
{

enum ProxyDatabaseStatements
{
    PROXYD_DB_GET_ACCT,                 // "SELECT id, gmlevel, sessionkey, sha_pass_hash, v, s, expansion FROM realmdb.account WHERE username = ?";
    PROXYD_DB_GET_CHAR,                 // "SELECT guid, name, race, class, gender, level, playerBytes, playerBytes2, position_x, position_y, position_z, map, zone FROM characters WHERE account = ?"
    PROXYD_DB_GET_PLR_GUID_FROM_NAME,   // "SELECT guid FROM characters WHERE name = ?"
    PROXYD_DB_GET_CHAR_COUNT,           // "SELECT COUNT(guid) FROM characters WHERE account = ?"
    PROXYD_DB_GET_NUMCHAR,              // "SELECT CAST(SUM(numchars) AS unsigned) FROM realmdb.realmcharacters WHERE acctid = ?"
    PROXYD_DB_INCR_NUMCHAR,             // "UPDATE realmdb.realmcharacters SET numchars = numchars + 1 WHERE realmid = ? AND acctid = ?"
    PROXYD_DB_INS_CHAR,                 // "INSERT INTO characters (guid, account, name, race, class, gender, level, zone, map, position_x, position_y, position_z) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)"
    PROXYD_DB_LOAD_PLR_CREATEINFO,      // "SELECT race, class, map, zone, position_x, position_y, position_z FROM player_createinfo"
    PROXYD_DB_GET_CHAR_MAX_GUID,        // "SELECT MAX(guid) FROM characters"
    PROXYD_DB_STMT_MAX
};

class ProxyDatabaseConnection : protected DatabaseConnection
{

public:

    ProxyDatabaseConnection(ACE_Activation_Queue* q): DatabaseConnection(q){};
    ~ProxyDatabaseConnection(){if(worker) delete worker;};
    bool open(const std::string& driver, const std::string& url);
};

class ProxyDB : public DatabaseHolder<ProxyDatabaseConnection>
{

public:

    ProxyDB(int c): DatabaseHolder<ProxyDatabaseConnection>(c){}

    /**
     * @brief Retrieves user account from database
     * @sa Proxy_Socket::handle_cmsg_auth_session
     */
    void get_account(Proxy_Sock_Ptr sock);
    void get_chars(Proxy_Sock_Ptr sock);
    void load_player_createinfo();
};

};
};
#endif //_PROXY_DATABASE_H
