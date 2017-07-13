/******************************************************************************
** File:  cfe_psp_support.c
**
**
**      Copyright (c) 2004-2006, United States government as represented by the
**      administrator of the National Aeronautics Space Administration.
**      All rights reserved. This software(cFE) was created at NASA Goddard
**      Space Flight Center pursuant to government contracts.
**
**      This software may be used only pursuant to a United States government
**      sponsored project and the United States government may not be charged
**      for use thereof.
**
**
** Purpose:
**   This file contains glue routines between the cFE and the OS Board Support Package ( BSP ).
**   The functions here allow the cFE to interface functions that are board and OS specific
**   and usually dont fit well in the OS abstraction layer.
**
** History:
**   2005/06/05  Alan Cudmore    | Initial version,
**
******************************************************************************/

/*
**  Include Files
*/
#include <stdio.h>
#include <unistd.h>
#include <rtems.h>

/*
** cFE includes
*/
#include <osapi.h>
#include <common_types.h>
#include <cfe_es.h>            /* For reset types */
#include <cfe_platform_cfg.h>  /* for processor ID */
#include <cfe_mission_cfg.h>   /* for spacecraft ID */

/*
** Types and prototypes for this module
*/
#include <cfe_psp.h>
#include "cfe_psp_memory.h"

/*
** Macro definitions
*/
#define CFE_PSP_PANIC_EXIT_DELAY_US 100000

/******************************************************************************
**  Function:  CFE_PSP_Restart()
**
**  Purpose:
**    Provides a common interface to the processor reset.
**
**  Arguments:
**    reset_type  : Type of reset.
**
**  Return:
**    (none)
*/

void CFE_PSP_Restart(uint32 reset_type)
{
  if (reset_type == CFE_ES_POWERON_RESET)
  {
    CFE_PSP_ReservedMemoryPtr->bsp_reset_type = CFE_ES_POWERON_RESET;
    CFE_PSP_FlushCaches(1, (uint32)CFE_PSP_ReservedMemoryPtr,
      sizeof(CFE_PSP_ReservedMemory_t));
    /* reboot(BOOT_CLEAR); Need RTEMS equiv. */
  }
  else
  {
    CFE_PSP_ReservedMemoryPtr->bsp_reset_type = CFE_ES_PROCESSOR_RESET;
    CFE_PSP_FlushCaches(1, (uint32)CFE_PSP_ReservedMemoryPtr,
      sizeof(CFE_PSP_ReservedMemory_t));
    /* reboot(BOOT_NORMAL); Need RTEMS Equiv */
  }

  printf("CFE_PSP_Restart is not implemented on this platform ( yet ! )\n");
  CFE_PSP_Panic(CFE_PSP_PANIC_GENERAL_FAILURE);
}

/******************************************************************************
**  Function:  CFE_PSP_Panic()
**
**  Purpose:
**    Provides a common interface to abort the cFE startup process and return
**    back to the OS.
**
**  Arguments:
**    ErrorCode  : Reason for Exiting.
**
**  Return:
**    (none)
*/

void CFE_PSP_Panic(int32 ErrorCode)
{
  printf("CFE_PSP_Panic Called with error code = %d. Exiting.\n",
    (int)ErrorCode);
  fflush(stdout);
  usleep(CFE_PSP_PANIC_EXIT_DELAY_US);
  exit(ErrorCode);
}

/******************************************************************************
**  Function:  CFE_PSP_FlushCaches()
**
**  Purpose:
**    Provides a common interface to flush the processor caches. This routine
**    is in the BSP because it is sometimes implemented in hardware and
**    sometimes taken care of by the RTOS.
*/

void CFE_PSP_FlushCaches(uint32 type, uint32 address, uint32 size)
{
  /* Types are not defined, ignore */
  (void) type;

  /* Paranoid, flush both */
  rtems_cache_flush_multiple_data_lines((void *)address, size);
  rtems_cache_invalidate_multiple_instruction_lines((void *)address, size);
}

/*
** Name: CFE_PSP_GetProcessorId
**
** Purpose:
**         return the processor ID.
**
** Return Values: Processor ID
*/

uint32 CFE_PSP_GetProcessorId(void)
{
  return CFE_CPU_ID;
}

/*
** Name: CFE_PSP_GetSpacecraftId
**
** Purpose:
**         return the spacecraft ID.
**
** Return Values: Spacecraft ID
*/
uint32 CFE_PSP_GetSpacecraftId(void)
{
  return CFE_SPACECRAFT_ID;
}
