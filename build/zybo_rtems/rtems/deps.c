/*
** deps.c 
** 
** This file is used to make sure that functions that are needed by loadable applications get included
** with the kernel image. Normally an RTEMS app only links in the functions that are being referenced.
** In this case, the RTEMS kernel is being built as a separate image from the applications. The applications
** need to call functions that otherwise would not be linked in. This file creates a reference to those 
** functions to be sure that they are included in the RTEMS kernel image. 
**
**
** If you have separately loadable application that links to this kernel image and it has undefined external 
** references, you must put a reference to that function/variable in here. 
*/

#include <rtems.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <dirent.h>

#include <semaphore.h>
#include <pthread.h>
#include <mqueue.h>
#include <dlfcn.h>
#include <sched.h>

extern int __gesf2;
extern int __gtsf2;
extern int __lesf2;
extern int __ltsf2;
extern int __divsf3;
extern int __fixunssfsi;

/*
** Declare the reference to the functions to pull them in
*/
int sleep_ptr = (int)&sleep;
int atol_ptr = (int)&atol;
int cos_ptr = (int)&cos;
int acos_ptr = (int)&acos;
int sin_ptr = (int)&sin;
int asin_ptr = (int)&asin;
int atan_ptr = (int)&atan;
int atan2_ptr = (int)&atan2;
int log_ptr = (int)&log;
int exp_ptr = (int)&exp;
int fmod_ptr = (int)&fmod;
int sqrt_ptr = (int)&sqrt;
int rewinddir_ptr = (int)&rewinddir;


int sinf_ptr = (int)&sinf;
int cosf_ptr = (int)&cosf;
int tanf_ptr = (int)&tanf;
int asinf_ptr = (int)&asinf;
int acosf_ptr = (int)&acosf;
int atanf_ptr = (int)&atanf;
int atan2f_ptr = (int)&atan2f;
int expf_ptr = (int)&expf;
int logf_ptr = (int)&logf;
int powf_ptr = (int)&powf;
int sqrtf_ptr = (int)&sqrtf;
int floorf_ptr = (int)&floorf;
int ceilf_ptr = (int)&ceilf;
int fabsf_ptr = (int)&fabsf;
int fmodf_ptr = (int)&fmodf;

int __gesf2_ptr = (int)&__gesf2;
int __gtsf2_ptr = (int)&__gtsf2;
int __lesf2_ptr = (int)&__lesf2;
int __ltsf2_ptr = (int)&__ltsf2;
int __divsf3_ptr = (int)&__divsf3;
int __fixunssfsi_ptr = (int)&__fixunssfsi;

int scanf_ptr = (int)&scanf;
int clock_gettime_ptr = (int)&clock_gettime;

int rtems_timer_reset_ptr = (int)&rtems_timer_reset;
int rtems_timer_fire_after_ptr = (int)&rtems_timer_fire_after;
int rtems_timer_create_ptr = (int)&rtems_timer_create;
int rtems_task_set_priority_ptr = (int)&rtems_task_set_priority;
int rtems_task_variable_add_ptr = (int)&rtems_task_variable_add;
int rtems_message_queue_create_ptr = (int)&rtems_message_queue_create;
int rtems_message_queue_delete_ptr = (int)&rtems_message_queue_delete;
int rtems_message_queue_receive_ptr = (int)&rtems_message_queue_receive;
int rtems_message_queue_send_ptr = (int)&rtems_message_queue_send;
int rtems_semaphore_flush_ptr = (int)&rtems_semaphore_flush;
int sysconf_ptr = (int)&sysconf;
int vsnprintf_ptr = (int)&vsnprintf;
int vsprintf_ptr = (int)&vsprintf;
int remove_ptr = (int)&remove;
int system_ptr = (int)&system;
int rtems_timer_cancel_ptr = (int)&rtems_timer_cancel;
int rtems_timer_delete_ptr = (int)&rtems_timer_delete;
int strstr_ptr = (int)&strstr;
int bzero_ptr = (int)&bzero;
int rtems_io_register_driver_ptr = (int)&rtems_io_register_driver;

// Is this really needed here?
int rtems_timer_server_fire_after_ptr = (int)&rtems_timer_server_fire_after;
int rtems_timer_initiate_server_ptr = (int)&rtems_timer_initiate_server;

int sem_init_ptr = (int)&sem_init;
int sem_trywait_ptr = (int)&sem_trywait;
int sem_destroy_ptr = (int)&sem_destroy;
int sem_getvalue_ptr = (int)&sem_getvalue;
int sem_post_ptr = (int)&sem_post;

int sigfillset_ptr = (int)&sigfillset;
int sigdelset_ptr = (int)&sigdelset;
int sigsuspend_ptr = (int)&sigsuspend;

int sched_get_priority_max_ptr = (int)&sched_get_priority_max;
int sched_get_priority_min_ptr = (int)&sched_get_priority_min;

int pthread_mutexattr_settype_ptr = (int)&pthread_mutexattr_settype;
int pthread_getschedparam_ptr = (int)&pthread_getschedparam;
int pthread_setschedparam_ptr = (int)&pthread_setschedparam;
int pthread_attr_init_ptr = (int)&pthread_attr_init;
int pthread_attr_setinheritsched_ptr = (int)&pthread_attr_setinheritsched;
int pthread_attr_setstacksize_ptr = (int)&pthread_attr_setstacksize;
int pthread_attr_setschedpolicy_ptr = (int)&pthread_attr_setschedpolicy;
int pthread_attr_setschedparam_ptr = (int)&pthread_attr_setschedparam;
int pthread_create_ptr = (int)&pthread_create;
int pthread_detach_ptr = (int)&pthread_detach;
int pthread_attr_destroy_ptr = (int)&pthread_attr_destroy;
int pthread_cancel_ptr = (int)&pthread_cancel;
int pthread_cond_signal_ptr = (int)&pthread_cond_signal;
int pthread_cond_timedwait_ptr = (int)&pthread_cond_timedwait;
int pthread_equal_ptr = (int)&pthread_equal;

int dlerror_ptr = (int)&dlerror;
int dlsym_ptr = (int)&dlsym;
int dlopen_ptr = (int)&dlopen;
int dlinfo_ptr = (int)&dlinfo;
int dlclose_ptr = (int)&dlclose;

int mq_open_ptr = (int)&mq_open;
int mq_close_ptr = (int)&mq_close;
int mq_unlink_ptr = (int)&mq_unlink;
int mq_receive_ptr = (int)&mq_receive;
int mq_getattr_ptr = (int)&mq_getattr;
int mq_timedreceive_ptr = (int)&mq_timedreceive;
int mq_send_ptr = (int)&mq_send;

int timer_create_ptr = (int)&timer_create;
int timer_settime_ptr = (int)&timer_settime;
int timer_delete_ptr = (int)&timer_delete;
int clock_getres_ptr = (int)&clock_getres;

//int _ptr = (int)&;
