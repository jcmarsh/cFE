
/*
 * Filename: cmdlineopt.h
 *
 * Purpose: This file contains the typedefs and function prototypes for the file cmdlineopt.c.
 *
 * Design Notes:
 *
 * References:
 *
 */

#ifndef _cmdlineopt_
#define	_cmdlineopt_

/*
 * Includes
 */

#include "common_types.h"
#include "staticloadfile.h"

/*
 * Macro Definitions
 */

#define     MAX_FILENAME_SIZE           64

/*
 * Type Definitions
 */

typedef struct {
    char            InputFilename[MAX_FILENAME_SIZE];
    char            OutputFilename[MAX_FILENAME_SIZE];
    char            EntryPointNameOverride[OBJ_NAME_SIZE];
    boolean         EntryPointNameOverrideEnabled;
    char            ObjectNameOverride[OBJ_NAME_SIZE];
    boolean         ObjectNameOverrideEnabled;
    boolean         PrintInputFile;
    boolean         PrintOutputFileHeader;
    boolean         PrintOutputFileData;
    boolean         Compression;
} CommandLineOptions_t;

/*
 * Exported Functions
 */

/* Set the default values for the command line options, this should be called before calling
 * ProcessCommandLineOptions */
void SetCommandLineOptionsDefaults(CommandLineOptions_t *CommandLineOptions);

/* Process command line options, options specified on the command line will override the values
 * set by SetCommandLineOptionsDefauls */
void ProcessCommandLineOptions(int argc, char *argv[], CommandLineOptions_t *CommandLineOptions);

#endif	

