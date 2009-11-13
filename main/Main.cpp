#include <ace/OS.h>
#include <ace/Arg_Shifter.h>
#include <ace/ARGV.h>
#include <ace/Process_Manager.h>
#include <ace/Time_Value.h>

#include "Service_Manager.h"
#include "Realm_Service.h"
#include "Proxy_Service.h"

#if PLATFORM == PLATFORM_WIN32
extern "C"
{
    #ifdef _DEBUG
        #pragma  comment (lib,"ACEd.lib")
    #else
        #pragma  comment (lib,"ACE.lib")
    #endif
}
#endif

int
ACE_TMAIN(int argc, char* argv[])
{
  if(argc < 2)
    return -1;

  ACE_ARGV args(argc, argv);

  if(ACE_OS::strcmp(args.argv()[1], "runrealmd") == 0)
    {
      sRealm->start();
      return 0;
    }
  else if (ACE_OS::strcmp(args.argv()[1], "rungamed") == 0)
    {
      sProxy->start();
      return 0;
    }

  //If we got to this point, it means we are not in slave mode. Yay!
  sServiceManager->args.add(args.argv());
  ACE_Arg_Shifter shifty(argc, argv); // This class ruins our argv, but we have copy in args.

  shifty.consume_arg();



  while(shifty.is_anything_left() )
    {
      
      if(ACE_OS::strcmp(shifty.get_current(), "-realm") == 0)
	{
	  sServiceManager->run_realmd();
	}
      else 
	if(ACE_OS::strcmp(shifty.get_current(), "-proxy") == 0)
	  {
	    sServiceManager->run_proxyd();
	  }
      
      shifty.consume_arg();
    }
  ACE_Time_Value time;
  time.sec(1);
  while(1)
    {
      ACE_Process_Manager::instance()->wait(time); // for now.
      sServiceManager->update_services();
    }
}
