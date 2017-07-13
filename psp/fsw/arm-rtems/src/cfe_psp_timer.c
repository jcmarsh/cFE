/*
** File:  cfe_psp_timer.c
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
**      This file contains glue routines between the cFE and the OS Board
**      Support Package ( BSP ).  The functions here allow the cFE to interface
**      functions that are board and OS specific and usually dont fit well in
**      the OS abstraction layer.
**
** History:
**   2005/06/05  K.Audra    | Initial version,
**
*/

/*
**  Include Files
*/

#include <time.h>

/*
** cFE includes
*/
#include <common_types.h>
#include <osapi.h>

/*
** Types and prototypes for this module
*/
#include <cfe_psp.h>

/*
** Macro Definitions
*/

/*
** Low 32 tick resolution is 1 us with OSAL
*/
#define CFE_PSP_TIMER_LOW32_RESOLUTION 1000000
/* The number at which the least significant 32 bits of the 64 bit time stamp
** returned by CFE_PSP_Get_Timebase rolls over.  If the lower 32 bits rolls at
** 1 second, then the CFE_PSP_TIMER_LOW32_ROLLOVER will be 1000000.  If the
** lower 32 bits rolls at its maximum value (2^32) then
** CFE_PSP_TIMER_LOW32_ROLLOVER will be 0.
*/
#define CFE_PSP_TIMER_LOW32_ROLLOVER CFE_PSP_TIMER_LOW32_RESOLUTION

/*
** CFE_PSP_Get_Timer_Tick is not implemented due poor specifications, and lack
** of usage in cFE and cFS
*/

/*
** CFE_PSP_Get_Dec is not implemented due to poor specification, and lack of
** usage in cFE and cFS.
*/

/******************************************************************************
**  Function:  CFE_PSP_GetTime()
**
**  Purpose: Gets the value of the time from the hardware
**
**  Arguments: LocalTime - where the time is returned through
******************************************************************************/
void CFE_PSP_GetTime(OS_time_t *LocalTime)
{
  OS_GetLocalTime(LocalTime);
}

/******************************************************************************
**  Function:  CFE_PSP_GetTimerTicksPerSecond()
**
**  Purpose:
**    Provides the resolution of the least significant 32 bits of the 64 bit
**    time stamp returned by CFE_PSP_Get_Timebase in timer ticks per second.
**    The timer resolution for accuracy should not be any slower than 1000000
**    ticks per second or 1 us per tick
**
**  Arguments:
**
**  Return:
**    The number of ticks in the lower 32bits of a time stamp retrieved by
**    CFE_PSP_Get_Timebase which represent one second. Each "tick" is one
**    increment.
*/
uint32 CFE_PSP_GetTimerTicksPerSecond(void)
{
  return CFE_PSP_TIMER_LOW32_RESOLUTION;
}

/******************************************************************************
**  Function:  CFE_PSP_GetTimerLow32Rollover()
**
**  Purpose:
**    Provides the number that the least significant 32 bits of the 64 bit
**    time stamp returned by CFE_PSP_Get_Timebase rolls over.  If the lower 32
**    bits rolls at 1 second, then the CFE_PSP_TIMER_LOW32_ROLLOVER will be 1000000.
**    if the lower 32 bits rolls at its maximum value (2^32) then
**    CFE_PSP_TIMER_LOW32_ROLLOVER will be 0.
**
**  Arguments:
**
**  Return:
**    The number at which the least significant 32 bits of the 64 bit time
**    stamp returned by CFE_PSP_Get_Timebase rolls over.
*/
uint32 CFE_PSP_GetTimerLow32Rollover(void)
{
  return CFE_PSP_TIMER_LOW32_ROLLOVER;
}

/******************************************************************************
**  Function:  CFE_PSP_Get_Timebase()
**
**  Purpose:
**    Provides a common interface to system timebase. This routine
**    is in the BSP because it is sometimes implemented in hardware and
**    sometimes taken care of by the RTOS.
**
**  Arguments:
**    Tbu: Timebase upper value (seconds)
**    Tbl: Timebase lower value (microseconds)
*/
void CFE_PSP_Get_Timebase(uint32 *Tbu, uint32* Tbl)
{
  OS_time_t time;

  OS_GetLocalTime(&time);
  *Tbu = time.seconds;
  *Tbl = time.microsecs;
}
