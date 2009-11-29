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
 *  @brief   Proxy database implementation.
 *  @author  raczman <raczman@gmail.com>
 *  @date    2009-11-24
 *
 */

#include "Proxy_Database.h"
#include "Configuration.h"

using namespace Trinity::Proxyd;

namespace Trinity
{
namespace DatabaseAccess
{

namespace Proxyd
{
  template <class C>
  class getAcctObsv : public SqlOperationObserver<C, bool>
  {
  public:
    getAcctObsv(Callback<C, bool> c): SqlOperationObserver<C, bool>(c){}
    void update(const ACE_Future<SQL::ResultSet*> &future)
    {
      SQL::ResultSet* res;
      future.get(res);

      if(res->rowsCount() == 0)
	{
	  SqlOperationObserver<C, bool>::callback.call(false);
	}
      else
	{
	  res->next();
	  Account acct;
	  acct.id = res->getUint64(1);
	  acct.gmlevel = res->getUint8(2);
	  acct.sessionkey = res->getString(3);
	  acct.sha_pass_hash = res->getString(4);
	  acct.v = res->getString(5);
	  acct.s = res->getString(6);
	  acct.expansion = res->getUint8(7);
	  SqlOperationObserver<C, bool>::callback.get_obj()->set_account(acct);
	  SqlOperationObserver<C, bool>::callback.call(true);
	}

      delete res;
      delete this;
    }
  };

};
  

#define ADD_STMT(x, y) this->statement_holder[x] = this->connection->prepareStatement(y)
bool
ProxyDatabaseConnection::open(const std::string& driver, const std::string& url)
  try
    {
      std::string realmdb = sConfig->getString("proxyd","RealmDB");
      std::string query;
      this->driver = Trinity::SQL::DriverManager::getDriver(driver);
      this->connection = this->driver->connect(url);
      this->statement_holder.resize(PROXYD_DB_STMT_MAX);
      query += "SELECT id, gmlevel, sessionkey, sha_pass_hash, v, s, expansion FROM ";
      query += realmdb;
      query += ".account WHERE username = ?";
      ADD_STMT(PROXYD_DB_GET_ACCT, query.c_str());
  
      this->worker = new DatabaseWorker(this->query_queue, this);
      return true;
    }
  catch(SQL::SQLException& e)
    {
	ACE_ERROR((LM_ERROR,"Error when setting up connection: %s\n",e.what() ));
	ACE_OS::exit(-1);
    }
#undef ADD_STMT

void 
ProxyDB::get_account(Proxy_Sock_Ptr sock)
{
  if(sock.null())
    return;
  ACE_Future<SQL::ResultSet*> res;
  res.attach(new Proxyd::getAcctObsv<Proxy_Socket>
	     (Callback<Proxy_Socket, bool>
	      (sock, &Proxy_Socket::account_retrieved)
	      ));
  SqlOperationRequest* op = new SqlOperationRequest(PROXYD_DB_GET_ACCT, res);
  op->add_string(1, sock->get_login().c_str());
  this->enqueue(op);
  
}

};
};
