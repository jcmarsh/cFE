
/*
 * Filename: cmdlineopt.c
 *
 * Purpose: This file contains functions for processing command line options.
 *
 */

/*
 * Includes
 */

#include "common_types.h"
#include "cmdlineopt.h"
#include "elf2staticload.h"
#include <getopt.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/*
 * Local Function Prototypes
 */

void            DisplayVersion(void);
void            DisplayUsage(void);

/*
 * Function Definitions
 */

/* Set the default values for the command line options, this should be called before calling
 * ProcessCommandLineOptions */
void SetCommandLineOptionsDefaults(CommandLineOptions_t *CommandLineOptions)
{
    memset(&CommandLineOptions->InputFilename, 0, MAX_FILENAME_SIZE);
    memset(&CommandLineOptions->OutputFilename, 0, MAX_FILENAME_SIZE);
    memset(&CommandLineOptions->ObjectNameOverride, 0, OBJ_NAME_SIZE);
    CommandLineOptions->ObjectNameOverrideEnabled = FALSE;
    memset(&CommandLineOptions->EntryPointNameOverride, 0, OBJ_NAME_SIZE);
    CommandLineOptions->EntryPointNameOverrideEnabled = FALSE;
    CommandLineOptions->PrintInputFile = FALSE;
    CommandLineOptions->PrintOutputFileHeader = FALSE;
    CommandLineOptions->PrintOutputFileData = FALSE;
    CommandLineOptions->Compression = FALSE;
}

/* Process command line options, options specified on the command line will override the values
 * set by SetCommandLineOptionsDefauls */
void ProcessCommandLineOptions(int argc, char *argv[], CommandLineOptions_t *CommandLineOptions)
{
    int   opt = 0;
    int   longIndex = 0;

    static const char *optString = "o:e:cIHDVh";

    static const struct option longOpts[] = {
            { "object_name",              required_argument, NULL, 'o' },
            { "entry_point_name",         required_argument, NULL, 'e' },
            { "compression",              no_argument      , NULL, 'c' },
            { "print_input_file",         no_argument,       NULL, 'I' },
            { "print_output_file_header", no_argument,       NULL, 'H' },
            { "print_output_file_data",   no_argument,       NULL, 'D' },
            { "version",                  no_argument,       NULL, 'V' },
            { "help",                     no_argument,       NULL, 'h' },
            { NULL,                       no_argument,       NULL, 0 }
    };

    /* Process optional parameters */
    opt = getopt_long(argc, argv, optString, longOpts, &longIndex);
    while(opt != -1)
    {
        switch(opt)
        {

            case 'o':
                if (strlen(optarg) < OBJ_NAME_SIZE) {
                    strncpy(CommandLineOptions->ObjectNameOverride, optarg, OBJ_NAME_SIZE);
                    CommandLineOptions->ObjectNameOverrideEnabled = TRUE;
                }
                else {
                    UglyExit("Error: Object Name Too Long, Max Length: %lu\n", OBJ_NAME_SIZE);                    
                }
                break;

            case 'e':
                if (strlen(optarg) < OBJ_NAME_SIZE) {
                    strncpy(CommandLineOptions->EntryPointNameOverride, optarg, OBJ_NAME_SIZE);
                    CommandLineOptions->EntryPointNameOverrideEnabled = TRUE;
                }
                else {
                    UglyExit("Error: Entry Point Name Too Long, Max Length: %lu\n", OBJ_NAME_SIZE);
                }
                break;

            case 'c':
                CommandLineOptions->Compression = TRUE;
                break;

            case 'I':
                CommandLineOptions->PrintInputFile = TRUE;
                break;

            case 'H':
                CommandLineOptions->PrintOutputFileHeader = TRUE;
                break;

            case 'D':
                CommandLineOptions->PrintOutputFileData = TRUE;
                break;

            case 'V':
                DisplayVersion();
                break;

            case 'h':
                DisplayUsage();
                break;

            default:
                break;
        }

        opt = getopt_long(argc, argv, optString, longOpts, &longIndex);
    }

    /* Adjust argc and argv to remove all of the options we have already processed */
    argc -= optind;
    argv += optind;

    /* The Input Filename and the Output Filename are required parameters */
    if (argc == 2) {
        strncpy(CommandLineOptions->InputFilename, argv[0], MAX_FILENAME_SIZE);
        strncpy(CommandLineOptions->OutputFilename, argv[1], MAX_FILENAME_SIZE);
    }
    else {
        DisplayUsage();
    }
}

/* Displays version information on the console and exits */
void DisplayVersion(void)
{
    printf("elf2staticload     %.1f\n", VERSION_NUMBER);
    printf("\n");
    exit(1);
}

/* Displays usage information on the console and exits */
void DisplayUsage(void)
{
    printf("Usage: elf2staticload [OPTION]... INPUT_FILE OUTPUT_FILE\n");
    printf("Converts a static link ELF file into a CFE load file:\n");
    printf(" \n");
    printf("  Options:\n");
    printf("  -o, --object_name                 override the default object name\n");
    printf("  -e, --entry_point_name            override the default entry point name\n");
    printf("  -c, --compression                 compress the text and data sections\n");
    printf("  -I, --print_input_file            print the contents of the ELF file to\n");
    printf("                                      the console\n");
    printf("  -H, --print_output_file_header    print the CFE static link file header\n");
    printf("                                      to the console\n");
    printf("  -D, --print_output_file_data      print the CFE static link file data to\n");
    printf("                                      the console\n");
    printf("  -V, --version                     output version information and exit\n");
    printf("  -h, --help                        output usage information and exit\n");
    printf(" \n");
    printf("  By default the object and entry point names are extracted from\n");
    printf("    the ELF file.\n");
    printf(" \n");
    exit(1);
}
