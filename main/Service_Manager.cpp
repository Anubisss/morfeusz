#include "Service_Manager.h"
#include <ace/Process_Manager.h>

bool
Service_Manager::run_realmd()
{

  ACE_ARGV realmd_args;
  realmd_args.add(this->args.argv()[0]);
  realmd_args.add("runrealmd");
  
  ACE_Process_Manager* pmgr = ACE_Process_Manager::instance();
  ACE_Process_Options pop;
  pop.command_line(realmd_args.argv());
  pid_t realmpid = pmgr->spawn(pop);
  if(realmpid == ACE_INVALID_PID)
    {
      return false;
    }
  ServiceInfo *si = new ServiceInfo(realmpid);
  this->svcs.insert(std::pair<TrinityServices, ServiceInfo*>(LOGINSERVER, si));
  ACE_DEBUG((LM_DEBUG,"Realmd runs at pid %u\n", realmpid));
  return true;
  
}

void 
Service_Manager::update_services()
{

  

}
