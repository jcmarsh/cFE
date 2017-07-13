
/*
 * Filename: elf2staticload.c
 *
 * Purpose: This application extracts the text, data, and bss sections from the specified statically linked ELF file and builds
 *   a CFE static load file (SLF).
 *
 */

/*
 * Includes
 */

#include "elf2staticload.h"
#include "staticloadfile.h"
#include "elffile.h"
#include "cmdlineopt.h"
#include "LzmaEnc.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>

/*
 * Macro Definitions
 */

#define LZMA_UNCOMPRESSEDDATASIZE_SIZE      sizeof(uint32)

/*
 * Type Definitions
 */

typedef struct {
    uint8                   Properties[LZMA_PROPS_SIZE];
    uint8                   UncompressedDataSize[LZMA_UNCOMPRESSEDDATASIZE_SIZE]; /* byte encoded in little endian format */
} Lzma_Header_t;

typedef struct {
    Lzma_Header_t           Header;
    uint8                   Data;
} Lzma_Archive_t;

typedef struct {
    uint8                  *SectionData;
    uint32                  SectionDataSize;
} Lzma_CompressedSection_t;

/*
 * Local Data
 */

CommandLineOptions_t        CommandLineOptions;
void                       *ELF_EntryPoint       = NULL;
ELF_Section_t              *ELF_TextSection      = NULL;
ELF_Section_t              *ELF_DataSection      = NULL;
ELF_Section_t              *ELF_BSSSection       = NULL;
ELF_Symbol_t               *ELF_EntryPointSymbol = NULL;
Lzma_CompressedSection_t    Lzma_CompressedTextSection;
Lzma_CompressedSection_t    Lzma_CompressedDataSection;
void                       *SzAlloc(void *p, size_t size) { p = p; return malloc(size); }
void                        SzFree(void *p, void *address) { p = p; free(address); }
ISzAlloc                    g_Alloc = { SzAlloc, SzFree };

/*
 * Local Function Prototypes
 */

void                        ReadELFFile(char *InputFilename);
void                        WriteStaticLoadFile(char *OutputFilename);
void                        ByteSwapOutputFileHeader(static_load_file_header_t *OutputFileHeader);
void                        PrintOutputFileHeader(static_load_file_header_t *OutputFileHeader);
void                        HexDump(void *Memory, uint32 Length);
void                        Lzma_CompressSection(uint8 *UncompressedData, size_t UncompressedDataSize, Lzma_CompressedSection_t *CompressedSection);
void                        Lzma_FreeSection(Lzma_CompressedSection_t *CompressedSection);

/*
 * Function Definitions
 */

int main(int argc, char *argv[])
{
    SetCommandLineOptionsDefaults(&CommandLineOptions);
    ProcessCommandLineOptions(argc, argv, &CommandLineOptions);

    ReadELFFile(CommandLineOptions.InputFilename);

    if (CommandLineOptions.Compression) {
        Lzma_CompressSection(ELF_TextSection->SectionData, ELF_TextSection->SectionDataSize, &Lzma_CompressedTextSection);
        if ((ELF_DataSection) && (ELF_DataSection->SectionDataSize > 0))
            Lzma_CompressSection(ELF_DataSection->SectionData, ELF_DataSection->SectionDataSize, &Lzma_CompressedDataSection);
    }

    WriteStaticLoadFile(CommandLineOptions.OutputFilename);

    if (CommandLineOptions.Compression) {
        Lzma_FreeSection(&Lzma_CompressedTextSection);
        Lzma_FreeSection(&Lzma_CompressedDataSection);
    }
    ELF_FreeSection(ELF_TextSection);
    ELF_FreeSection(ELF_DataSection);
    ELF_FreeSection(ELF_BSSSection);
    ELF_FreeSymbol(ELF_EntryPointSymbol);
    ELF_CloseFile();

    return (EXIT_SUCCESS);
}

