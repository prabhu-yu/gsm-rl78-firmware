#include "0_ubuntu.h"

#if MACHINE == UBUNTU
void R_WDT_Restart(void)
{    
    return;
}
pfdl_status_t R_FDL_Erase(pfdl_u16 blockno) { }
#endif
