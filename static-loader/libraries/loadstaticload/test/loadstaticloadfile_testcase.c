
/*
 * Filename: loadstaticloadfile_testcase.c
 *
 * Purpose: This file contains a unit test case for the functions contained in the file loadstaticloadfile.c.
 *
 */

/*
 * Includes
 */

#include "utassert.h"
#include "uttest.h"
#include "staticloadfile.h"
#include "loadstaticloadfile.h"
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include "unistd.h"

/*
 * Macro Definitions
 */

#define TEST_CODE_BUFFER_SIZE               500000
#define TEST_DATA_BUFFER_SIZE               30000
#define TEST_BSS_BUFFER_SIZE                3000

#define TEST_COMPRESSED_CODE_SIZE           49977
#define TEST_UNCOMPRESSED_CODE_SIZE         194704
#define TEST_COMPRESSED_DATA_SIZE           1561
#define TEST_UNCOMPRESSED_DATA_SIZE         6288

/*
 * Local Data
 */

uint8                          *TestCodeBuffer = NULL;
uint8                          *TestDataBuffer = NULL;
uint8                          *TestBSSBuffer = NULL;

/*
 * Local function in loadstaticloadfile.c that is not exported
 */

int                             Lzma_Read(int FileDescriptor, uint8 *UncompressedData, uint32 FileSize);

/*
 * Function Definitions
 */

void CreateStaticLoadFile(char *Filename, uint32 FileMarker, uint32 Flags, uint32 CodeSize, uint32 DataSize, uint32 BSSSize)
{
    static_load_file_header_t    FileHeader;
    FILE                        *FilePointer;

    TestCodeBuffer = malloc(CodeSize);
    TestDataBuffer = malloc(DataSize);
    TestBSSBuffer  = malloc(BSSSize);

    memset(TestCodeBuffer, 1, CodeSize);
    memset(TestDataBuffer, 2, DataSize);
    
    memset(&FileHeader, 0, sizeof(static_load_file_header_t));

    FileHeader.file_marker = FileMarker;
    FileHeader.entry_point = 0x12345678;
    FileHeader.flags = Flags;

    if (CodeSize > 0) {
        FileHeader.code_target = (uint32)TestCodeBuffer;
        FileHeader.code_size = CodeSize;
        FileHeader.code_offset = sizeof(static_load_file_header_t);
    }

    if (DataSize > 0) {
        FileHeader.data_target = (uint32)TestDataBuffer;
        FileHeader.data_size = DataSize;
        FileHeader.data_offset = sizeof(static_load_file_header_t) + FileHeader.code_size;
    }

    if (BSSSize) {
        FileHeader.bss_target = (uint32)TestBSSBuffer;
        FileHeader.bss_size = BSSSize;
    }

    strncpy(FileHeader.object_name, "Test App", OBJ_NAME_SIZE);
    strncpy(FileHeader.entry_point_name, "Test Main", OBJ_NAME_SIZE);

    FilePointer = fopen(Filename, "w");

    fwrite(&FileHeader, sizeof(static_load_file_header_t), 1, FilePointer);
    fwrite(TestCodeBuffer, CodeSize, 1, FilePointer);
    fwrite(TestDataBuffer, DataSize, 1, FilePointer);

    fclose(FilePointer);

    memset(TestCodeBuffer, 0xFF, CodeSize);
    memset(TestDataBuffer, 0xFF, DataSize);
    memset(TestBSSBuffer,  0xFF, BSSSize);
}

