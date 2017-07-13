/*
** staticloadfile.h
**
** Author: Alan Cudmore
**
**
*/

#ifndef _staticloadfile_
#define _staticloadfile_

#include "common_types.h"
#include "staticload_version.h"

/*
** Defines
*/

#define OBJ_NAME_SIZE 32

/*
** Flags
*/

#define STATIC_FILE_UNCOMPRESSED  1
#define STATIC_FILE_LZMA          2
#define STATIC_FILE_GZIP          3

/* 
** static file identifier "LOADFILE"
*/

#define STATIC_FILE_MARKER    0x10ADF11E

/*
** File Types
*/

/*
** Static load file header -- currently 108 bytes
*/

typedef struct
{
    uint32     file_marker;  /* Unique file type or marker */
    uint32     entry_point;  /* Entry point for Task/Application */
    uint32     flags;        /* Flags - Currently just to indicate compression */
    uint32     code_target;  /* Address in RAM where code is copied */
    uint32     code_size;    /* Size of Code Segment */
    uint32     code_offset;  /* Offset in this file where code segment starts */
    uint32     data_target;  /* Address in RAM where data is copied */
    uint32     data_size;    /* Size of Data Segment */
    uint32     data_offset;  /* Offset in this file where data segment starts */
    uint32     bss_target;   /* Address in RAM where BSS is located */
    uint32     bss_size;     /* Size of BSS Segment */
    char       object_name[OBJ_NAME_SIZE];  /* Name of Object -- CI_APP */
    char       entry_point_name[OBJ_NAME_SIZE]; /* Name of Entry point -- CI_AppMain */

} static_load_file_header_t;

#endif 

