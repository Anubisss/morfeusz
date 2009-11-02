#ifndef sServiceManager
#include <ace/Singleton.h>
#include <map>
#include <ace/ARGV.h>

enum TrinityServices
  {
    LOGINSERVER = 0,
    GAMESERVER,
    ZONESERVER,
    SOCIALSERVER,
    SERVICES_MAX
  };

enum ServiceStatus
  {
    OFF = 0,
    ON
  };

struct ServiceInfo
{
ServiceInfo():status(OFF),pid(0) {}
ServiceInfo(pid_t pid):pid(pid), status(ON) {}
  ServiceStatus status;
  pid_t pid;
};

class Service_Manager
{
  friend class ACE_Singleton<Service_Manager, ACE_Thread_Mutex>;
 public:
  ACE_ARGV args;
  bool run_realmd();
  void update_services();
  static Service_Manager* instance(){return ACE_Singleton<Service_Manager, ACE_Thread_Mutex>::instance();}
 private:
  Service_Manager(){};
  std::map<TrinityServices, ServiceInfo*> svcs;
};

#define sServiceManager Service_Manager::instance()
#else
#endif