void CreateCompressedStaticLoadFile(char *Filename, char *CompressedCodeFilename, uint32 CompressedCodeSize, uint32 UncompressedCodeSize,
                                    char *CompressedDataFilename, uint32 CompressedDataSize, uint32 UncompressedDataSize, uint32 BSSSize)
{
    static_load_file_header_t    FileHeader;
    FILE                        *FilePointer;

    TestCodeBuffer = malloc(UncompressedCodeSize);
    TestDataBuffer = malloc(UncompressedDataSize);
    TestBSSBuffer  = malloc(BSSSize);

    memset(&FileHeader, 0, sizeof(static_load_file_header_t));

    FileHeader.file_marker = STATIC_FILE_MARKER;
    FileHeader.entry_point = 0x12345678;
    FileHeader.flags = STATIC_FILE_LZMA;

    if (CompressedCodeSize > 0) {
        FileHeader.code_target = (uint32)TestCodeBuffer;
        FileHeader.code_size = CompressedCodeSize;
        FileHeader.code_offset = sizeof(static_load_file_header_t);
        UtBinFile2Mem(TestCodeBuffer, CompressedCodeFilename, CompressedCodeSize);
    }

    if (CompressedDataSize > 0) {
        FileHeader.data_target = (uint32)TestDataBuffer;
        FileHeader.data_size = CompressedDataSize;
        FileHeader.data_offset = sizeof(static_load_file_header_t) + FileHeader.code_size;
        UtBinFile2Mem(TestDataBuffer, CompressedDataFilename, CompressedDataSize);
    }

    if (BSSSize) {
        FileHeader.bss_target = (uint32)TestBSSBuffer;
        FileHeader.bss_size = BSSSize;
    }

    strncpy(FileHeader.object_name, "Test App", OBJ_NAME_SIZE);
    strncpy(FileHeader.entry_point_name, "Test Main", OBJ_NAME_SIZE);

    FilePointer = fopen(Filename, "w");

    fwrite(&FileHeader, sizeof(static_load_file_header_t), 1, FilePointer);
    fwrite(TestCodeBuffer, CompressedCodeSize, 1, FilePointer);
    fwrite(TestDataBuffer, CompressedDataSize, 1, FilePointer);

    fclose(FilePointer);

    memset(TestCodeBuffer, 0xFF, UncompressedCodeSize);
    memset(TestDataBuffer, 0xFF, UncompressedDataSize);
    memset(TestBSSBuffer,  0xFF, BSSSize);
}

void FreeStaticLoadFile(char *Filename)
{
    unlink(Filename);
    free(TestCodeBuffer); TestCodeBuffer = NULL;
    free(TestDataBuffer); TestDataBuffer = NULL;
    free(TestBSSBuffer);  TestBSSBuffer  = NULL;
}

/* LoadStaticLoadFile_Test01 - Success */
void LoadStaticLoadFile_Test01(void)
{
    static_load_file_header_t   FileHeader;
    boolean                     ReturnStatus;

    /* Setup Inputs */
    CreateStaticLoadFile("staticload.slf", STATIC_FILE_MARKER, STATIC_FILE_UNCOMPRESSED, TEST_CODE_BUFFER_SIZE, TEST_DATA_BUFFER_SIZE, TEST_BSS_BUFFER_SIZE);
    memset(&FileHeader, 0, sizeof(static_load_file_header_t));

    /* Execute Test */
    ReturnStatus = LoadStaticLoadFile("staticload.slf", &FileHeader);

    /* Verify Outputs */

    /* Success Return Code */
    UtAssert_True(ReturnStatus == TRUE, "ReturnStatus == TRUE");

    /* Verify File Header */
    UtAssert_True(FileHeader.file_marker == STATIC_FILE_MARKER, "FileHeader.file_marker == STATIC_FILE_MARKER");
    UtAssert_True(FileHeader.entry_point == 0x12345678, "FileHeader.entry_point == 0x12345678");
    UtAssert_True(FileHeader.flags == STATIC_FILE_UNCOMPRESSED, "FileHeader.flags == STATIC_FILE_UNCOMPRESSED");
    UtAssert_True(FileHeader.code_target == (uint32)TestCodeBuffer, "FileHeader.code_target == TestCodeBuffer");
    UtAssert_True(FileHeader.code_size == TEST_CODE_BUFFER_SIZE, "FileHeader.code_size == TEST_CODE_BUFFER_SIZE");
    UtAssert_True(FileHeader.code_offset == sizeof(static_load_file_header_t), "FileHeader.code_offset == sizeof(static_load_file_header_t)");
    UtAssert_True(FileHeader.data_target == (uint32)TestDataBuffer, "FileHeader.data_target == TestDataBuffer");
    UtAssert_True(FileHeader.data_size == TEST_DATA_BUFFER_SIZE, "FileHeader.data_size == TEST_DATA_BUFFER_SIZE");
    UtAssert_True(FileHeader.data_offset == sizeof(static_load_file_header_t) + FileHeader.code_size, "FileHeader.data_offset == sizeof(static_load_file_header_t) + FileHeader.code_size");
    UtAssert_True(FileHeader.bss_target == (uint32)TestBSSBuffer, "FileHeader.bss_target == TestBSSBuffer");
    UtAssert_True(FileHeader.bss_size == TEST_BSS_BUFFER_SIZE, "FileHeader.bss_size == TEST_BSS_BUFFER_SIZE");
    UtAssert_StrCmp(FileHeader.object_name, "Test App", "FileHeader.object_name == \"Test App\"");
    UtAssert_StrCmp(FileHeader.entry_point_name, "Test Main", "FileHeader.entry_point_name == \"Test Main\"");

    /* Verify Data is copied from the file to the correct memory locations */
    UtAssert_MemCmpValue(TestCodeBuffer, 1, TEST_CODE_BUFFER_SIZE, "Text Memory is all 1's");
    UtAssert_MemCmpValue(TestDataBuffer, 2, TEST_DATA_BUFFER_SIZE, "Data Memory is all 2's");
    UtAssert_MemCmpValue(TestBSSBuffer,  0, TEST_BSS_BUFFER_SIZE,  "BSS Memory is all 0's");

    FreeStaticLoadFile("staticload.slf");
}

