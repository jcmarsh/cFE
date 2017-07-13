/******************************************************************************
**
** File:  cfe_psp_exception.c
**
**   Copyright (c) 2004-2006, United States government as represented by the
**   administrator of the National Aeronautics Space Administration.
**   All rights reserved. This software(cFE) was created at NASA's Goddard
**   Space Flight Center pursuant to government contracts.
**
**   This software may be used only pursuant to a United States government
**   sponsored project and the United States government may not be charged
**   for use thereof.
**
**
** Purpose:
**   cFE BSP exception-related functions.
**
** History:
**   2007/09/23  A. Cudmore      | RTEMS Coldfire m5235bcc version
**
******************************************************************************/

/*
**  Include Files
*/

/*
** cFE includes
*/

#include <common_types.h>
#include <osapi.h>

#include <cfe_psp.h>
#include "cfe_psp_memory.h"

/*
** Global variables
*/

CFE_PSP_ExceptionContext_t CFE_PSP_ExceptionContext;

/*
**
** IMPORTED FUNCTIONS
**
*/

/*
** Called by the exception handler, should it be implemented
*/
void CFE_ES_EXCEPTION_FUNCTION(uint32  HostTaskId, uint8 *ReasonString,
                               uint32 *ContextPointer, uint32 ContextSize);

/*
** Function Declarations
*/

/*
**
**   Name: CFE_PSP_AttachExceptions
**
**   Purpose:
**     This function Initializes the task exceptions and adds a hook into the
**     operation system's exception handling system.
*/
void CFE_PSP_AttachExceptions(void)
{
  OS_printf("CFE PSP: Exceptions not handled.\n");
}

/*
**
**   Name: CFE_PSP_SetDefaultExceptionEnvironment
**
**   Purpose:
**     This function sets a default exception environment that can be used
**
**   Notes:
**     The exception environment is local to each task, therefore, this must
**     be called for each task that that wants to do floating point and catch
**     exceptions. Currently, this is automatically called from
**     CFE_ES_RegisterApp and CFE_ES_RegisterChildTask for every task and
**     child.
*/
void CFE_PSP_SetDefaultExceptionEnvironment(void)
{
  /* Unimplemented */
  return;
}
