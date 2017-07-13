
/*
 * Filename: loadstaticloadfile.h
 *
 * Purpose: This file contains typedefs and function prototypes for the file loadstaticloadfile.c. 
 *
 * Design Notes:
 *
 * References:
 *
 */

#ifndef _loadstaticloadfile_
#define _loadstaticloadfile_

/*
 * Includes
 */

#include "staticloadfile.h"

/*
 * Exported Functions
 */

/* Load the specified static load file into memory and return the contents of the static load file header
 * in the FileHeader structure */
boolean             LoadStaticLoadFile(char *Filename, static_load_file_header_t *FileHeader);

#endif

