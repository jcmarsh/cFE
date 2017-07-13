/******************************************************************************
** File:  cfe_psp_watchdog.c
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
**   This file contains glue routines between the cFE and the OS Board Support
**   Package ( BSP ).  The functions here allow the cFE to interface functions
**   that are board and OS specific and usually dont fit well in the OS
**   abstraction layer.
**
** Notes:
**   Make sure that the MMU is configured to allow access to the memory range
**   of the WDT Handler core.
**
** History:
**   2009/07/20  A. Cudmore    | Initial version,
**   2016/07/12  P. Gauvin     | Support CSP "WDT Driver" core
**
** TODO:
**   Implement and use BSP watchdog driver, rather than direct access
******************************************************************************/

/*
**  Include Files
*/

/*
** cFE includes
*/
#include "common_types.h"
#include "osapi.h"

/*
**  System Include Files
*/
#include <stdbool.h>
#include <semaphore.h>
#include <errno.h>
#include <string.h>

/*
** Types and prototypes for this module
*/
#include "cfe_psp.h"

/*
** Macro definitions
*/

#define CFE_PSP_WATCHDOG_WDT_HANDLER_ADDRESS                 0x70000000
#define CFE_PSP_WATCHDOG_WDT_HANDLER_STROKE_OFFSET           0x0
#define CFE_PSP_WATCHDOG_WDT_HANDLER_PULSE_ASSERT_OFFSET     0x4
#define CFE_PSP_WATCHDOG_WDT_HANDLER_PULSE_DEASSERT_OFFSET   0x8
#define CFE_PSP_WATCHDOG_WDT_HANDLER_PULSE_ASSERT_VAL_OFFSET 0xc

#define CFE_PSP_WATCHDOG_ASSERT_LEN   25000000
#define CFE_PSP_WATCHDOG_DEASSERT_LEN 25000000
#define CFE_PSP_WATCHDOG_ASSERT_VAL   1

#define CFE_PSP_WATCHDOG_OS_TIMER_NAME "CFE_PSP_WDT"

#define CFE_PSP_WATCHDOG_SERVICE_ERR_NONE      0
#define CFE_PSP_WATCHDOG_SERVICE_ERR_UNKNOWN   1
#define CFE_PSP_WATCHDOG_SERVICE_ERR_TIMER_SET 2

/*
** Type definitions
*/

/* WDT Handler register map */
struct OS_PACK Wdt_handler {
  uint32_t pulse;
  uint32_t assert_len;
  uint32_t deassert_len;
  uint32_t assert_val;
};

struct Wdt {
  /* Allow touching of the watchdog */
  bool enabled;
  /* RTEMS Zynq BSP uses volatile pointers for IO access*/
  volatile struct Wdt_handler *wdt_handler;
  /* Software timeout in microseconds */
  uint32 timer_value;
  /* OSAL timer ID */
  uint32 timer_id;
  /*
  ** Used to indicate that re-arming is OK. Instead of using it to lock a
  ** variable, the semaphore itself indicates to the timer callback that it is
  ** OK to re-arm the timer. This prevents the possibility of the variable
  ** being locked mid-modification when the timer callback is called, resulting
  ** in the timer not being re-armed. This also allows prevents the timer
  ** callback from needing to wait for the semaphore to access a variable.
  ** A better alternative is atomic variables, but this is not C11 code.
  */
  sem_t sem_rearm;
};

/*
** Local data
*/

static struct Wdt wdt;
static bool CFE_PSP_Watchdog_initialized = false;
int CFE_PSP_Watchdog_service_last_error = CFE_PSP_WATCHDOG_SERVICE_ERR_NONE;

/*
** Function declarations
*/

static void Wdt_handler_init(volatile struct Wdt_handler *wdt_handler,
  uint32_t assert_len, uint32_t deassert_len, uint32_t assert_val)
{
  wdt_handler->assert_len = assert_len;
  wdt_handler->deassert_len = deassert_len;
  wdt_handler->assert_val = assert_val;
}

static inline void Wdt_handler_service(
  volatile struct Wdt_handler *wdt_handler)
{
  wdt_handler->pulse = 1;
}

void CFE_PSP_WatchdogOSTimer(uint32_t timer_id)
{
  int status;

  if (timer_id != wdt.timer_id)
    return;
  if (!wdt.enabled)
    return;

  Wdt_handler_service(wdt.wdt_handler);
  do /* Retry on EINTR, for all other errno do not set the timer */
  {
    status = sem_trywait(&wdt.sem_rearm);
  } while(status != 0 && errno == EINTR);
  if (0 == status)
  {
    int32 status;

    status = OS_TimerSet(wdt.timer_id, wdt.timer_value, 0);
    if (OS_SUCCESS != status)
      CFE_PSP_Watchdog_service_last_error =
          CFE_PSP_WATCHDOG_SERVICE_ERR_TIMER_SET;
    /*
    ** Do not post, only CFE_PSP_WatchdogTimerService does that; furthermore,
    ** this is the only function that will decrement the semaphore
    */
  }
  /* Should only get EAGAIN which indicates sem is zero */
  else if (errno != EAGAIN)
  {
    CFE_PSP_Watchdog_service_last_error = CFE_PSP_WATCHDOG_SERVICE_ERR_UNKNOWN;
  }
}

