#include <ace/Event_Handler.h>

namespace Trinity
{
class Realm_Timer : public ACE_Event_Handler
{
 private:
  int handle_timeout(const ACE_Time_Value &, const void *act)
  {
    
    sRealm->get_db()->get_realmlist();
    return 0;

  }
};
}