/* LoadStaticLoadFile_Test02 - File Not Found */
void LoadStaticLoadFile_Test02(void)
{
    static_load_file_header_t   FileHeader;
    boolean                     ReturnStatus;

    /* Setup Inputs */
    memset(&FileHeader, 0xFF, sizeof(static_load_file_header_t));

    /* Execute Test */
    ReturnStatus = LoadStaticLoadFile("staticload.slf", &FileHeader);

    /* Verify Outputs */
    UtAssert_True(ReturnStatus == FALSE, "ReturnStatus == FALSE");
    UtAssert_MemCmpValue(&FileHeader, 0xFF, sizeof(static_load_file_header_t),  "File Header is all 0xFF's");
}

/* LoadStaticLoadFile_Test03 - Error Reading File Header */
void LoadStaticLoadFile_Test03(void)
{
    static_load_file_header_t   FileHeader;
    boolean                     ReturnStatus;
    FILE                       *FilePointer;

    /* Setup Inputs */
    memset(&FileHeader, 0xFF, sizeof(static_load_file_header_t));

    /* Create a file that is smaller than the file header to create a error reading the file header */
    FilePointer = fopen("staticload.slf", "w");
    fwrite(&FileHeader, sizeof(static_load_file_header_t)-1, 1, FilePointer);
    fclose(FilePointer);

    /* Execute Test */
    ReturnStatus = LoadStaticLoadFile("staticload.slf", &FileHeader);

    /* Verify Outputs */
    UtAssert_True(ReturnStatus == FALSE, "ReturnStatus == FALSE");
    UtAssert_MemCmpValue(&FileHeader, 0xFF, sizeof(static_load_file_header_t),  "File Header is all 0xFF's");

    unlink("staticload.slf");
}

/* LoadStaticLoadFile_Test04 - Not a Static Load File */
void LoadStaticLoadFile_Test04(void)
{
    static_load_file_header_t   FileHeader;
    boolean                     ReturnStatus;

    /* Setup Inputs */
    CreateStaticLoadFile("staticload.slf", 0x12345678, STATIC_FILE_UNCOMPRESSED, TEST_CODE_BUFFER_SIZE, TEST_DATA_BUFFER_SIZE, TEST_BSS_BUFFER_SIZE);
    memset(&FileHeader, 0xFF, sizeof(static_load_file_header_t));

    /* Execute Test */
    ReturnStatus = LoadStaticLoadFile("staticload.slf", &FileHeader);

    /* Verify Outputs */
    UtAssert_True(ReturnStatus == FALSE, "ReturnStatus == FALSE");
    UtAssert_MemCmpValue(&FileHeader, 0, sizeof(static_load_file_header_t),  "File Header is all 0's");

    FreeStaticLoadFile("staticload.slf");
}

