#ifndef _0_UBUNTU_H
#define _0_UBUNTU_H

#include "0_all.h"

#if MACHINE == UBUNTU
void R_WDT_Restart(void);
pfdl_status_t R_FDL_Erase(pfdl_u16 blockno);
#endif

#endif
