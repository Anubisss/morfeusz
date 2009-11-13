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

  std::map<TrinityServices, ServiceInfo*>::iterator iter;

  for(iter = svcs.begin();iter != svcs.end(); iter++)
    {
      if (iter->second->status == OFF)
	continue;
      bool is_dead = false;
      char* path = new char[ 6 + 6 + 5];
      ACE_OS::sprintf(path,"/proc/%u/stat", iter->second->pid);
      ACE_HANDLE status_file = ACE_OS::open(path, GENERIC_READ);
      
      if (status_file == ACE_INVALID_HANDLE)
	is_dead = true;
      
      if (!is_dead)
	{
	  char *buf = new char[100]; //please excuse me this waste of bytes.
	  char delim[] = " ";
	  ACE_OS::read(status_file,buf,100);
	  strtok(buf,delim);
	  strtok(NULL,delim);
	  char * status = strtok(NULL, delim);
	  delete buf;
	  if (*status == 'Z')
	    {
	      is_dead = true;
	      ACE_OS::kill(iter->second->pid, SIGKILL);
	    }
	}

      if(is_dead)
	{
	  switch(iter->first)
	    {
	    case LOGINSERVER:
	      this->run_realmd();
	      break;
	    default:
	      break;
	    }
	}

    }

}