/* LoadStaticLoadFile_Test05 - Filename is NULL */
void LoadStaticLoadFile_Test05(void)
{
    static_load_file_header_t   FileHeader;
    boolean                     ReturnStatus;

    /* Setup Inputs */
    CreateStaticLoadFile("staticload.slf", STATIC_FILE_MARKER, STATIC_FILE_UNCOMPRESSED, TEST_CODE_BUFFER_SIZE, TEST_DATA_BUFFER_SIZE, TEST_BSS_BUFFER_SIZE);
    memset(&FileHeader, 0xFF, sizeof(static_load_file_header_t));

    /* Execute Test */
    ReturnStatus = LoadStaticLoadFile(NULL, &FileHeader);

    /* Verify Outputs */
    UtAssert_True(ReturnStatus == FALSE, "ReturnStatus == FALSE");
    UtAssert_MemCmpValue(&FileHeader, 0xFF, sizeof(static_load_file_header_t),  "File Header is all 0xFF's");

    FreeStaticLoadFile("staticload.slf");
}

/* LoadStaticLoadFile_Test06 - FileHeader is NULL */
void LoadStaticLoadFile_Test06(void)
{
    boolean                     ReturnStatus;

    /* Setup Inputs */
    CreateStaticLoadFile("staticload.slf", STATIC_FILE_MARKER, STATIC_FILE_UNCOMPRESSED, TEST_CODE_BUFFER_SIZE, TEST_DATA_BUFFER_SIZE, TEST_BSS_BUFFER_SIZE);

    /* Execute Test */
    ReturnStatus = LoadStaticLoadFile("staticload.slf", NULL);

    /* Verify Outputs */
    UtAssert_True(ReturnStatus == FALSE, "ReturnStatus == FALSE");

    FreeStaticLoadFile("staticload.slf");
}

/* LoadStaticLoadFile_Test07 - No Code Segment */
void LoadStaticLoadFile_Test07(void)
{
    static_load_file_header_t   FileHeader;
    boolean                     ReturnStatus;

    /* Setup Inputs */
    CreateStaticLoadFile("staticload.slf", STATIC_FILE_MARKER, STATIC_FILE_UNCOMPRESSED, 0, TEST_DATA_BUFFER_SIZE, TEST_BSS_BUFFER_SIZE);
    memset(&FileHeader, 0xFF, sizeof(static_load_file_header_t));

    /* Execute Test */
    ReturnStatus = LoadStaticLoadFile("staticload.slf", &FileHeader);

    /* Verify Outputs */
    UtAssert_True(ReturnStatus == TRUE, "ReturnStatus == TRUE");

    /* Verify Data is copied from the file to the correct memory locations */
    UtAssert_MemCmpValue(TestDataBuffer, 2, TEST_DATA_BUFFER_SIZE, "Data Memory is all 2's");
    UtAssert_MemCmpValue(TestBSSBuffer,  0, TEST_BSS_BUFFER_SIZE,  "BSS Memory is all 0's");

    FreeStaticLoadFile("staticload.slf");
}

/* LoadStaticLoadFile_Test08 - No Data Segment */
void LoadStaticLoadFile_Test08(void)
{
    static_load_file_header_t   FileHeader;
    boolean                     ReturnStatus;

    /* Setup Inputs */
    CreateStaticLoadFile("staticload.slf", STATIC_FILE_MARKER, STATIC_FILE_UNCOMPRESSED, TEST_CODE_BUFFER_SIZE, 0, TEST_BSS_BUFFER_SIZE);
    memset(&FileHeader, 0xFF, sizeof(static_load_file_header_t));

    /* Execute Test */
    ReturnStatus = LoadStaticLoadFile("staticload.slf", &FileHeader);

    /* Verify Outputs */
    UtAssert_True(ReturnStatus == TRUE, "ReturnStatus == TRUE");

    /* Verify Data is copied from the file to the correct memory locations */
    UtAssert_MemCmpValue(TestCodeBuffer, 1, TEST_CODE_BUFFER_SIZE, "Text Memory is all 1's");
    UtAssert_MemCmpValue(TestBSSBuffer,  0, TEST_BSS_BUFFER_SIZE,  "BSS Memory is all 0's");

    FreeStaticLoadFile("staticload.slf");
}

