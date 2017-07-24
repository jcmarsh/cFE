/******************************************************************************
** File:  cfe_psp_start.c
**
**
**      Copyright (c) 2004-2006, United States government as represented by the
**      administrator of the National Aeronautics Space Administration.
**      All rights reserved. This software(cFE) was created at NASAs Goddard
**      Space Flight Center pursuant to government contracts.
**
**      This software may be used only pursuant to a United States government
**      sponsored project and the United States government may not be charged
**      for use thereof.
**
**
** Purpose:
**   cFE BSP main entry point.
**
** History:
**   2004/09/23  J.P. Swinski    | Initial version,
**   2004/10/01  P.Kutt          | Replaced OS API task delay with VxWorks functions
**                                 since OS API is initialized later.
**
******************************************************************************/
#define _USING_RTEMS_INCLUDES_

/*
**  Include Files
*/
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <dlfcn.h>

/*
** RTEMS includes
*/
#include <rtems.h>
#include <rtems/untar.h>
#include <rtems/shell.h>
#include <bsp.h>

/*
** cFE includes
*/
#include <common_types.h>
#include <osapi.h>
#include <cfe_es.h>            /* For reset types */
#include <cfe_platform_cfg.h>  /* for processor ID */
#include <cfe_psp.h>
#include "cfe_psp_memory.h"

/*
** Enable watchdog. Uncomment only if the system has the FPGA core available,
** otherwise they system may crash attempting to access its address space.
*/
/*#define CFE_PSP_WATCHDOG_ENABLE*/

/*
**  External Declarations
*/
extern void CFE_TIME_Local1HzISR(void);


/*
** Global variables
*/
const char CFE_PSP_TIMER_NAME[] = "CFE_PSP_TIMER";
uint32     cfe_psp_timer_id;


/*
** Forward declarations
*/
void CFE_PSP_1HzTimerSet(rtems_id id);


/*
** Function definitions
*/

/*
** 1 HZ Timer "ISR"
**
** Use the RTEMS timer server since CFE_TIME_Local1HzISR uses a pthread mutex
** function that aborts with normal RTEMS timers. Additionally, OSAL timers
** spawned by this thread don't appear to work if this thread exits.
*/
rtems_timer_service_routine CFE_PSP_1HzTimerRoutine(rtems_id id, void *arg)
{
  (void) arg;

  CFE_TIME_Local1HzISR();
  CFE_PSP_1HzTimerSet(id);
}

void CFE_PSP_1HzTimerSet(rtems_id id)
{
  rtems_timer_server_fire_after(
    id,
    rtems_clock_get_ticks_per_second(),
    CFE_PSP_1HzTimerRoutine,
    NULL
  );
}

static void start_shell(void)
{
  rtems_status_code sc;

  sc = rtems_shell_init("SHLL", RTEMS_MINIMUM_STACK_SIZE * 4, 250,
    "/dev/console", false, false, NULL);
  if (sc != RTEMS_SUCCESSFUL)
    printf("PSP: rtems_shell_init failed: %d\n", sc);
  else
    printf("PSP: Shell started\n");
}

/*
** A simple entry point to start from the loader
*/
void *POSIX_Init(void *arg)
{
   (void) arg;

   CFE_PSP_Main(1, "/cf/cfe_es_startup.scr");

   /*
   ** Return to the shell/monitor
   */
   return NULL;
}