/* Open the ELF file and extract the .text, .data, and .bss sections */
void ReadELFFile(char *InputFilename)
{
    ELF_OpenFile(InputFilename);

    ELF_EntryPoint = ELF_GetEntryPoint();

    if ((ELF_TextSection = ELF_ReadSectionByName(".text")) == NULL)
        UglyExit("Error: ELF File Missing .text Section\n");

    /* Note: it is ok if there is no Data or BSS section */
    ELF_DataSection = ELF_ReadSectionByName(".data");
    ELF_BSSSection = ELF_ReadSectionByName(".bss");

    /* Using the entry point address try to find the symbol name in the symbol table */
    ELF_EntryPointSymbol = ELF_ReadSymbolByValue(ELF_EntryPoint);

    /* If we didn't find the entry point symbol name and the both the entry point
     * name and the object name was not overridden on the command line then flag an error */
    if ((ELF_EntryPointSymbol == NULL) &&
        (CommandLineOptions.EntryPointNameOverrideEnabled == FALSE ||
         CommandLineOptions.ObjectNameOverrideEnabled == FALSE)) {
        UglyExit("Error: Can't Find Entry Point Name - Override Entry Point Name and Object Name on the Command Line\n");
    }

    if (CommandLineOptions.PrintInputFile)
        ELF_PrintFile();
}

/* Write the static load file */
void WriteStaticLoadFile(char *OutputFilename)
{
    FILE                       *OutputFilePointer;
    static_load_file_header_t   OutputFileHeader;

    if ((OutputFilePointer = fopen(OutputFilename, "w"))) { 

        memset(&OutputFileHeader, 0, sizeof(static_load_file_header_t));

        /* Format the output file header */
        OutputFileHeader.file_marker = STATIC_FILE_MARKER;
        OutputFileHeader.entry_point = (uint32)ELF_EntryPoint;

        if (CommandLineOptions.Compression) {

            OutputFileHeader.flags = STATIC_FILE_LZMA;
            OutputFileHeader.code_target = (uint32)ELF_TextSection->SectionDataAddress;
            OutputFileHeader.code_size = Lzma_CompressedTextSection.SectionDataSize;
            OutputFileHeader.code_offset = sizeof(static_load_file_header_t);

            if ((ELF_DataSection) && (ELF_DataSection->SectionDataSize > 0)) {
                OutputFileHeader.data_target = (uint32)ELF_DataSection->SectionDataAddress;
                OutputFileHeader.data_size = Lzma_CompressedDataSection.SectionDataSize;
                OutputFileHeader.data_offset = sizeof(static_load_file_header_t) + OutputFileHeader.code_size;
            }
        }
        else {

            OutputFileHeader.flags = STATIC_FILE_UNCOMPRESSED;
            OutputFileHeader.code_target = (uint32)ELF_TextSection->SectionDataAddress;
            OutputFileHeader.code_size = ELF_TextSection->SectionDataSize;
            OutputFileHeader.code_offset = sizeof(static_load_file_header_t);

            if ((ELF_DataSection) && (ELF_DataSection->SectionDataSize > 0)) {
                OutputFileHeader.data_target = (uint32)ELF_DataSection->SectionDataAddress;
                OutputFileHeader.data_size = ELF_DataSection->SectionDataSize;
                OutputFileHeader.data_offset = sizeof(static_load_file_header_t) + OutputFileHeader.code_size;
            }
        }

        if ((ELF_BSSSection) && (ELF_BSSSection->SectionDataSize > 0)) {
            OutputFileHeader.bss_target = (uint32)ELF_BSSSection->SectionDataAddress;
            OutputFileHeader.bss_size = ELF_BSSSection->SectionDataSize;
        }

        if (CommandLineOptions.ObjectNameOverrideEnabled)
            strncpy(OutputFileHeader.object_name, CommandLineOptions.ObjectNameOverride, OBJ_NAME_SIZE);
        else if (ELF_EntryPointSymbol)
            strncpy(OutputFileHeader.object_name, ELF_EntryPointSymbol->SymbolName, OBJ_NAME_SIZE);

        if (CommandLineOptions.EntryPointNameOverrideEnabled)
            strncpy(OutputFileHeader.entry_point_name, CommandLineOptions.EntryPointNameOverride, OBJ_NAME_SIZE);
        else if (ELF_EntryPointSymbol)
            strncpy(OutputFileHeader.entry_point_name, ELF_EntryPointSymbol->SymbolName, OBJ_NAME_SIZE);

        if (CommandLineOptions.PrintOutputFileHeader)
            PrintOutputFileHeader(&OutputFileHeader);

        if (CommandLineOptions.PrintOutputFileData) {
            printf("\nStatic Load File Text Section:\n");
            HexDump(ELF_TextSection->SectionData, ELF_TextSection->SectionDataSize);
            if ((ELF_DataSection) && (ELF_DataSection->SectionDataSize > 0)) {
                printf("\nStatic Load File Data Section:\n");
                HexDump(ELF_DataSection->SectionData, ELF_DataSection->SectionDataSize);
            }
        }

        if (ELF_ByteSwapRequired())
            ByteSwapOutputFileHeader(&OutputFileHeader);

        fwrite(&OutputFileHeader, sizeof(static_load_file_header_t), 1, OutputFilePointer);
        if (CommandLineOptions.Compression) {

            fwrite(Lzma_CompressedTextSection.SectionData, Lzma_CompressedTextSection.SectionDataSize, 1, OutputFilePointer);
            if ((ELF_DataSection) && (ELF_DataSection->SectionDataSize > 0))
                fwrite(Lzma_CompressedDataSection.SectionData, Lzma_CompressedDataSection.SectionDataSize, 1, OutputFilePointer);
        }
        else {

            fwrite(ELF_TextSection->SectionData, ELF_TextSection->SectionDataSize, 1, OutputFilePointer);
            if ((ELF_DataSection) && (ELF_DataSection->SectionDataSize > 0))
                fwrite(ELF_DataSection->SectionData, ELF_DataSection->SectionDataSize, 1, OutputFilePointer);
        }

        fclose(OutputFilePointer);
    }
    else {
        UglyExit("Error Opening Output File: %s, %s\n", OutputFilename, strerror(errno));
    }
}

