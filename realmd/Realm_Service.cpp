#include "Realm_Service.h"
#include "Realm_Socket.h"
#include "Configuration.h"
#include "Realm_Database.h"
#include "Realm_Timer.h"
#include <sstream>
#include <ace/TP_Reactor.h>
#include <ace/Dev_Poll_Reactor.h>

void
Realm_Service::start()
{

  REALM_LOG("Starting realmd\n");

#if defined (ACE_HAS_EVENT_POLL) || defined (ACE_HAS_DEV_POLL)
  this->reactor = new ACE_Reactor(new ACE_Dev_Poll_Reactor());
  this->reactor->restart(1);
#else
  this->reactor = new ACE_Reactor(new ACE_TP_Reactor(), 1);
#endif

  this->acceptor = new RealmdAcceptor();
  
  if(this->acceptor->open(ACE_INET_Addr(sConfig->getString("realmd", "BindAddr").c_str()), this->reactor) == -1)
    {
      REALM_LOG("Couldn't bind to interface!\n");
      delete reactor;
      delete acceptor;
      return;
    }
  ACE_Time_Value tm(1 * 60 * sConfig->getInt("realmd","UpdateInterval"));
 
  this->database = new RealmDB(sConfig->getInt("realmd", "DBThreads"));
  this->database->open(sConfig->getString("realmd", "DBengine"),sConfig->getString("realmd", "DBUrl") );
  this->database->get_realmlist();
  this->is_running = true;
  this->activate(THR_NEW_LWP | THR_JOINABLE, sConfig->getInt("realmd", "NetThreads"));
  this->reactor->schedule_timer(new Realm_Timer(), 0, tm, tm);
  ACE_Thread_Manager::instance()->wait();
  return;
}

void
Realm_Service::update_realms(Trinity::SQL::ResultSet* res)
{
  REALM_LOG("Updating realms\n");

  if(!res)
    return;
  Realm rlm;
  uint32 id;
  while(res->next())
    {
      std::ostringstream oss;
      oss.clear();
      rlm.name = res->getString(2); 
      oss << res->getString(3) << ":" << res->getUint16(4);
      rlm.address = oss.str();
      rlm.icon = res->getUint8(5);
      rlm.color = res->getUint8(6);
      rlm.timezone = res->getUint8(7);
      rlm.allowedSecurityLevel = res->getUint8(8);
      rlm.population = res->getFloat(9);
      rlm.build = res->getUint16(10);
      id = res->getUint32(1);
      this->realm_map[id] = rlm;
      REALM_LOG("Added realm %s (ID: %u) %f\n",rlm.name.c_str(), res->getUint8(1), rlm.population);
    }

}

int 
Realm_Service::svc()
{
  while(this->is_running)
    this->reactor->handle_events();
  
  return 0;
}

void
Realm_Service::stop()
{
  this->is_running = false;
  this->reactor->end_reactor_event_loop();
  this->database->close();

}