/* LoadStaticLoadFile_Test09 - No BSS Segment */
void LoadStaticLoadFile_Test09(void)
{
    static_load_file_header_t   FileHeader;
    boolean                     ReturnStatus;

    /* Setup Inputs */
    CreateStaticLoadFile("staticload.slf", STATIC_FILE_MARKER, STATIC_FILE_UNCOMPRESSED, TEST_CODE_BUFFER_SIZE, TEST_DATA_BUFFER_SIZE, 0);
    memset(&FileHeader, 0xFF, sizeof(static_load_file_header_t));

    /* Execute Test */
    ReturnStatus = LoadStaticLoadFile("staticload.slf", &FileHeader);

    /* Verify Outputs */
    UtAssert_True(ReturnStatus == TRUE, "ReturnStatus == TRUE");

    /* Verify Data is copied from the file to the correct memory locations */
    UtAssert_MemCmpValue(TestCodeBuffer, 1, TEST_CODE_BUFFER_SIZE, "Text Memory is all 1's");
    UtAssert_MemCmpValue(TestDataBuffer, 2, TEST_DATA_BUFFER_SIZE, "Data Memory is all 2's");

    FreeStaticLoadFile("staticload.slf");
}

/* LoadStaticLoadFile_Test11 - Success Uncompression */
void LoadStaticLoadFile_Test11(void)
{
    static_load_file_header_t   FileHeader;
    boolean                     ReturnStatus;

    /* Setup Inputs */
    CreateCompressedStaticLoadFile("staticload.slf", "test_compressed_code.dat", TEST_COMPRESSED_CODE_SIZE, TEST_UNCOMPRESSED_CODE_SIZE,
                                   "test_compressed_data.dat", TEST_COMPRESSED_DATA_SIZE, TEST_UNCOMPRESSED_DATA_SIZE, TEST_BSS_BUFFER_SIZE);
    memset(&FileHeader, 0, sizeof(static_load_file_header_t));

    /* Execute Test */
    ReturnStatus = LoadStaticLoadFile("staticload.slf", &FileHeader);

    /* Verify Outputs */

    /* Success Return Code */
    UtAssert_True(ReturnStatus == TRUE, "ReturnStatus == TRUE");

    /* Verify File Header */
    UtAssert_True(FileHeader.file_marker == STATIC_FILE_MARKER, "FileHeader.file_marker == STATIC_FILE_MARKER");
    UtAssert_True(FileHeader.entry_point == 0x12345678, "FileHeader.entry_point == 0x12345678");
    UtAssert_True(FileHeader.flags == STATIC_FILE_LZMA, "FileHeader.flags == STATIC_FILE_LZMA");
    UtAssert_True(FileHeader.code_target == (uint32)TestCodeBuffer, "FileHeader.code_target == TestCodeBuffer");
    UtAssert_True(FileHeader.code_size == TEST_COMPRESSED_CODE_SIZE, "FileHeader.code_size == TEST_COMPRESSED_CODE_SIZE");
    UtAssert_True(FileHeader.code_offset == sizeof(static_load_file_header_t), "FileHeader.code_offset == sizeof(static_load_file_header_t)");
    UtAssert_True(FileHeader.data_target == (uint32)TestDataBuffer, "FileHeader.data_target == TestDataBuffer");
    UtAssert_True(FileHeader.data_size == TEST_COMPRESSED_DATA_SIZE, "FileHeader.data_size == TEST_COMPRESSED_DATA_SIZE");
    UtAssert_True(FileHeader.data_offset == sizeof(static_load_file_header_t) + FileHeader.code_size, "FileHeader.data_offset == sizeof(static_load_file_header_t) + FileHeader.code_size");
    UtAssert_True(FileHeader.bss_target == (uint32)TestBSSBuffer, "FileHeader.bss_target == TestBSSBuffer");
    UtAssert_True(FileHeader.bss_size == TEST_BSS_BUFFER_SIZE, "FileHeader.bss_size == TEST_BSS_BUFFER_SIZE");
    UtAssert_StrCmp(FileHeader.object_name, "Test App", "FileHeader.object_name == \"Test App\"");
    UtAssert_StrCmp(FileHeader.entry_point_name, "Test Main", "FileHeader.entry_point_name == \"Test Main\"");

    /* Verify Data is copied from the file to the correct memory locations */
    UtAssert_Mem2BinFileCmp(TestCodeBuffer, "test_uncompressed_code.dat", "Uncompressed Code Matches test_uncompressed_code.dat");
    UtAssert_Mem2BinFileCmp(TestDataBuffer, "test_uncompressed_data.dat", "Uncompressed Data Matches test_uncompressed_data.dat");
    UtAssert_MemCmpValue(TestBSSBuffer,  0, TEST_BSS_BUFFER_SIZE,  "BSS Memory is all 0's");

    FreeStaticLoadFile("staticload.slf");
}

