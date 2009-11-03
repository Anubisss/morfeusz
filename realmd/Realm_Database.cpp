#include "Realm_Database.h"
#include "Realm_Socket.h"
#include "Realm_Service.h"

namespace Trinity
{

namespace DatabaseAccess
{

  RealmDatabaseConnection::RealmDatabaseConnection(ACE_Activation_Queue* q) 
    : DatabaseConnection::DatabaseConnection(q)
  {} 

  RealmDatabaseConnection::~RealmDatabaseConnection()
  {
    if(worker)
      delete worker;
  }

#define ADD_STMT(x, y) this->statement_holder[x] = this->connection->prepareStatement(y)

  bool
  RealmDatabaseConnection::open(const std::string& driver, const std::string& url)
  {
    try
      {
	this->driver = Trinity::SQL::DriverManager::getDriver(driver);
	this->connection = this->driver->connect(url);

	this->statement_holder.resize(REALMD_DB_STMT_MAX);

	ADD_STMT(REALMD_DB_PRUNE_BANS, "DELETE FROM ip_banned WHERE unbandate<=UNIX_TIMESTAMP() AND unbandate<>bandate");
	ADD_STMT(REALMD_DB_CHECK_IP_BAN, "SELECT * FROM ip_banned WHERE ip = ?");
	ADD_STMT(REALMD_DB_CHECK_ACCT_BAN, "SELECT UPPER(a.sha_pass_hash) , a.id, a.locked, a.last_ip, a.gmlevel, ab.active, a.failed_logins FROM account AS a LEFT OUTER JOIN account_banned AS ab ON ab.id = a.id WHERE a.username = ?");
	ADD_STMT(REALMD_DB_SET_S_V, "UPDATE account SET v = ?, s = ? WHERE username = ?");
	ADD_STMT(REALMD_DB_UPDATE_ACCOUNT, "UPDATE account SET sessionkey = ?, last_ip = ?, last_login = NOW(), locale = ?, failed_logins = 0 WHERE username = ?");
	ADD_STMT(REALMD_DB_GET_REALMLIST, "SELECT id, name, address, port, icon, color, timezone, allowedSecurityLevel, population, gamebuild FROM realmlist WHERE color <>3 ORDER BY id");
	ADD_STMT(REALMD_DB_GET_NUMCHAR, "SELECT realmid, numchars FROM realmcharacters WHERE acctid = ?");
	ADD_STMT(REALMD_DB_UPDATE_ACCOUNT,"UPDATE account SET sessionkey = ?, last_ip = ?, last_login = NOW(), locale = ?, failed_logins = 0 WHERE username = ?");
	ADD_STMT(REALMD_DB_FIX_SV, "UPDATE account set s ='', v ='' where username = ?");
	ADD_STMT(REALMD_DB_SET_FAILED_LOGINS, "UPDATE account set failed_logins = failed_logins + 1 where id = ?");
	ADD_STMT(REALMD_DB_ADD_IP_AUTOBAN, "INSERT INTO ip_banned VALUES (?,UNIX_TIMESTAMP(),UNIX_TIMESTAMP()+ ?,'Trinity realmd','Failed login autoban')");
	ADD_STMT(REALMD_DB_ADD_ACCOUNT_AUTOBAN, "INSERT INTO account_banned VALUES (? ,UNIX_TIMESTAMP(),UNIX_TIMESTAMP()+ ?,'Trinity realmd','Failed login autoban',1)");

	this->worker = new DatabaseWorker(this->query_queue, this);
      }
    catch (SQL::SQLException& e)
      {
	ACE_ERROR((LM_ERROR,"Error when setting up connection: %s\n",e.what() ));
	ACE_OS::exit(-1);
      }
    return true;
  }
#undef ADD_STMT


void
RealmDB::ban_failed_logins(uint64 id)
{
  SqlOperationRequest* op = new SqlOperationRequest(REALMD_DB_ADD_ACCOUNT_AUTOBAN);
  op->add_uint64(1, id);
  op->add_uint32(2, 600);
  this->enqueue(op);
}

void
RealmDB::ban_failed_logins(const std::string &ip)
{
  SqlOperationRequest* op = new SqlOperationRequest(REALMD_DB_ADD_IP_AUTOBAN);
  op->add_string(1, ip.c_str());
  op->add_uint32(2, 600);
  this->enqueue(op);

}

void
RealmDB::increment_failed_logins(uint64 id)
{
  SqlOperationRequest* op = new SqlOperationRequest(REALMD_DB_SET_FAILED_LOGINS);
  op->add_uint64(1, id);
  this->enqueue(op);
}

void
RealmDB::check_ip_ban(Realm_Sock_Ptr conn)
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

void
RealmDB::get_account(Realm_Sock_Ptr conn)
{
  if(conn.null())
    return;
  ACE_Future<SQL::ResultSet*> result;
  SqlOperationRequest* op = new SqlOperationRequest(REALMD_DB_CHECK_ACCT_BAN, result);
  result.attach(new checkAcctObsv<Realm_Socket>
		(Callback<Realm_Socket, AccountState>
		 (conn, &Realm_Socket::account_checked)
		 ));
  op->add_string(1, conn->get_login().c_str() );
  this->enqueue(op);
}

void
RealmDB::get_char_amount(Realm_Sock_Ptr conn)
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

void
RealmDB::get_realmlist()
{
  ACE_Future<SQL::ResultSet*> res;
  SqlOperationRequest* op = new SqlOperationRequest(REALMD_DB_GET_REALMLIST,res);
  res.attach(new getRealmListObsv<Realm_Service>
	     (Callback<Realm_Service, SQL::ResultSet*>
	      (sRealm, &Realm_Service::update_realms)
	      ));
  this->enqueue(op);
}


}

}

