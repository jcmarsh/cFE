/******************************************************************************
** File:  cfe_psp_memory.c
**
**   Copyright (c) 2004-2006, United States government as represented by the
**   administrator of the National Aeronautics Space Administration.
**   All rights reserved. This software(cFE) was created at NASA Goddard
**   Space Flight Center pursuant to government contracts.
**
**   This software may be used only pursuant to a United States government
**   sponsored project and the United States government may not be charged
**   for use thereof.
**
** Purpose:
**   cFE PSP Memory related functions. This is the implementation of the cFE
**   memory areas that have to be preserved, and the API that is designed to
**   allow acccess to them. It also contains memory related routines to return
**   the address of the kernel code used in the cFE checksum.
**
** History:
**   2007/09/23  A. Cudmore      | m5235bcc Coldfire RTEMS version
**
******************************************************************************/

/*
**  Include Files
*/

#include <string.h>

/*
** cFE includes
*/
#include <common_types.h>
#include <osapi.h>
#include <cfe_es.h>            /* For reset types */

/*
** Types and prototypes for this module
*/
#include <cfe_psp.h>
#include "cfe_psp_memory.h"

/*
** Global variables
*/

CFE_PSP_ReservedMemory_t *CFE_PSP_ReservedMemoryPtr;

/*
*******************************************************************************
** CDS related functions
*******************************************************************************
*/

/******************************************************************************
**  Function: CFE_PSP_GetCDSSize
**
**  Purpose:
**    This function fetches the size of the OS Critical Data Store area.
*/
int32 CFE_PSP_GetCDSSize(uint32 *SizeOfCDS)
{
  if (SizeOfCDS == NULL)
    return CFE_PSP_ERROR;

  *SizeOfCDS = sizeof(CFE_PSP_ReservedMemoryPtr->CDSMemory);

  return CFE_PSP_SUCCESS;
}

/******************************************************************************
**  Function: CFE_PSP_WriteToCDS
**
**  Purpose:
**    This function writes to the CDS Block.
*/
int32 CFE_PSP_WriteToCDS(void *PtrToDataToWrite, uint32 CDSOffset,
                         uint32 NumBytes)
{
  if (PtrToDataToWrite == NULL ||
      CDSOffset + NumBytes > sizeof(CFE_PSP_ReservedMemoryPtr->CDSMemory))
    return CFE_PSP_ERROR;

  memcpy(&CFE_PSP_ReservedMemoryPtr->CDSMemory[CDSOffset], PtrToDataToWrite,
         NumBytes);

  return CFE_PSP_SUCCESS;
}

/******************************************************************************
**  Function: CFE_PSP_ReadFromCDS
**
**  Purpose:
**   This function reads from the CDS Block
*/
int32 CFE_PSP_ReadFromCDS(void *PtrToReadDataTo, uint32 CDSOffset,
                          uint32 NumBytes)
{
  if (PtrToReadDataTo == NULL ||
      CDSOffset + NumBytes > sizeof(CFE_PSP_ReservedMemoryPtr->CDSMemory))
    return CFE_PSP_ERROR;

  memcpy(PtrToReadDataTo, &CFE_PSP_ReservedMemoryPtr->CDSMemory[CDSOffset],
         NumBytes);

  return CFE_PSP_SUCCESS;
}

/*
*******************************************************************************
** ES Reset Area related functions
*******************************************************************************
*/

/******************************************************************************
**  Function: CFE_PSP_GetResetArea
**
**  Purpose:
**    This function returns the location and size of the ES Reset information
**    area.  This area is preserved during a processor reset and is used to
**    store the ER Log, System Log and reset related variables
*/
int32 CFE_PSP_GetResetArea(cpuaddr *PtrToResetArea, uint32 *SizeOfResetArea)
{
  if (SizeOfResetArea == NULL || PtrToResetArea == NULL)
    return CFE_PSP_ERROR;

  *(void **)PtrToResetArea = CFE_PSP_ReservedMemoryPtr->ResetMemory;
  *SizeOfResetArea = sizeof(CFE_PSP_ReservedMemoryPtr->ResetMemory);

  return CFE_PSP_SUCCESS;
}

