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
 *  @brief   Contains base class for RealmDB and DB operation observers.
 *  @author  raczman <raczman@gmail.com>
 *  @date    2009-10-24
 *  @ingroup Realmd
 *
 */


#pragma once
#ifndef REALM_DATABASE_H
#define REALM_DATABASE_H
#include "Database.h"
#include "Callback.h"
#include "Realm_Socket.h"

#include <ace/Refcounted_Auto_Ptr.h>
#include <openssl/bn.h>

using namespace Trinity::Realmd;

namespace Trinity
{

namespace DatabaseAccess
{

  /**
   * @brief Realmd's database statements are indexed by values from this enum.
   */
enum RealmdDatabaseStatements
{
  REALMD_DB_SET_S_V = 0,
  REALMD_DB_PRUNE_BANS,
  REALMD_DB_SET_INACTIVE_BANS,
  REALMD_DB_UPDATE_ACCOUNT,
  REALMD_DB_SET_FAILED_LOGINS,
  REALMD_DB_ADD_ACCOUNT_AUTOBAN,
  REALMD_DB_ADD_IP_AUTOBAN,
  REALMD_DB_CHECK_IP_BAN,
  REALMD_DB_GET_ACCT,
  REALMD_DB_CHECK_ACCT_BAN,
  REALMD_DB_CHECK_FAILED_LOGIN,
  REALMD_DB_GET_SESSIONKEY,
  REALMD_DB_GET_PASS_HASH,
  REALMD_DB_GET_NUMCHAR,
  REALMD_DB_PING,
  REALMD_DB_GET_REALMLIST,
  REALMD_DB_FIX_SV,  //this is here to get around bug in TC1/2
  REALMD_DB_STMT_MAX
};

  /**
   * @brief Observer for retrieveing sessionkey from database, used when reconnecting.
   * @see Realm_Socket::get_sessionkey(bool)
   */
  template <class C>
  class getSessionKeyObsv : public SqlOperationObserver<C, bool> 
  {
  public:
    getSessionKeyObsv(Callback<C, bool> c): SqlOperationObserver<C, bool>(c){}
    void update(const ACE_Future<SQL::ResultSet*> &future)
    {
      SQL::ResultSet* res;
      future.get(res);
      res->next();
      if(res->rowsCount())
	{
	  SqlOperationObserver<C, bool>::callback.call(false);
	}
      else
	{
	  BN_hex2bn(&(SqlOperationObserver<C, bool>::callback.get_obj()->k), res->getString(1).c_str());
	  SqlOperationObserver<C, bool>::callback.call(true);
	}

      delete res;
      delete this;
    }
  };

  /**
   * @brief Observer for retrieving amount of characters on realms.
   * @see Realm_Socket::get_char_amount(std::map<uint8, uint8>)
   */
  template <class C>
  class getCharAmntObsv : public SqlOperationObserver<C, std::map<uint8, uint8> >
  {
  public:
  getCharAmntObsv(Callback<C, std::map<uint8, uint8> > c): SqlOperationObserver<C, std::map<uint8, uint8> >(c){}
    void update(const ACE_Future<SQL::ResultSet*> &future)
    {
      SQL::ResultSet* res;
      future.get(res);
      std::map<uint8, uint8> amnt;
      if(res->rowsCount() == 0)
	{
	  SqlOperationObserver<C, std::map<uint8, uint8> > ::callback.call(amnt);
	  return;
	}

      while(res->next())
	{
	  amnt[res->getUint8(1)] = res->getUint8(2);
	};
      SqlOperationObserver<C, std::map<uint8, uint8> > ::callback.call(amnt);
      delete res;
      delete this;
    }
  };

  /**
   * @brief Observer for fetching realmlist from database.
   * @see Realm_Service::update_realms(Trinity::SQL::ResultSet*)
   */
template <class C>
  class getRealmListObsv : public SqlOperationObserver<C, SQL::ResultSet*>
  {
  public:
  getRealmListObsv(Callback<C, SQL::ResultSet*> c) : SqlOperationObserver<C, SQL::ResultSet*>(c) {}
    void update(const ACE_Future<SQL::ResultSet*> &fut)
    {
      SQL::ResultSet* result;
      fut.get(result);
      SqlOperationObserver<C, SQL::ResultSet*>::callback.call(result);

      delete result;
      delete this;
    }
  };

