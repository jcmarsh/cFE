
/*
 * Filename: elffile.h
 *
 * Purpose: This file contains functions to extract Sections and Symbols from ELF files
 *
 * Design Notes:
 *   This file contains functions to extract Sections and Symbols from ELF files.  This code should be used as follows:
 *     1. Call ELF_OpenFile to open the ELF file and read the File Header, the Section Header Table, and the Symbol Table.  You
 *        can only have one ELF file open at a time.
 *     2. Call one or more of the following functions to extract the desired data from the ELF file:
 *         ELF_GetEntryPoint
 *         ELF_ReadSectionByName
 *         ELF_SectionExists
 *         ELF_ReadSymbolByValue
 *         ELF_ReadSymbolByName
 *         ELF_SymbolExists
 *     3. When you are finished processing the ELF data then free the sections and symbols that were previously read and
 *        close the ELF file.  You should free all sections and symbols before calling ELF_CloseFile.
 *         ELF_FreeSection
 *         ELF_FreeSymbol
 *         ELF_CloseFile
 *     4. Optional Tools:
 *         ELF_PrintFile
 *         ELF_ByteSwapRequired
 *         ELF_SwapUInt16
 *         ELF_SwapUInt32
 *
 * References:
 *
 */

#ifndef _ELFFILE_H
#define	_ELFFILE_H

/*
 * Includes
 */

#include "common_types.h"

/*
 * Type Definitions
 */

typedef struct {
    char           *SectionName;
    char            SectionType;
    void           *SectionData;
    void           *SectionDataAddress;
    uint32          SectionDataSize;
} ELF_Section_t;

typedef struct {
    char           *SymbolName;
    void           *SymbolData;
    void           *SymbolDataAddress;
    uint32          SymbolDataSize;
    char           *SectionName;
    char            SectionType;
} ELF_Symbol_t;

/*
 * Exported Functions
 */

/* This function opens the ELF file and reads the File Header, Section Header Table,
 * and the Symbol Table. */
void                ELF_OpenFile(char *Filename);

/* This function closes a ELF file opened with ELF_FileOpen and frees memory allocated for
 * the section header table and the symbol table */
void                ELF_CloseFile(void);

/* This function prints the contents of the ELF File Header, Section Header Table, and Symbol Table */
void                ELF_PrintFile(void);

/* This function returns the entry point address from the ELF File Header Table */
void               *ELF_GetEntryPoint(void);

/* This function searches the Section Header Table for the specified section name and if found returns a pointer to a
 * ELF_Section_t structure.  If the specified section name is not found then a NULL pointer is returned.  When you
 * are finished with the section data the ELF_Section_t structure should be freed by calling ELF_FreeSection */
ELF_Section_t      *ELF_ReadSectionByName(char *SectionName);

/* This function returns TRUE if the specified section exists in the Section Header Table and FALSE if the specified section
 * does not exist in the Section Header Table */
boolean             ELF_SectionExists(char *SectionName);

/* This function frees a section read by ELF_ReadSectionByName */
void                ELF_FreeSection(ELF_Section_t *Section);

/* This function searches the Symbol Table for the specified Symbol Value (Address) and if found returns a pointer to a
 * ELF_Symbol_t structure.  If the specified symbol is not found then a NULL pointer is returned.  When you are finished
 * with the symbol data the ELF_Symbol_t structure should be freed by calling ELF_FreeSymbol */
ELF_Symbol_t       *ELF_ReadSymbolByValue(void *Value);

/* This function searches the Symbol Table for the specified Symbol name and if found returns a pointer to a
 * ELF_Symbol_t structure.  If the specified symbol is not found then a NULL pointer is returned.  When you are finished
 * with the symbol data the ELF_Symbol_t structure should be freed by calling ELF_FreeSymbol */
ELF_Symbol_t       *ELF_ReadSymbolByName(char *SymbolName);

/* This function returns TRUE if the specified symbol exists in the Symbol Table and FALSE if the specified symbol
 * does not exist in the Symbol Table */
boolean             ELF_SymbolExists(char *SymbolName);

/* This function frees a symbol read by ELF_ReadSymbolByValue or ELF_ReadSymbolByName */
void                ELF_FreeSymbol(ELF_Symbol_t *Symbol);

/* This function prints the contents of the ELF File Header, Section Header Table, and Symbol Table */
void                ELF_PrintFile(void);

/* This function checks to see if the endianess of this machine and
 * the target machine are different, if so then some data structures will
 * require byte swapping */
boolean             ELF_ByteSwapRequired(void);

/* This function byte swaps a 16 bit integer */
void                ELF_SwapUInt16(uint16 *ValueToSwap);

/* This function byte swaps a 32 bit integer */
void                ELF_SwapUInt32(uint32 *ValueToSwap);

#endif

