#pragma once
#ifndef REALM_DATABASE_H
#define REALM_DATABASE_H
#include "Database.h"
#include "Callback.h"
#include "Realm_Socket.h"

#include <ace/Refcounted_Auto_Ptr.h>

class Realm_Socket;

namespace Trinity
{

namespace DatabaseAccess
{

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
      else if(res->getInt32(6) != 0)
	SqlOperationObserver<C, AccountState>::callback.call(ACCOUNT_BANNED);
      else
	{
	  Account acc;
	  acc.sha_pass = res->getString(1);
	  acc.id = res->getUint64(2);
	  acc.locked = res->getBool(3);
	  acc.last_ip = res->getString(4);
	  acc.gmlevel = res->getUint8(5);
	  acc.failed_logins = res->getUint8(6);
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

class RealmDB : public RealmDBInh
{
 public:
 RealmDB(int c): RealmDBInh(c){}
  void check_ip_ban(Realm_Sock_Ptr conn);
  void get_account(Realm_Sock_Ptr conn);
  void get_char_amount(Realm_Sock_Ptr conn);
  void increment_failed_logins(uint64 id);
  void ban_failed_logins(uint64 id);
  void ban_failed_logins(const std::string &ip);
  void get_realmlist();
};

}

}

#else
#endif