/*  Function:  CFE_PSP_WatchdogInit()
**
**  Purpose:
**    To setup the timer resolution and/or other settings custom to this
**    platform.
*/
void CFE_PSP_WatchdogInit(void)
{
  uint32 clock_accuracy;
  int32 status;
  int pstatus;

  wdt.enabled = false;
  wdt.wdt_handler = (struct Wdt_handler *)CFE_PSP_WATCHDOG_WDT_HANDLER_ADDRESS;
  wdt.timer_value = CFE_PSP_WATCHDOG_MAX;

  pstatus = sem_init(&wdt.sem_rearm, 0, 0);
  if (0 != pstatus)
  {
    OS_printf("CFE_PSP: WDT: sem_init failed: %s\n", strerror(errno));
    goto fail;
  }

  Wdt_handler_init(wdt.wdt_handler, CFE_PSP_WATCHDOG_ASSERT_LEN,
    CFE_PSP_WATCHDOG_DEASSERT_LEN, CFE_PSP_WATCHDOG_ASSERT_VAL);

  status = OS_TimerCreate(&wdt.timer_id, CFE_PSP_WATCHDOG_OS_TIMER_NAME,
    &clock_accuracy, CFE_PSP_WatchdogOSTimer);
  if (status != OS_SUCCESS)
  {
    OS_printf("CFE_PSP: WDT: OS timer creation failed\n");
    OS_printf("CFE_PSP: WDT: OS_TimerCreate failed: %ld\n", status);
    goto fail_sem_destroy;
  }
  CFE_PSP_Watchdog_initialized = true;

  /* Success */
  return;

fail_sem_destroy:
  pstatus = sem_destroy(&wdt.sem_rearm);
  if (pstatus != 0)
    OS_printf("CFE_PSP: WDT: sem_destroy failure: %s\n", strerror(errno));
fail:
  return;
}

/******************************************************************************
**  Function:  CFE_PSP_WatchdogEnable()
**
**  Purpose:
**    Enable touching of the watchdog hardware.
*/
void CFE_PSP_WatchdogEnable(void)
{
  int32 status;

  if (!CFE_PSP_Watchdog_initialized)
    return;
  if (wdt.enabled)
    return;

  wdt.enabled = true;
  /* Start the timer for the first period */
  status = OS_TimerSet(wdt.timer_id, wdt.timer_value, 0);
  if (OS_SUCCESS != status)
    OS_printf("CFE_PSP: WDT: OS_TimerSet failed: %ld\n", status);
}

/******************************************************************************
**  Function:  CFE_PSP_WatchdogDisable()
**
**  Purpose:
**    Disable touching of the watchdog hardware.
*/
void CFE_PSP_WatchdogDisable(void)
{
  if (!CFE_PSP_Watchdog_initialized)
    return;

  /* Note that this will eventually cause the dog to bite if it was previously
   * enabled.
   */
  wdt.enabled = false;
}

/******************************************************************************
**  Function:  CFE_PSP_WatchdogService()
**
**  Purpose:
**    Indicates to the watchdog timer callback that it should rearm the timer
**    once it expires. If this is not called before the end of the timeout
**    period (indicative of a software failure), the watchdog will not be
**    touched again, even if CFE_PSP_WatchdogService is called.
*/
void CFE_PSP_WatchdogService(void)
{
  int status;
  int value;

  if (!CFE_PSP_Watchdog_initialized)
    return;
  if (!wdt.enabled)
    return;

  status = sem_getvalue(&wdt.sem_rearm, &value);
  if (status != 0)
  {
    OS_printf("CFE_PSP: WDT: sem_getvalue failed: %s\n", strerror(errno));
  }
  /* Check if this function has not posted */
  else if (value == 0)
  {
    /* Post so the timer service routine will know to rearm */
    status = sem_post(&wdt.sem_rearm);
    if (0 != status) /* Should never happen */
    {
      OS_printf("CFE_PSP: WDT: sem_post failed: %s\n", strerror(errno));
    }
  }
}

/******************************************************************************
**  Function:  CFE_PSP_WatchdogGet
**
**  Purpose:
**    Get the current watchdog software timeout value.
**
**  Return:
**    The current timeout value in microseconds
*/
uint32 CFE_PSP_WatchdogGet(void)
{
  if (!CFE_PSP_Watchdog_initialized)
    return 0;

  return wdt.timer_value;
}

/******************************************************************************
**  Function:  CFE_PSP_WatchdogSet
**
**  Purpose:
**    Set the current watchdog software timeout value.
**
**  Arguments:
**    The new timeout value in microseconds
*/
void CFE_PSP_WatchdogSet(uint32 WatchdogValue)
{
  if (!CFE_PSP_Watchdog_initialized)
    return;

  wdt.timer_value = WatchdogValue;
}