/* LoadStaticLoadFile_Test12 - Error Uncompresseing Code */
void LoadStaticLoadFile_Test12(void)
{
    static_load_file_header_t   FileHeader;
    boolean                     ReturnStatus;

    /* Setup Inputs */
    CreateCompressedStaticLoadFile("staticload.slf", "test_compressed_code.dat", TEST_COMPRESSED_CODE_SIZE-1, TEST_UNCOMPRESSED_CODE_SIZE,
                                   "test_compressed_data.dat", TEST_COMPRESSED_DATA_SIZE, TEST_UNCOMPRESSED_DATA_SIZE, TEST_BSS_BUFFER_SIZE);
    memset(&FileHeader, 0, sizeof(static_load_file_header_t));

    /* Execute Test */
    ReturnStatus = LoadStaticLoadFile("staticload.slf", &FileHeader);

    /* Verify Outputs */
    UtAssert_True(ReturnStatus == FALSE, "ReturnStatus == FALSE");

    FreeStaticLoadFile("staticload.slf");
}

/* LoadStaticLoadFile_Test13 - Error Uncompresseing Data */
void LoadStaticLoadFile_Test13(void)
{
    static_load_file_header_t   FileHeader;
    boolean                     ReturnStatus;

    /* Setup Inputs */
    CreateCompressedStaticLoadFile("staticload.slf", "test_compressed_code.dat", TEST_COMPRESSED_CODE_SIZE, TEST_UNCOMPRESSED_CODE_SIZE,
                                   "test_compressed_data.dat", TEST_COMPRESSED_DATA_SIZE-1, TEST_UNCOMPRESSED_DATA_SIZE, TEST_BSS_BUFFER_SIZE);
    memset(&FileHeader, 0, sizeof(static_load_file_header_t));

    /* Execute Test */
    ReturnStatus = LoadStaticLoadFile("staticload.slf", &FileHeader);

    /* Verify Outputs */
    UtAssert_True(ReturnStatus == FALSE, "ReturnStatus == FALSE");

    FreeStaticLoadFile("staticload.slf");
}

/* LoadStaticLoadFile_Test14 - Error Reading Code Segment */
void LoadStaticLoadFile_Test14(void)
{
    static_load_file_header_t   FileHeader;
    FILE                       *FilePointer;
    boolean                     ReturnStatus;

    /* Setup Inputs */
    memset(&FileHeader, 0, sizeof(static_load_file_header_t));
    FileHeader.file_marker = STATIC_FILE_MARKER;
    FileHeader.entry_point = 0x12345678;
    FileHeader.flags = STATIC_FILE_UNCOMPRESSED;
    FileHeader.code_target = 0x12345678;
    FileHeader.code_size = 10;
    FileHeader.code_offset = sizeof(static_load_file_header_t);
    FilePointer = fopen("staticload.slf", "w");
    fwrite(&FileHeader, sizeof(static_load_file_header_t), 1, FilePointer);
    fclose(FilePointer);

    /* Execute Test */
    ReturnStatus = LoadStaticLoadFile("staticload.slf", &FileHeader);

    /* Verify Outputs */
    UtAssert_True(ReturnStatus == FALSE, "ReturnStatus == FALSE");
    UtAssert_MemCmpValue(&FileHeader, 0, sizeof(static_load_file_header_t),  "File Header is all 0's");

    FreeStaticLoadFile("staticload.slf");
}

