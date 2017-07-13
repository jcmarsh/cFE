
/*
 * Filename: loadstaticloadfile.c
 *
 * Purpose: This file contains functions to load a static load file into memory.
 *
 */

/*
 * Includes
 */

#include "staticloadfile.h"
#include "LzmaDec.h"
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include "unistd.h"

/*
 * Macro Definitions
 */

#define LZMA_UNCOMPRESSEDDATASIZE_SIZE      sizeof(uint32)

/*
 * Type Definitions
 */

typedef struct {
    uint8               Properties[LZMA_PROPS_SIZE];
    uint8               UncompressedDataSize[LZMA_UNCOMPRESSEDDATASIZE_SIZE]; /* byte encoded in little endian format */
} Lzma_Header_t;

typedef struct {
    Lzma_Header_t       Header;
    uint8               Data;
} Lzma_Archive_t;

/*
 * Local Data
 */

static void            *SzAlloc(void *p, size_t size) { p = p; return malloc(size); }
static void             SzFree(void *p, void *address) { p = p; free(address); }
static ISzAlloc         g_Alloc = { SzAlloc, SzFree };

/*
 * Local Function Prototypes
 */

int                     Lzma_Read(int FileDescriptor, uint8 *UncompressedData, uint32 FileSize);

/*
 * Function Definitions
 */

/* Load the specified static load file into memory and return the contents of the static load file header
 * in the FileHeader structure */
boolean LoadStaticLoadFile(char *Filename, static_load_file_header_t *FileHeader)
{
    int                 FileDescriptor;
    boolean             ReturnStatus = TRUE;
           
    if ((FileDescriptor = open(Filename, O_RDONLY, 0)) != -1) {

printf("LSL: File opened\n");
        if ((read(FileDescriptor, FileHeader, sizeof(static_load_file_header_t))) == sizeof(static_load_file_header_t)) {

            if (FileHeader->file_marker == STATIC_FILE_MARKER) {

                if ((FileHeader->code_target != 0) &&
                    (FileHeader->code_size > 0)) {

printf("LSL: Read file descriptor\n");
                    lseek(FileDescriptor, FileHeader->code_offset, 0);
                    if (FileHeader->flags == STATIC_FILE_LZMA) {
                        if (Lzma_Read(FileDescriptor, (void *)FileHeader->code_target, FileHeader->code_size) != (int32)FileHeader->code_size) {
                            ReturnStatus = FALSE;
                        }
                    }
                    else {

printf("LSL: Reading code ..\n");
                        if ((read(FileDescriptor, (void *)FileHeader->code_target, FileHeader->code_size)) != (int32)FileHeader->code_size) {
                            ReturnStatus = FALSE;
                        }
printf("LSL: Reading code .. Done\n");
                    }
                }

                if (ReturnStatus == TRUE) {

                    if ((FileHeader->data_target != 0) &&
                        (FileHeader->data_size > 0)) {

                        lseek(FileDescriptor, FileHeader->data_offset, 0);
                        if (FileHeader->flags == STATIC_FILE_LZMA) {
                            if (Lzma_Read(FileDescriptor, (void *)FileHeader->data_target, FileHeader->data_size) != (int32)FileHeader->data_size) {
                                ReturnStatus = FALSE;
                            }
                        }
                        else {

printf("LSL: Reading Data ..\n");
                            if ((read(FileDescriptor, (void *)FileHeader->data_target, FileHeader->data_size)) != (int32)FileHeader->data_size) {
                                ReturnStatus = FALSE;
                            }
printf("LSL: Reading Data .. done\n");
                        }
                    }
                }

                if (ReturnStatus == TRUE) {

                    if ((FileHeader->bss_target != 0) &&
                        (FileHeader->bss_size > 0)) {

printf("LSL: clearing BSS\n");
printf("LSL:    bss_target = 0x%08X\n",FileHeader->bss_target);
printf("LSL:    bss_size = 0x%08X\n",FileHeader->bss_size);
                        memset((void *)FileHeader->bss_target, 0, FileHeader->bss_size);
printf("LSL: clearing BSS.. done\n");
                    }
                }
            }
            else { /* Not a static load file */
                ReturnStatus = FALSE;
            }

            /* If an error occurs after the File Header is read memset the File Header to 0
             * to avoid any problems where the calling code tries to use invalid data
             * from the File Header */
            if (ReturnStatus == FALSE) {
                memset((void *)FileHeader, 0, sizeof(static_load_file_header_t));
            }
        }
        else { /* Error reading file header */
            ReturnStatus = FALSE;
        }

        close(FileDescriptor);
    }
    else { /* File open error */
        ReturnStatus = FALSE;
    }

    return(ReturnStatus);
}

/* Allocate a tempory buffer, read the compressed data from the file into the temporary buffer, and then uncompresses the
 * data directly into the target memory.  The FileSize parameter is the size of the compressed archive, not the uncompressed
 * size.  The UncompressedData pointer points to a buffer that must be large enough to hold the uncompressed result */
int Lzma_Read(int FileDescriptor, uint8 *UncompressedData, uint32 FileSize)
{

    size_t              UncompressedDataSize = 0;
    Lzma_Archive_t     *CompressedData = NULL;
    size_t              CompressedDataSize = 0;
    ELzmaStatus         Status = 0;
    int                 ReturnStatus = 0;
    uint32              i;

    if (FileSize > 0) {

        /* Allocate a temporary buffer */
        if ((CompressedData = malloc(FileSize)) != NULL) {

            /* Read the compressed data into the temporary buffer */
            if ((read(FileDescriptor, CompressedData, FileSize)) == (int32)FileSize) {

                /* Extract the compressed and uncompressed data sizes */
                CompressedDataSize = FileSize - sizeof(Lzma_Header_t);
                UncompressedDataSize = 0;
                for (i=0; i < LZMA_UNCOMPRESSEDDATASIZE_SIZE; i++)
                    UncompressedDataSize += (uint32)CompressedData->Header.UncompressedDataSize[i] << (i * 8);

                /* Uncompress the data into the target memory */
                if ((LzmaDecode(UncompressedData, &UncompressedDataSize, &CompressedData->Data, &CompressedDataSize,
                               (uint8 *)&CompressedData->Header.Properties, LZMA_PROPS_SIZE, LZMA_FINISH_ANY, &Status, &g_Alloc)) == SZ_OK) {
                    
                    ReturnStatus = FileSize;
                }
                else { /* Decompression Error */
                    ReturnStatus = -1;
                }
            }
            else { /* File Read Error */
                ReturnStatus = -1;
            }

            free(CompressedData);
        }
        else { /* Memory Allocation Error */
            ReturnStatus = -1;
        }
    }
    else { /* FileSize = 0 */
        ReturnStatus = 0;
    }

    return(ReturnStatus);
}
