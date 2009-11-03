#ifndef sRealm
#include <map>
#include <ace/Singleton.h>
#include <ace/Acceptor.h>
#include <ace/SOCK_Acceptor.h>
#include "Realm_Database.h"

class ACE_Reactor;
class Realm_Socket;

using namespace Trinity::DatabaseAccess;

namespace Trinity
{

typedef ACE_Acceptor<Realm_Socket, ACE_SOCK_ACCEPTOR> RealmdAcceptor;

struct Realm
{
  std::string name;
  std::string address;
  uint8 icon;
  uint8 color;
  uint8 timezone;
  uint8 allowedSecurityLevel;
  float population;
  uint16 build;
};

class Realm_Service : public ACE_Task_Base
{
  friend class ACE_Singleton<Realm_Service, ACE_Recursive_Thread_Mutex>;
 public:
  void start();
  void stop();
  static Realm_Service* instance(){return ACE_Singleton<Realm_Service, ACE_Recursive_Thread_Mutex>::instance();}
  ACE_Reactor* get_reactor(){return reactor;}
  int svc();
  RealmDB* get_db(){return database;}
  std::map<uint8, Realm>* get_realmlist(){return &realm_map;}
  void update_realms(Trinity::SQL::ResultSet*);
 private:
  std::map<uint8, Realm> realm_map;
  Realm_Service(){}
  ACE_Reactor* reactor;
  RealmdAcceptor* acceptor;
  RealmDB* database;
  bool is_running;
};

}
#define REALM_PREFIX ACE_TEXT("REALMD: ")
#define REALM_LOG(...) ACE_DEBUG((LM_INFO, REALM_PREFIX __VA_ARGS__))
//#define REALM_TRACE ACE_DEBUG((LM_DEBUG,"%s\n", __PRETTY_FUNCTION__))
#define REALM_TRACE NULL
#define sRealm Trinity::Realm_Service::instance()
#else
#endif
