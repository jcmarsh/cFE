/******************************************************************************
** File:  cfe_psp_ssr.c
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
**   This file contains glue routines between the cFE and the OS Board Support
**   Package ( BSP ).  The functions here allow the cFE to interface functions
**   that are board and OS specific and usually dont fit well in the OS
**   abstraction layer.
**
** History:
**   2005/06/05  Alan Cudmore    | Initial version,
**
******************************************************************************/

/*
**  Include Files
*/
#include <stdio.h>
#include <stdlib.h>

/*
** cFE includes
*/
#include <common_types.h>
#include <osapi.h>

/*
** Types and prototypes for this module
*/
#include <cfe_psp.h>

/******************************************************************************
**  Function:  CFE_PSP_InitSSR
**
**  Purpose:
**    Initializes the Solid State Recorder device. Dummy function for this
**    platform.
**
**  Arguments:
**    bus, device, device name
**
**  Return:
**    CFE_PSP_ERROR on failure, CFE_PSP_SUCCESS on success
*/
int32 CFE_PSP_InitSSR(uint32 bus, uint32 device, char *DeviceName )
{
  /* Not implemented */
  (void) bus;
  (void) device;
  (void) DeviceName;

  return CFE_PSP_ERROR;
}
