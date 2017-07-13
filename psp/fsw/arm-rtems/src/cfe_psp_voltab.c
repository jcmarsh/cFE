/*
** File   : cfe_psp_voltab.c
** Author : Nicholas Yanchik / GSFC Code 582
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
**
** OS Volume table for file systems
*/

/*
** Includes
*/
#include "common_types.h"
#include "osapi.h"

/*
**  Volume table. This table has the OS_ prefix, since it belongs to the OSAL,
**  not the cFE PSP.
*/
OS_VolumeInfo_t OS_VolumeTable[NUM_TABLE_ENTRIES] =
{
/* Dev Name  Phys Dev  Vol Type        Volatile?  Free?     IsMounted? Volname  MountPt BlockSz */
/* cFE RAM Disk */
{ "/ramdev0", "/dev/rd1",  RAM_DISK,     TRUE,      TRUE,     FALSE,     " ",      " ",     512      },
/* cFE non-volatile Disk -- Auto-Mapped to an existing CF disk */
{"/cf0",     "/cf",       FS_BASED,      TRUE,     FALSE,      TRUE,     "/",    "/cf",     0        },
{"unused",   "unused",    FS_BASED,      TRUE,      TRUE,     FALSE,     " ",      " ",     0        },
{"unused",   "unused",    FS_BASED,      TRUE,      TRUE,     FALSE,     " ",      " ",     0        },
{"unused",   "unused",    FS_BASED,      TRUE,      TRUE,     FALSE,     " ",      " ",     0        },
{"unused",   "unused",    FS_BASED,      TRUE,      TRUE,     FALSE,     " ",      " ",     0        },
{"unused",   "unused",    FS_BASED,      TRUE,      TRUE,     FALSE,     " ",      " ",     0        },
{"unused",   "unused",    FS_BASED,      TRUE,      TRUE,     FALSE,     " ",      " ",     0        },
{"unused",   "unused",    FS_BASED,      TRUE,      TRUE,     FALSE,     " ",      " ",     0        },
{"unused",   "unused",    FS_BASED,      TRUE,      TRUE,     FALSE,     " ",      " ",     0        },
{"unused",   "unused",    FS_BASED,      TRUE,      TRUE,     FALSE,     " ",      " ",     0        },
{"unused",   "unused",    FS_BASED,      TRUE,      TRUE,     FALSE,     " ",      " ",     0        },
{"unused",   "unused",    FS_BASED,      TRUE,      TRUE,     FALSE,     " ",      " ",     0        },
{"unused",   "unused",    FS_BASED,      TRUE,      TRUE,     FALSE,     " ",      " ",     0        }
};
