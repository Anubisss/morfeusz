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
 *  @brief   Implementation of RealmDB class
 *  @author  raczman <raczman@gmail.com>
 *  @date    2009-10-24
 *  @ingroup Realmd
 *
 */

#include "Realm_Database.h"
#include "Realm_Socket.h"
#include "Realm_Service.h"

using namespace Morpheus::DatabaseAccess::Realmd;

namespace Morpheus
{

namespace DatabaseAccess
{

    RealmDatabaseConnection::RealmDatabaseConnection(ACE_Activation_Queue* q) 
        : DatabaseConnection(q)
    {} 

    RealmDatabaseConnection::~RealmDatabaseConnection()
    {
        if(worker)
        delete worker;
    }

#define ADD_STMT(x, y) this->statement_holder[x] = this->connection->prepareStatement(y)
bool RealmDatabaseConnection::open(const std::string& driver, const std::string& url)
{
    try {
        this->driver = Morpheus::SQL::DriverManager::getDriver(driver);
        this->connection = this->driver->connect(url);

        this->statement_holder.resize(REALMD_DB_STMT_MAX);

        ADD_STMT(REALMD_DB_SET_INACTIVE_BANS, "UPDATE account_banned SET active = 0 WHERE unbandate<=UNIX_TIMESTAMP() AND unbandate<>bandate");
        ADD_STMT(REALMD_DB_PRUNE_BANS, "DELETE FROM ip_banned WHERE unbandate<=UNIX_TIMESTAMP() AND unbandate<>bandate");
        ADD_STMT(REALMD_DB_CHECK_IP_BAN, "SELECT * FROM ip_banned WHERE ip = ?");
        ADD_STMT(REALMD_DB_CHECK_ACCT_BAN, "SELECT UPPER(a.sha_pass_hash) , a.id, a.locked, a.last_ip, ab.active, a.failed_logins FROM account AS a LEFT OUTER JOIN account_banned AS ab ON ab.id = a.id AND ab.active = 1 WHERE a.username = ?");
        ADD_STMT(REALMD_DB_SET_S_V, "UPDATE account SET v = ?, s = ? WHERE username = ?");
        ADD_STMT(REALMD_DB_UPDATE_ACCOUNT, "UPDATE account SET sessionkey = ?, last_ip = ?, last_login = NOW(), locale = ?, failed_logins = 0 WHERE username = ?");
        ADD_STMT(REALMD_DB_GET_REALMLIST, "SELECT id, name, icon, color, timezone, allowedSecurityLevel, population, gamebuild FROM realmlist WHERE color <> 3 ORDER BY id");
        ADD_STMT(REALMD_DB_GET_NUMCHAR, "SELECT realmid, numchars FROM realmcharacters WHERE acctid = ?");
        ADD_STMT(REALMD_DB_UPDATE_ACCOUNT,"UPDATE account SET sessionkey = ?, last_ip = ?, last_login = NOW(), locale = ?, failed_logins = 0 WHERE username = ?");
        ADD_STMT(REALMD_DB_FIX_SV, "UPDATE account set s ='', v ='' where username = ?");
        ADD_STMT(REALMD_DB_SET_FAILED_LOGINS, "UPDATE account set failed_logins = failed_logins + 1 where id = ?");
        ADD_STMT(REALMD_DB_ADD_IP_AUTOBAN, "INSERT INTO ip_banned VALUES (?,UNIX_TIMESTAMP(),UNIX_TIMESTAMP()+ ?,'Morpheus realmd','Failed login autoban')");
        ADD_STMT(REALMD_DB_ADD_ACCOUNT_AUTOBAN, "INSERT INTO account_banned VALUES (? ,UNIX_TIMESTAMP(),UNIX_TIMESTAMP()+ ?,'Morpheus realmd','Failed login autoban',1)");
        ADD_STMT(REALMD_DB_GET_SESSIONKEY, "SELECT sessionkey FROM account WHERE username = ?");

        this->worker = new DatabaseWorker(this->query_queue, this);
    }
    catch (SQL::SQLException& e) {
        ACE_ERROR((LM_ERROR,"Error when setting up connection: %s\n",e.what() ));
        ACE_OS::exit(-1);
    }
    
    return true;
}
#undef ADD_STMT

void RealmDB::ban_failed_logins(uint64 id)
{
    SqlOperationRequest* op = new SqlOperationRequest(REALMD_DB_ADD_ACCOUNT_AUTOBAN);
    op->add_uint32(1, id);
    op->add_uint32(2, 600); // FIXME: Move to config
    this->enqueue(op);
}

void RealmDB::ban_failed_logins(const std::string &ip)
{
    SqlOperationRequest* op = new SqlOperationRequest(REALMD_DB_ADD_IP_AUTOBAN);
    op->add_string(1, ip.c_str());
    op->add_uint32(2, 600); // FIXME: Move to config
    this->enqueue(op);
}

void RealmDB::increment_failed_logins(uint64 id)
{
    SqlOperationRequest* op = new SqlOperationRequest(REALMD_DB_SET_FAILED_LOGINS);
    op->add_uint64(1, id);
    this->enqueue(op);
}

void RealmDB::fix_sv(std::string& login)
{
    SqlOperationRequest* fix = new SqlOperationRequest(REALMD_DB_FIX_SV);
    fix->add_string(1, login.c_str());
    this->enqueue_with_priority(fix, PRIORITY_HIGH);
}

void RealmDB::set_sv(std::string& login, const char* s_str, const char* v_str)
{
    SqlOperationRequest* sv = new SqlOperationRequest(REALMD_DB_SET_S_V);

    sv->add_string(1, v_str);
    sv->add_string(2, s_str);
    sv->add_string(3, login.c_str());
    this->enqueue(sv);

    ACE_OS::free((void*)s_str);
    ACE_OS::free((void*)v_str);
}

void RealmDB::update_account(std::string login, std::string ip, uint8* K_buff)
{
    SqlOperationRequest* op = new SqlOperationRequest(REALMD_DB_UPDATE_ACCOUNT);
    BIGNUM* K = BN_new();
    std::reverse((uint8*)K_buff, (uint8*)K_buff + 40);
    BN_bin2bn(K_buff, 40, K);
    const char* K_hexb = BN_bn2hex(K);
    BN_free(K);

    op->add_string(1, K_hexb);
    free((void*)K_hexb);

    op->add_string(2, ip.c_str());
    op->add_uint8(3, 0);
    op->add_string(4, login.c_str());
    this->enqueue(op);
}

void RealmDB::check_ip_ban(Realm_Sock_Ptr conn)
{
    if(conn.null())
        return;
    ACE_Future<SQL::ResultSet*> result;
    result.attach(new checkIpBanObsv<Realm_Socket>
        (Callback<Realm_Socket, bool>
        (conn, &Realm_Socket::ip_ban_checked)
        ));
    SqlOperationRequest* op = new SqlOperationRequest(REALMD_DB_CHECK_IP_BAN, result);
    op->add_string(1, conn->get_ip().c_str() );
    this->enqueue(op);
}

void RealmDB::get_account(Realm_Sock_Ptr conn)
{
    if(conn.null())
        return;
    ACE_Future<SQL::ResultSet*> result;
    SqlOperationRequest* op = new SqlOperationRequest(REALMD_DB_CHECK_ACCT_BAN, result);
    result.attach(new checkAcctObsv<Realm_Socket>
        (Callback<Realm_Socket, AccountState>
        (conn, &Realm_Socket::account_checked)
        ));
    op->add_string(1, conn->get_login().c_str());
    this->enqueue(op);
}

void RealmDB::get_char_amount(Realm_Sock_Ptr conn)
{
    if(conn.null())
        return;
    ACE_Future<SQL::ResultSet*> res;
    res.attach(new getCharAmntObsv<Realm_Socket>
        (Callback<Realm_Socket, std::map<uint8, uint8> >
        (conn, &Realm_Socket::get_char_amount)
        ));
    SqlOperationRequest* op = new SqlOperationRequest(REALMD_DB_GET_NUMCHAR, res);
    op->add_uint32(1, conn->acct.id);
    this->enqueue(op);
}

void RealmDB::get_sessionkey(Realm_Sock_Ptr conn)
{
    if(conn.null())
        return;
    ACE_Future<SQL::ResultSet*> res;
    res.attach(new getSessionKeyObsv<Realm_Socket>
        (Callback<Realm_Socket, bool>
        (conn, &Realm_Socket::get_sessionkey)
        ));
    SqlOperationRequest* op = new SqlOperationRequest(REALMD_DB_GET_SESSIONKEY, res);
    op->add_string(1, conn->get_login().c_str());
    this->enqueue(op);
}

void RealmDB::get_realmlist()
{
    ACE_Future<SQL::ResultSet*> res;
    SqlOperationRequest* op = new SqlOperationRequest(REALMD_DB_GET_REALMLIST,res);
    res.attach(new getRealmListObsv<Realm_Service>
        (Callback<Realm_Service, SQL::ResultSet*>
        (sRealm, &Realm_Service::update_realms)
        ));
    this->enqueue(op);
}

};
};