/******************************************************************************
**  Function:  CFE_PSP_Main()
**
**  Purpose:
**    Application entry point.
**
**  Arguments:
**    (none)
**
**  Return:
**    (none)
*/
void CFE_PSP_Main(uint32 ModeId, char *StartupFilePath)
{
   int32             os_status;
   uint32            reset_type;
   uint32            reset_subtype;
   rtems_status_code RtemsStatus;
   rtems_id          timer_id;

   (void) ModeId;

   /*
   ** Initialize the OS API
   */
   os_status = OS_API_Init();
   if (os_status != OS_SUCCESS)
     printf("CFE_PSP: OS_API_Init() failed: %ld\n", os_status);
   /* FIXME: Isn't this fatal? */

   /*
   ** Allocate memory for the cFE memory. Note that this is malloced on
   ** the COTS board, but will be a static location in the ETU.
   */
   printf("Sizeof BSP reserved memory = %d bytes\n",
          sizeof(CFE_PSP_ReservedMemory_t));
   CFE_PSP_ReservedMemoryPtr = malloc(sizeof(CFE_PSP_ReservedMemory_t));
   if ( CFE_PSP_ReservedMemoryPtr == NULL )
   {
      printf("CFE_PSP: Error: Cannot malloc BSP reserved memory!\n");
   }
   else
   {
      printf("CFE_PSP: Allocated %d bytes for PSP reserved memory at: 0x%08X\n",
             sizeof(CFE_PSP_ReservedMemory_t),
             (int)CFE_PSP_ReservedMemoryPtr);
   }
   /*
   ** Determine Reset type by reading the hardware reset register.
   */
   reset_type = CFE_ES_POWERON_RESET;
   reset_subtype = CFE_ES_POWER_CYCLE;

   /*
   ** Initialize the reserved memory
   */
   CFE_PSP_InitProcessorReservedMemory(reset_type);

   /*
   ** Initialize watchdog
   */
#ifdef CFE_PSP_WATCHDOG_ENABLE
   CFE_PSP_WatchdogInit();
#endif /* CFE_PSP_WATCHDOG_ENABLE */

   /*
   ** Call cFE entry point. This will return when cFE startup
   ** is complete.
   */
   CFE_ES_Main(reset_type,reset_subtype, 1, (uint8 *)StartupFilePath);

   /*
   ** Setup the timer to fire at 1hz
   */
   RtemsStatus = rtems_timer_initiate_server(
     RTEMS_MINIMUM_PRIORITY, /* only handles the high-priority 1Hz timer */
     RTEMS_MINIMUM_STACK_SIZE * 2,
     RTEMS_DEFAULT_ATTRIBUTES
   );
   if (RTEMS_SUCCESSFUL != RtemsStatus)
   {
     OS_printf("CFE_PSP: RTEMS timer server start failed: %d\n", RtemsStatus);
   }
   else
   {
     RtemsStatus = rtems_timer_create(rtems_build_name('1', 'H', 'z', ' '),
       &timer_id);
     if (RTEMS_SUCCESSFUL != RtemsStatus)
     {
       OS_printf("CFE_PSP: Interval timer creation failed: %d\n", RtemsStatus);
     }
     else
     {
       CFE_PSP_1HzTimerSet(timer_id);
       OS_printf("CFE_PSP: Interval timer started\n");
     }
   }

   start_shell();
}


//#define CONFIGURE_APPLICATION_NEEDS_CLOCK_DRIVER
//#define CONFIGURE_APPLICATION_NEEDS_CONSOLE_DRIVER

//#define CONFIGURE_LIBIO_MAXIMUM_FILE_DESCRIPTORS     128

//#define CONFIGURE_MAXIMUM_TIMERS 4

//#define CONFIGURE_UNIFIED_WORK_AREAS
//#define CONFIGURE_UNLIMITED_OBJECTS
//#define CONFIGURE_MAXIMUM_POSIX_THREADS              128
//#define CONFIGURE_MAXIMUM_POSIX_CONDITION_VARIABLES  128
//#define CONFIGURE_MAXIMUM_POSIX_MUTEXES              128
//#define CONFIGURE_MAXIMUM_POSIX_SEMAPHORES           128
//#define CONFIGURE_MAXIMUM_POSIX_MESSAGE_QUEUES       512
//#define CONFIGURE_MAXIMUM_POSIX_MESSAGE_QUEUE_DESCRIPTORS 2048
//#define CONFIGURE_MAXIMUM_POSIX_KEYS                 128
//#define CONFIGURE_MAXIMUM_POSIX_TIMERS               16
//#define CONFIGURE_MAXIMUM_POSIX_QUEUED_SIGNALS       128
//#define CONFIGURE_MAXIMUM_POSIX_BARRIERS             128 /* may not need */
//#define CONFIGURE_MAXIMUM_POSIX_SPINLOCKS            128 /* may not need */
//#define CONFIGURE_POSIX_INIT_THREAD_TABLE

//#define CONFIGURE_POSIX_INIT_THREAD_STACK_SIZE 8 * RTEMS_MINIMUM_STACK_SIZE

//#define CONFIGURE_APPLICATION_NEEDS_LIBBLOCK
//#define CONFIGURE_SHELL_COMMANDS_ALL
//#define CONFIGURE_SHELL_COMMANDS_INIT
//#define CONFIGURE_MAXIMUM_TASKS 2 /* timer server, shell */
//#include <rtems/shellconfig.h>

/*#define CONFIGURE_APPLICATION_NEEDS_LIBBLOCK*/
//#define CONFIGURE_MICROSECONDS_PER_TICK 100
/*#define CONFIGURE_MAXIMUM_DRIVERS 16*/

//#define CONFIGURE_INIT
//#include <rtems/confdefs.h>