  /**
   * @brief Observer for checking IP ban on new connections.
   * @see Realm_Socket::ip_ban_checked(bool)
   */
template <class C>
  class checkIpBanObsv : public SqlOperationObserver<C, bool>
  {
  public:
  checkIpBanObsv(Callback<C, bool> c): SqlOperationObserver<C, bool>(c){}
    
    void update(const ACE_Future<SQL::ResultSet*> &future)
    {
      SQL::ResultSet* res;
      future.get(res);
      res->next();
      if(res->rowsCount() == 0)
	SqlOperationObserver<C, bool>::callback.call(false);
      else
	SqlOperationObserver<C, bool>::callback.call(true);
      
      
      delete res;
      delete this;
    }
  };

  /**
   * @brief Observer for retrieving and checking account ban.
   * @see Realm_Socket::account_checked(AccountState)
   */
template <class C>
  class checkAcctObsv : public SqlOperationObserver<C, AccountState>
  {
  public:
  checkAcctObsv(Callback<C, AccountState> c): 
    SqlOperationObserver<C, AccountState>(c){}
    
    void update(const ACE_Future<SQL::ResultSet*> &future)
    {
      SQL::ResultSet* res;
      future.get(res);
      res->next();
      
      if(res->rowsCount() == 0 )
	SqlOperationObserver<C, AccountState>::callback.call(ACCOUNT_NOTFOUND);
      else if(res->getUint8(6) != 0)
	SqlOperationObserver<C, AccountState>::callback.call(ACCOUNT_BANNED);
      else
	{
	  Account acc;
	  acc.sha_pass = res->getString(1);
	  acc.id = res->getUint64(2);
	  acc.locked = res->getBool(3);
	  acc.last_ip = res->getString(4);
	  acc.gmlevel = res->getUint8(5);
	  acc.failed_logins = res->getUint8(7);
	  SqlOperationObserver<C, AccountState>::callback.get_obj()->acct = acc;
	  SqlOperationObserver<C, AccountState>::callback.call(ACCOUNT_EXISTS);
	}
      
      delete res;
      delete this;
    }
  };

class RealmDatabaseConnection : protected DatabaseConnection
{
public:
  RealmDatabaseConnection(ACE_Activation_Queue*);
  ~RealmDatabaseConnection();
  bool open(const std::string& driver, const std::string& url);
  
};

typedef DatabaseHolder<RealmDatabaseConnection> RealmDBInh;

/**
 * @brief This class encapsules most of database access functions.
 */
class RealmDB : public RealmDBInh
{
 public:
  RealmDB(int c): RealmDBInh(c){}

  /**
   * @brief Checks if IP is banned, passing result to conn through callback.
   * @param conn Refcounted pointer to Realm_Socket
   * @see Realm_Socket::ip_ban_checked(bool)
   */
  void check_ip_ban(Realm_Sock_Ptr conn);

  /**
   * @brief Fetches account from database, callbacks into Realm_Socket
   * @see Realm_Socket::account_checked(AccountState)
   */
  void get_account(Realm_Sock_Ptr conn);

  /**
   * @brief Retrieves amount of characters on each of realms.
   *        Callback passes the result to Realm_Socket.
   * @see Realm_Socket::get_char_amount(std::map<uint8, uint8>)
   */
  void get_char_amount(Realm_Sock_Ptr conn);

  /**
   * @brief Fetches sessionkey for account from database.
   * 
   */
  void get_sessionkey(Realm_Sock_Ptr conn);

  /**
   * @brief Increments number of failed logins on account, 
   *        mainly to handle failed logins autobanning.
   * @param id Account id.
   */
  void increment_failed_logins(uint64 id);

  /**
   * @brief Bans specified account id for failed logins.
   * @param id Account id.
   * @see RealmDB::ban_failed_logins(const std::string&)
   */
  void ban_failed_logins(uint64 id);

  /**
   * @brief Bans IP for failed logins.
   * @param ip IP to ban.
   * @see RealmDB::ban_failed_logins(uint64)
   */
  void ban_failed_logins(const std::string &ip);

  /**
   * @brief Retrieves realmlist from DB, and callbacks
   *        Realm_Service to set its realmlist.
   */
  void get_realmlist();

  void fix_sv(std::string& login);
  void set_sv(std::string& login, const char* s_str, const char* v_str);
  void update_account(std::string login, std::string ip, uint8* K_buff);
};

}

}

#else
#endif