/* Byte swap the output file header */
void ByteSwapOutputFileHeader(static_load_file_header_t *OutputFileHeader)
{
    ELF_SwapUInt32(&OutputFileHeader->file_marker);
    ELF_SwapUInt32(&OutputFileHeader->entry_point);
    ELF_SwapUInt32(&OutputFileHeader->flags);
    ELF_SwapUInt32(&OutputFileHeader->code_target);
    ELF_SwapUInt32(&OutputFileHeader->code_size);
    ELF_SwapUInt32(&OutputFileHeader->code_offset);
    ELF_SwapUInt32(&OutputFileHeader->data_target);
    ELF_SwapUInt32(&OutputFileHeader->data_size);
    ELF_SwapUInt32(&OutputFileHeader->data_offset);
    ELF_SwapUInt32(&OutputFileHeader->bss_target);
    ELF_SwapUInt32(&OutputFileHeader->bss_size);
}

/* Print the contents of the output file header to the console */
void PrintOutputFileHeader(static_load_file_header_t *OutputFileHeader)
{
    printf("\nStatic Load File Header:\n");
    printf("   file_marker      = 0x%lx\n",   OutputFileHeader->file_marker);
    printf("   entry_point      = 0x%08lx\n", OutputFileHeader->entry_point);
    printf("   flags            = 0x%08lx\n", OutputFileHeader->flags);

    printf("   code_target      = 0x%08lx\n", OutputFileHeader->code_target);
    printf("   code_offset      = %lu\n",     OutputFileHeader->code_offset);
    printf("   code_size        = %lu\n",     OutputFileHeader->code_size);

    printf("   data_target      = 0x%08lx\n", OutputFileHeader->data_target);
    printf("   data_offset      = %lu\n",     OutputFileHeader->data_offset);
    printf("   data_size        = %lu\n",     OutputFileHeader->data_size);

    printf("   bss_target       = 0x%08lx\n", OutputFileHeader->bss_target);
    printf("   bss_size         = %lu\n",     OutputFileHeader->bss_size);

    printf("   object_name      = %s\n",      OutputFileHeader->object_name);
    printf("   entry_point_name = %s\n",      OutputFileHeader->entry_point_name);
}