/*
*******************************************************************************
** ES User Reserved Area related functions
*******************************************************************************
*/

/******************************************************************************
**  Function: CFE_PSP_GetUserReservedArea
**
**  Purpose:
**    This function returns the location and size of the memory used for the
**    cFE User reserved area.
*/
int32 CFE_PSP_GetUserReservedArea(cpuaddr *PtrToUserArea, uint32 *SizeOfUserArea)
{
  if (SizeOfUserArea == NULL || PtrToUserArea == NULL)
    return CFE_PSP_ERROR;

  *(void **)PtrToUserArea = CFE_PSP_ReservedMemoryPtr->UserReservedMemory;
  *SizeOfUserArea = sizeof(CFE_PSP_ReservedMemoryPtr->UserReservedMemory);

  return CFE_PSP_SUCCESS;
}

/*
*******************************************************************************
** ES Volatile disk memory related functions
*******************************************************************************
*/

/******************************************************************************
**  Function: CFE_PSP_GetVolatileDiskMem
**
**  Purpose:
**    This function returns the location and size of the memory used for the
**    cFE volatile disk.
*/
int32 CFE_PSP_GetVolatileDiskMem(cpuaddr *PtrToVolDisk, uint32 *SizeOfVolDisk)
{
  if (SizeOfVolDisk == NULL || PtrToVolDisk == NULL)
    return CFE_PSP_ERROR;

  *(void **)PtrToVolDisk = CFE_PSP_ReservedMemoryPtr->VolatileDiskMemory;
  *SizeOfVolDisk = sizeof(CFE_PSP_ReservedMemoryPtr->VolatileDiskMemory);

  return CFE_PSP_SUCCESS;
}

/*
*******************************************************************************
** ES BSP Top Level Reserved memory initialization
*******************************************************************************
*/

/******************************************************************************
**  Function: CFE_PSP_InitProcessorReservedMemory
**
**  Purpose:
**    This function performs the top level reserved memory initialization.
*/
int32 CFE_PSP_InitProcessorReservedMemory(uint32 RestartType)
{
   if (RestartType != CFE_ES_PROCESSOR_RESET)
   {
     OS_printf("CFE_PSP: Clearing Processor Reserved Memory.\n");
     memset(CFE_PSP_ReservedMemoryPtr, 0, sizeof(*CFE_PSP_ReservedMemoryPtr));

     /*
      ** Set the default reset type in case a watchdog reset occurs
      */
     CFE_PSP_ReservedMemoryPtr->bsp_reset_type = CFE_ES_PROCESSOR_RESET;
   }

   return CFE_PSP_SUCCESS;
}

/*
*******************************************************************************
** ES BSP kernel memory segment functions
*******************************************************************************
*/

/******************************************************************************
**  Function: CFE_PSP_GetKernelTextSegmentInfo
**
**  Purpose:
**    This function returns the start and end address of the kernel text
**    segment. It may not be implemented on all architectures.
*/
int32 CFE_PSP_GetKernelTextSegmentInfo(cpuaddr *PtrToKernelSegment,
                                       uint32 *SizeOfKernelSegment)
{
  /* Unimplemented */
  (void) PtrToKernelSegment;
  (void) SizeOfKernelSegment;

  return CFE_PSP_ERROR;
}

/******************************************************************************
**  Function: CFE_PSP_GetCFETextSegmentInfo
**
**  Purpose:
**    This function returns the start and end address of the CFE text segment.
**    It may not be implemented on all architectures.
*/
int32 CFE_PSP_GetCFETextSegmentInfo(cpuaddr *PtrToCFESegment,
                                    uint32 *SizeOfCFESegment)
{
  /* Unimplemented */
  (void) PtrToCFESegment;
  (void) SizeOfCFESegment;

  return CFE_PSP_ERROR;
}