/* LoadStaticLoadFile_Test15 - Error Reading Data Segment */
void LoadStaticLoadFile_Test15(void)
{
    static_load_file_header_t   FileHeader;
    FILE                       *FilePointer;
    boolean                     ReturnStatus;

    /* Setup Inputs */
    memset(&FileHeader, 0, sizeof(static_load_file_header_t));
    FileHeader.file_marker = STATIC_FILE_MARKER;
    FileHeader.entry_point = 0x12345678;
    FileHeader.flags = STATIC_FILE_UNCOMPRESSED;
    FileHeader.code_offset = sizeof(static_load_file_header_t);
    FileHeader.data_target = 0x12345678;
    FileHeader.data_size = 10;
    FileHeader.data_offset = sizeof(static_load_file_header_t) + FileHeader.code_size;
    FilePointer = fopen("staticload.slf", "w");
    fwrite(&FileHeader, sizeof(static_load_file_header_t), 1, FilePointer);
    fclose(FilePointer);

    /* Execute Test */
    ReturnStatus = LoadStaticLoadFile("staticload.slf", &FileHeader);

    /* Verify Outputs */
    UtAssert_True(ReturnStatus == FALSE, "ReturnStatus == FALSE");
    UtAssert_MemCmpValue(&FileHeader, 0, sizeof(static_load_file_header_t),  "File Header is all 0's");

    FreeStaticLoadFile("staticload.slf");
}

/* Lzma_Read_Test01 - Successful Uncompress Segment */
void Lzma_Read_Test01(void)
{
    int             FileDescriptor;
    void           *TargetMemory;
    int             ReturnStatus;

    /* Setup Inputs */
    FileDescriptor = open("test_compressed_code.dat", O_RDONLY, 0);
    TargetMemory = malloc(TEST_UNCOMPRESSED_CODE_SIZE);

    /* Execute Test */
    ReturnStatus = Lzma_Read(FileDescriptor, TargetMemory, TEST_COMPRESSED_CODE_SIZE);

    /* Verify Outputs */
    UtAssert_True(ReturnStatus == TEST_COMPRESSED_CODE_SIZE, "ReturnStatus == TEST_COMPRESSED_CODE_SIZE");
    UtAssert_Mem2BinFileCmp(TargetMemory, "test_uncompressed_code.dat", "Uncompressed Code Matches test_uncompressed_code.dat");

    free(TargetMemory);
    close(FileDescriptor);
}

/* Lzma_Read_Test02 - Error FileSize = 0 */
void Lzma_Read_Test02(void)
{
    int             FileDescriptor;
    void           *TargetMemory;
    int             ReturnStatus;

    /* Setup Inputs */
    FileDescriptor = open("test_compressed_code.dat", O_RDONLY, 0);
    TargetMemory = malloc(TEST_UNCOMPRESSED_CODE_SIZE);

    /* Execute Test */
    ReturnStatus = Lzma_Read(FileDescriptor, TargetMemory, 0);

    /* Verify Outputs */
    UtAssert_True(ReturnStatus == 0, "ReturnStatus == 0");

    free(TargetMemory);
    close(FileDescriptor);
}

/* Lzma_Read_Test03 - File Read Error */
void Lzma_Read_Test03(void)
{
    int             FileDescriptor;
    void           *TargetMemory;
    int             ReturnStatus;

    /* Setup Inputs */
    FileDescriptor = open("test_compressed_code.dat", O_RDONLY, 0);
    TargetMemory = malloc(TEST_UNCOMPRESSED_CODE_SIZE);

    /* Execute Test */
    ReturnStatus = Lzma_Read(FileDescriptor, TargetMemory, TEST_COMPRESSED_CODE_SIZE+1);

    /* Verify Outputs */
    UtAssert_True(ReturnStatus == -1, "ReturnStatus == -1");

    free(TargetMemory);
    close(FileDescriptor);
}

/* Lzma_Read_Test04 - Uncompression Error */
void Lzma_Read_Test04(void)
{
    int             FileDescriptor;
    void           *TargetMemory;
    int             ReturnStatus;

    /* Setup Inputs */
    FileDescriptor = open("test_compressed_code.dat", O_RDONLY, 0);
    TargetMemory = malloc(TEST_UNCOMPRESSED_CODE_SIZE);

    /* Execute Test */
    ReturnStatus = Lzma_Read(FileDescriptor, TargetMemory, TEST_COMPRESSED_CODE_SIZE-1);

    /* Verify Outputs */
    UtAssert_True(ReturnStatus == -1, "ReturnStatus == -1");

    free(TargetMemory);
    close(FileDescriptor);
}