/* Dump a section of memory to the console */
void  HexDump(void *Memory, uint32 Length)
{
    uint32       i;
    uint32       j;

    for (i=0; i < Length; i+=16) {
        printf("   %06lx: ", i);
        for (j=0; j < 16; j++) {
            if ((i+j) < Length)
                printf("%02x ", ((uint8 *)Memory)[i+j]);
            else
                printf("   ");
        }
        printf(" ");
        for (j=0; j < 16; j++) {
            if ((i+j) < Length)
                printf("%c", isprint(((uint8 *)Memory)[i+j]) ? ((uint8 *)Memory)[i+j] : '.');
        }
        printf("\n");
    }
}

/* Compress a section and return the compressed archive in the specified Lzma_CompressedSection_t structure.  Note that 
 * Lzma_FreeSection should be called to free the memory allocated by Lzma_CompressSection.  This function is just a wrapper
 * around the LzmaEncode function provided by the open source LZMA compression SDk from www.7-zip.org.  The LzmaEncode
 * function does not give me a compressed archive that contains everything I need to decompress it, so I made my own type
 * (Lzma_Archive_t) that includes a header with additional information along with the compressed data returned from the
 * SDK.  My implementation is identical to the format used by 7-zip with the exception of the compressed data size in my
 * implementation is only 32 bits where the 7-zip implementation uses 64 bits. */
void Lzma_CompressSection(uint8 *UncompressedData, size_t UncompressedDataSize, Lzma_CompressedSection_t *CompressedSection)
{
    CLzmaEncProps       Properties;
    size_t              PropertiesSize = LZMA_PROPS_SIZE;
    Lzma_Archive_t     *CompressedData = NULL;
    size_t              CompressedDataSize = 0;
    uint32              i;

    LzmaEncProps_Init(&Properties);

    /* Allocate a buffer for the compressed data, I assume that the compressed data will be smaller than the uncompressed
     * data however this is not always the case.  If the data to compress is small then there is a chance that the compression
     * will not be very good and the compressed data can be larger than the uncompressed data.  So to handle this case I
     * added an additional 500 bytes to the compressed data buffer. */
    CompressedDataSize = (UncompressedDataSize + sizeof(Lzma_Header_t) + 500);
    if ((CompressedData = malloc(CompressedDataSize)) != NULL) {

        memset(CompressedData, 0, CompressedDataSize);

        /* Byte encode the uncompressed data size in little endian format and store it in the header */
        for (i=0; i < LZMA_UNCOMPRESSEDDATASIZE_SIZE; i++)
           CompressedData->Header.UncompressedDataSize[i] = (Byte)(UncompressedDataSize >> (8 * i));

        /* Compress the data, this will also retutn a byte encoded version of the properties that is also
         * stored in the header */
        if ((LzmaEncode(&CompressedData->Data, &CompressedDataSize, UncompressedData, UncompressedDataSize, &Properties,
                        (uint8 *)&CompressedData->Header.Properties, &PropertiesSize, 0, NULL, &g_Alloc, &g_Alloc)) == SZ_OK) {

            /* Return the compressed data, note that I am actually returning a structure that contains the lzma
             * header along with the data.  The SectionDataSize is also adjusted to account for the header. */
            CompressedSection->SectionData = (uint8 *)CompressedData;
            CompressedSection->SectionDataSize = CompressedDataSize + sizeof(Lzma_Header_t);
        }
        else {  /* Compression Error */
            free(CompressedData);
            UglyExit("Error: Can't Compress Data Buffer\n"); /* FIXME maybe save the return status so I can debug problems */
        }
    }
    else {  /* Memory Allocation Error */
        UglyExit("Error: Can't Allocate Memory For Compressed Data Buffer: %u\n", CompressedDataSize);
    }
}

/* Free memory allocated in Lzma_CompressSection */
void Lzma_FreeSection(Lzma_CompressedSection_t *CompressedSection)
{
    if (CompressedSection != NULL) {
        if (CompressedSection->SectionData != NULL)
            free(CompressedSection->SectionData);
        CompressedSection->SectionData = NULL;
        CompressedSection->SectionDataSize = 0;
    }
}

/* Print an error message to the console and exit */
void UglyExit(char *Spec, ...)
{
    va_list         Args;
    static char     Text[256];

    va_start(Args, Spec);
    vsprintf(Text, Spec, Args);
    va_end(Args);

    printf("%s", Text);
    exit(1);
}
