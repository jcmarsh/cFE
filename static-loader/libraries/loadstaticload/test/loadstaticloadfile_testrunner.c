
/*
 * Filename: loadstaticloadfile_testrunner.c
 *
 * Purpose: This file contains a unit test runner for the Static File Loader.
 *
 */

/*
 * Includes
 */

#include "uttest.h"

/*
 * External Function Prototypes
 */

void LoadStaticLoadFile_AddTestCase(void);

/*
 * Function Definitions
 */

int main(void)
{
    LoadStaticLoadFile_AddTestCase();
    return(UtTest_Run());
}