void LoadStaticLoadFile_Setup(void)
{

}

void LoadStaticLoadFile_TearDown(void)
{

}

/* I put AddTestCase last in the file so I don't have to declare prorotypes for each test method */
void LoadStaticLoadFile_AddTestCase(void)
{
    UtTest_Add(LoadStaticLoadFile_Test01, &LoadStaticLoadFile_Setup, &LoadStaticLoadFile_TearDown, "LoadStaticLoadFile_Test01 - Success");
    UtTest_Add(LoadStaticLoadFile_Test02, &LoadStaticLoadFile_Setup, &LoadStaticLoadFile_TearDown, "LoadStaticLoadFile_Test02 - File Not Found");
    UtTest_Add(LoadStaticLoadFile_Test03, &LoadStaticLoadFile_Setup, &LoadStaticLoadFile_TearDown, "LoadStaticLoadFile_Test03 - Error Reading File Header");
    UtTest_Add(LoadStaticLoadFile_Test04, &LoadStaticLoadFile_Setup, &LoadStaticLoadFile_TearDown, "LoadStaticLoadFile_Test04 - Not a Static Load File");
    UtTest_Add(LoadStaticLoadFile_Test05, &LoadStaticLoadFile_Setup, &LoadStaticLoadFile_TearDown, "LoadStaticLoadFile_Test05 - Filename is NULL");
    UtTest_Add(LoadStaticLoadFile_Test06, &LoadStaticLoadFile_Setup, &LoadStaticLoadFile_TearDown, "LoadStaticLoadFile_Test06 - FileHeader is NULL");
    UtTest_Add(LoadStaticLoadFile_Test07, &LoadStaticLoadFile_Setup, &LoadStaticLoadFile_TearDown, "LoadStaticLoadFile_Test07 - No Code Segment");
    UtTest_Add(LoadStaticLoadFile_Test08, &LoadStaticLoadFile_Setup, &LoadStaticLoadFile_TearDown, "LoadStaticLoadFile_Test08 - No Data Segment");
    UtTest_Add(LoadStaticLoadFile_Test09, &LoadStaticLoadFile_Setup, &LoadStaticLoadFile_TearDown, "LoadStaticLoadFile_Test09 - No BSS Segment");
    UtTest_Add(LoadStaticLoadFile_Test11, &LoadStaticLoadFile_Setup, &LoadStaticLoadFile_TearDown, "LoadStaticLoadFile_Test11 - Success Uncompression");
    UtTest_Add(LoadStaticLoadFile_Test12, &LoadStaticLoadFile_Setup, &LoadStaticLoadFile_TearDown, "LoadStaticLoadFile_Test12 - Error Uncompresseing Code");
    UtTest_Add(LoadStaticLoadFile_Test13, &LoadStaticLoadFile_Setup, &LoadStaticLoadFile_TearDown, "LoadStaticLoadFile_Test13 - Error Uncompresseing Data");
    UtTest_Add(LoadStaticLoadFile_Test14, &LoadStaticLoadFile_Setup, &LoadStaticLoadFile_TearDown, "LoadStaticLoadFile_Test14 - Error Reading Code Segment");
    UtTest_Add(LoadStaticLoadFile_Test15, &LoadStaticLoadFile_Setup, &LoadStaticLoadFile_TearDown, "LoadStaticLoadFile_Test15 - Error Reading Data Segment");
    UtTest_Add(Lzma_Read_Test01, &LoadStaticLoadFile_Setup, &LoadStaticLoadFile_TearDown, "Lzma_Read_Test01 - Successful Uncompress Segment");
    UtTest_Add(Lzma_Read_Test02, &LoadStaticLoadFile_Setup, &LoadStaticLoadFile_TearDown, "Lzma_Read_Test02 - Error FileSize = 0");
    UtTest_Add(Lzma_Read_Test03, &LoadStaticLoadFile_Setup, &LoadStaticLoadFile_TearDown, "Lzma_Read_Test03 - File Read Error");
    UtTest_Add(Lzma_Read_Test04, &LoadStaticLoadFile_Setup, &LoadStaticLoadFile_TearDown, "Lzma_Read_Test04 - Uncompression Error");
}
