
/*
 * Filename: elffile.c
 *
 * Purpose:  This file contains functions to extract Sections and Symbols from ELF files.
 *
 */

/*
 * Includes
 */

#include "common_types.h"
#include "elftypes.h"
#include "elffile.h"
#include "elf2staticload.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

/*
 * Type Definitions
 */

typedef struct {
    Elf32_Shdr     *SectionHeaderTableEntries;
    char   	   *StringTable;
    uint32	    NumberOfSections;
} ELF_SectionHeaderTable_t;

typedef struct {
    Elf32_Sym      *SymbolTableEntries;
    char   	   *StringTable;
    uint32          NumberOfSymbols;
} ELF_SymbolTable_t;

/*
 * Local Data
 */

FILE                       *ELF_FilePointer;
char                       *ELF_Filename;
Elf32_Ehdr                  ELF_FileHeader;
ELF_SectionHeaderTable_t    ELF_SectionHeaderTable;
ELF_SymbolTable_t	    ELF_SymbolTable;

/*
 * Local Function Prototypes
 */

void                        ELF_ReadFileHeader(FILE *ELF_FilePointer, Elf32_Ehdr *ELF_FileHeader);
void                        ELF_ReadSectionHeaderTable(FILE *ELF_FilePointer, Elf32_Ehdr *ELF_FileHeader, ELF_SectionHeaderTable_t *ELF_SectionHeaderTable);
void                        ELF_FreeSectionHeaderTable(ELF_SectionHeaderTable_t *ELF_SectionHeaderTable);
void                        ELF_ReadSymbolTable(FILE *ELF_FilePointer, ELF_SectionHeaderTable_t *ELF_SectionHeaderTable, ELF_SymbolTable_t *ELF_SymbolTable);
void                        ELF_FreeSymbolTable(ELF_SymbolTable_t *ELF_SymbolTable);
Elf32_Shdr                 *ELF_FindSectionHeaderByName(ELF_SectionHeaderTable_t *ELF_SectionHeaderTable, char *SectionName);
Elf32_Sym                  *ELF_FindSymbolByName(ELF_SymbolTable_t *ELF_SymbolTable, char *SymbolName);
Elf32_Sym                  *ELF_FindSymbolByValue(ELF_SymbolTable_t *ELF_SymbolTable, void *SymbolValue);
ELF_Symbol_t               *ELF_ReadSymbol(Elf32_Sym *SymbolTableEntry);
uint32                      ELF_ThisMachineDataEncoding(void);
void                        ELF_ByteSwapFileHeader(Elf32_Ehdr *ELF_FileHeader);
void                        ELF_ByteSwapSectionHeader(Elf32_Shdr *SectionHeader);
void                        ELF_ByteSwapSymbol(Elf32_Sym *Symbol);

/*
 * Function Definitions
 */

/* This function opens the ELF file and reads the File Header, Section Header Table,
 * and the Symbol Table. */
void ELF_OpenFile(char *Filename)
{

    ELF_FilePointer = fopen(Filename, "r");
    if (ELF_FilePointer) {

        ELF_Filename = Filename;
        ELF_ReadFileHeader(ELF_FilePointer, &ELF_FileHeader);
        ELF_ReadSectionHeaderTable(ELF_FilePointer, &ELF_FileHeader, &ELF_SectionHeaderTable);
        ELF_ReadSymbolTable(ELF_FilePointer, &ELF_SectionHeaderTable, &ELF_SymbolTable);
    }
    else {

        UglyExit("Error Opening ELF File: %s, %s\n", Filename, strerror(errno));
    }
}

/* This function closes a ELF file opened with ELF_FileOpen and frees memory allocated for
 * the section header table and the symbol table */
void ELF_CloseFile(void)
{
    ELF_Filename = NULL;
    memset(&ELF_FileHeader, 0, sizeof(Elf32_Ehdr));
    ELF_FreeSectionHeaderTable(&ELF_SectionHeaderTable);
    ELF_FreeSymbolTable(&ELF_SymbolTable);
    fclose(ELF_FilePointer);
}

/* This function reads the ELF File Header Table and does some minimal validation */
void ELF_ReadFileHeader(FILE *ELF_FilePointer, Elf32_Ehdr *ELF_FileHeader)
{
    fread(ELF_FileHeader, sizeof(Elf32_Ehdr), 1, ELF_FilePointer);

    /* verify the ELF magic number */
    if ((ELF_FileHeader->e_ident[EI_MAG0] != ELFMAG0) ||
        (ELF_FileHeader->e_ident[EI_MAG1] != ELFMAG1) ||
        (ELF_FileHeader->e_ident[EI_MAG2] != ELFMAG2) ||
        (ELF_FileHeader->e_ident[EI_MAG3] != ELFMAG3)) {

        UglyExit("Error: Invalid ELF Magic Number\n");
    }

    /* Verify the class */
    if (ELF_FileHeader->e_ident[EI_CLASS] != ELFCLASS32) {

        UglyExit("Error: Unsupported ELF Class\n");
    }

    /* Verify the version number */
    if (ELF_FileHeader->e_ident[EI_VERSION] != EV_CURRENT) {

        UglyExit("Error: Unsupported ELF Version Number\n");
    }

    if (ELF_ByteSwapRequired()) {

        ELF_ByteSwapFileHeader(ELF_FileHeader);
    }
}

/* This function returns the entry point address from the ELF File Header Table */
void *ELF_GetEntryPoint(void)
{
    return(ELF_FileHeader.e_entry);
}

/* This function reads the ELF Section Header table and it's associated String Table */
void ELF_ReadSectionHeaderTable(FILE *ELF_FilePointer, Elf32_Ehdr *ELF_FileHeader, ELF_SectionHeaderTable_t *ELF_SectionHeaderTable)
{
    uint32      i;
    Elf32_Shdr *StringTableSectionHeader = NULL;

    /* Read the Section Header Table */
    ELF_SectionHeaderTable->NumberOfSections = ELF_FileHeader->e_shnum;
    ELF_SectionHeaderTable->SectionHeaderTableEntries = malloc(sizeof(Elf32_Shdr) * ELF_SectionHeaderTable->NumberOfSections);
    fseek(ELF_FilePointer, ELF_FileHeader->e_shoff, SEEK_SET);
    fread(ELF_SectionHeaderTable->SectionHeaderTableEntries, sizeof(Elf32_Shdr), ELF_SectionHeaderTable->NumberOfSections, ELF_FilePointer);

    if (ELF_ByteSwapRequired()) {
        for (i=1; i < ELF_SectionHeaderTable->NumberOfSections; i++) {
            ELF_ByteSwapSectionHeader(&ELF_SectionHeaderTable->SectionHeaderTableEntries[i]);
        }
    }

    /* Now Read the String table */
    StringTableSectionHeader = &ELF_SectionHeaderTable->SectionHeaderTableEntries[ELF_FileHeader->e_shstrndx];
    ELF_SectionHeaderTable->StringTable = malloc(StringTableSectionHeader->sh_size);
    fseek(ELF_FilePointer, StringTableSectionHeader->sh_offset, SEEK_SET);
    fread(ELF_SectionHeaderTable->StringTable, StringTableSectionHeader->sh_size, 1, ELF_FilePointer);
}

/* This function frees the ELF Section Header Table and it's associated String Table */
void ELF_FreeSectionHeaderTable(ELF_SectionHeaderTable_t *ELF_SectionHeaderTable)
{
    if (ELF_SectionHeaderTable != NULL) {
        free(ELF_SectionHeaderTable->SectionHeaderTableEntries);
        ELF_SectionHeaderTable->SectionHeaderTableEntries = NULL;
        free(ELF_SectionHeaderTable->StringTable);
        ELF_SectionHeaderTable->StringTable = NULL;
        ELF_SectionHeaderTable->NumberOfSections = 0;
    }
}

/* This function reads the ELF Symbol Table and it's associated String Table */
void ELF_ReadSymbolTable(FILE *ELF_FilePointer, ELF_SectionHeaderTable_t *ELF_SectionHeaderTable, ELF_SymbolTable_t *ELF_SymbolTable)
{
    uint32      i;
    Elf32_Shdr *SymbolTableSectionHeader = NULL;
    Elf32_Shdr *StringTableSectionHeader = NULL;

    /* Search the Section Header Table looking for the Symbol Table */
    SymbolTableSectionHeader = ELF_FindSectionHeaderByName(ELF_SectionHeaderTable, ".symtab");
    if (SymbolTableSectionHeader != NULL) {

        /* Read the Symbol Table */
        ELF_SymbolTable->NumberOfSymbols = (SymbolTableSectionHeader->sh_size / SymbolTableSectionHeader->sh_entsize);
        ELF_SymbolTable->SymbolTableEntries = malloc(SymbolTableSectionHeader->sh_size);
        fseek(ELF_FilePointer, SymbolTableSectionHeader->sh_offset, SEEK_SET);
        fread(ELF_SymbolTable->SymbolTableEntries, SymbolTableSectionHeader->sh_size, 1, ELF_FilePointer);

        if (ELF_ByteSwapRequired()) {
            for (i=1; i < ELF_SymbolTable->NumberOfSymbols; i++) {
                ELF_ByteSwapSymbol(&ELF_SymbolTable->SymbolTableEntries[i]);
            }
        }

        /* Now Read the String table */
        StringTableSectionHeader = &ELF_SectionHeaderTable->SectionHeaderTableEntries[SymbolTableSectionHeader->sh_link];
        ELF_SymbolTable->StringTable = malloc(StringTableSectionHeader->sh_size);
        fseek(ELF_FilePointer, StringTableSectionHeader->sh_offset, SEEK_SET);
        fread(ELF_SymbolTable->StringTable, StringTableSectionHeader->sh_size, 1, ELF_FilePointer);
    }
    else {

        UglyExit("Error: Can't Find Symbol Table\n");
    }
}

/* This function frees the ELF Symbol Table and it's associated String Table */
void ELF_FreeSymbolTable(ELF_SymbolTable_t *ELF_SymbolTable)
{
    if (ELF_SymbolTable != NULL) {
        free(ELF_SymbolTable->SymbolTableEntries);
        ELF_SymbolTable->SymbolTableEntries = NULL;
        free(ELF_SymbolTable->StringTable);
        ELF_SymbolTable->StringTable = NULL;
        ELF_SymbolTable->NumberOfSymbols = 0;
    }
}

/* This function searches the Section Header Table for the specified section name and if found returns a pointer to a
 * ELF_Section_t structure.  If the specified section name is not found then a NULL pointer is returned.  When you
 * are finished with the section data the ELF_Section_t structure should be freed by calling ELF_FreeSection */
ELF_Section_t *ELF_ReadSectionByName(char *SectionName)
{

    Elf32_Shdr    *SectionHeaderTableEntry = NULL;
    ELF_Section_t *NewSection = NULL;

    /* Search the Section Header Table for the specified Section Name */
    SectionHeaderTableEntry = ELF_FindSectionHeaderByName(&ELF_SectionHeaderTable, SectionName);
    if (SectionHeaderTableEntry != NULL) {

        NewSection = malloc(sizeof(ELF_Section_t));
        NewSection->SectionName = SectionName;
        NewSection->SectionType = SectionHeaderTableEntry->sh_type;
        NewSection->SectionDataAddress = SectionHeaderTableEntry->sh_addr;
        NewSection->SectionDataSize = SectionHeaderTableEntry->sh_size;

        /* Make sure the Section has Data associated with it.  A section with a type of SHT_NOBITS does not
         * occupy any space in the ELF file, which is true for the .bss Section. */
        if ((SectionHeaderTableEntry->sh_size > 0) && (SectionHeaderTableEntry->sh_type != SHT_NOBITS)) {

            NewSection->SectionData = malloc(SectionHeaderTableEntry->sh_size);
            fseek(ELF_FilePointer, SectionHeaderTableEntry->sh_offset, SEEK_SET);
            fread(NewSection->SectionData, SectionHeaderTableEntry->sh_size, 1, ELF_FilePointer);
        }
        else {
            NewSection->SectionData = NULL;
        }

        return(NewSection);
    }

    return(NULL);
}

/* This function frees a section read by ELF_ReadSectionByName */
void ELF_FreeSection(ELF_Section_t *Section)
{
    if (Section != NULL) {
        free(Section->SectionData);
        free(Section);
    }
}

/* This function searches the Symbol Table for the specified Symbol Value (Address) and if found returns a pointer to a
 * ELF_Symbol_t structure.  If the specified symbol is not found then a NULL pointer is returned.  When you are finished
 * with the symbol data the ELF_Symbol_t structure should be freed by calling ELF_FreeSymbol */
ELF_Symbol_t *ELF_ReadSymbolByValue(void *SymbolValue)
{

    Elf32_Sym               *SymbolTableEntry;

    /* Search the Symbol Table for the specified entry */
    SymbolTableEntry = ELF_FindSymbolByValue(&ELF_SymbolTable, SymbolValue);
    if (SymbolTableEntry != NULL) {

        return(ELF_ReadSymbol(SymbolTableEntry));
    }

    return(NULL);
}

/* This function searches the Symbol Table for the specified Symbol name and if found returns a pointer to a
 * ELF_Symbol_t structure.  If the specified symbol is not found then a NULL pointer is returned.  When you are finished
 * with the symbol data the ELF_Symbol_t structure should be freed by calling ELF_FreeSymbol */
ELF_Symbol_t *ELF_ReadSymbolByName(char *SymbolName)
{

    Elf32_Sym               *SymbolTableEntry;

    SymbolTableEntry = ELF_FindSymbolByName(&ELF_SymbolTable, SymbolName);
    if (SymbolTableEntry != NULL) {

        return(ELF_ReadSymbol(SymbolTableEntry));
    }

    return(NULL);
}

/* This function reads the specified symbol and returns a pointer to a ELF_Symbol_t structure. */
ELF_Symbol_t *ELF_ReadSymbol(Elf32_Sym *SymbolTableEntry)
{
    ELF_Symbol_t            *NewSymbol;

    NewSymbol = malloc(sizeof(ELF_Symbol_t));
    NewSymbol->SymbolName = &ELF_SymbolTable.StringTable[SymbolTableEntry->st_name];
    NewSymbol->SymbolDataAddress = SymbolTableEntry->st_value;
    NewSymbol->SymbolDataSize = SymbolTableEntry->st_size;

    /* Make sure the Section Header Index points to a Section in the Section Header Table.  Some Section Header
     * Index values have alternate meanings and do not point to a Section in the Section Header Table. */
    if ((SymbolTableEntry->st_shndx != SHN_UNDEF) &&
        (SymbolTableEntry->st_shndx != SHN_LORESERVE) &&
        (SymbolTableEntry->st_shndx != SHN_LOPROC) &&
        (SymbolTableEntry->st_shndx != SHN_HIPROC) &&
        (SymbolTableEntry->st_shndx != SHN_ABS) &&
        (SymbolTableEntry->st_shndx != SHN_COMMON) &&
        (SymbolTableEntry->st_shndx != SHN_HIRESERVE)) {

        NewSymbol->SectionName = &ELF_SectionHeaderTable.StringTable[(&ELF_SectionHeaderTable.SectionHeaderTableEntries[SymbolTableEntry->st_shndx])->sh_name];
        NewSymbol->SectionType = (&ELF_SectionHeaderTable.SectionHeaderTableEntries[SymbolTableEntry->st_shndx])->sh_type;

        /* Make sure the Symbol has Data associated with it.  A Symbol contained inside a Section with a type of SHT_NOBITS does not occupy any space
         * in the ELF file, which is true for the .bss Section. */
        if ((SymbolTableEntry->st_size > 0) && (((&ELF_SectionHeaderTable.SectionHeaderTableEntries[SymbolTableEntry->st_shndx])->sh_type != SHT_NOBITS))) {

            NewSymbol->SymbolData = malloc(SymbolTableEntry->st_size);

            /* The Symbol Table does not directly tell me where in the ELF file to find the Symbol Data.  So to find the location of the Symbol Data in
             * the ELF file I first have to find the offset of the Symbol within the Section that contains it.  Each Symbol in the Symbol Table
             * contains the index of the Section that the Symbol belongs to.  Since I know which Section the Symbol belongs to I can calculate the
             * relative offset of the Symbol within the Section by subtracting the Address of the Symbol from the starting Address of the Section.  Then to
             * find the offset of the Symbol within the ELF file I add the relative offset of the Symbol within the Section to the starting offset of
             * the Section in the ELF File. */
            fseek(ELF_FilePointer, ((SymbolTableEntry->st_value - (&ELF_SectionHeaderTable.SectionHeaderTableEntries[SymbolTableEntry->st_shndx])->sh_addr) + (&ELF_SectionHeaderTable.SectionHeaderTableEntries[SymbolTableEntry->st_shndx])->sh_offset), SEEK_SET);
            fread(NewSymbol->SymbolData, SymbolTableEntry->st_size, 1, ELF_FilePointer);
        }
        else {
            NewSymbol->SymbolData = NULL;
        }
    }
    else {

        /* This Symbol does not have a associated Section in the Section Header Table. */
        NewSymbol->SectionName = "\0";
        NewSymbol->SectionType = 0;
        NewSymbol->SymbolData = NULL;
    }

    return(NewSymbol);
}

/* This function frees a symbol read by ELF_ReadSymbolByValue or ELF_ReadSymbolByName */
void ELF_FreeSymbol(ELF_Symbol_t *Symbol)
{
    if (Symbol != NULL) {
        free(Symbol->SymbolData);
        free(Symbol);
    }
}

/* This function searches the Symbol Table looking for the specified symbol Value (Address).  If the
 * symbol is found then a pointer is returned to the found entry in the Symbol Table, otherwise a
 * NULL pointer is returned */
Elf32_Sym *ELF_FindSymbolByValue(ELF_SymbolTable_t *ELF_SymbolTable, void *SymbolValue)
{
    uint32 i;

    for (i=1; i < ELF_SymbolTable->NumberOfSymbols; i++) {
        if (((&ELF_SymbolTable->SymbolTableEntries[i])->st_value == SymbolValue) &&
          ((((&ELF_SymbolTable->SymbolTableEntries[i])->st_info & 0xF) == STT_NOTYPE) ||
           (((&ELF_SymbolTable->SymbolTableEntries[i])->st_info & 0xF) == STT_OBJECT) ||
           (((&ELF_SymbolTable->SymbolTableEntries[i])->st_info & 0xF) == STT_FUNC))) {
            return(&ELF_SymbolTable->SymbolTableEntries[i]);
        }
    }
    return(NULL);
}

/* This function searches the Symbol Table looking for the specified symbol name.  If the
 * symbol is found then a pointer is returned to the found entry in the Symbol Table, otherwise a
 * NULL pointer is returned */
Elf32_Sym *ELF_FindSymbolByName(ELF_SymbolTable_t *ELF_SymbolTable, char *SymbolName)
{
    uint32 i;

    for (i=1; i < ELF_SymbolTable->NumberOfSymbols; i++) {
        if (strcmp(&ELF_SymbolTable->StringTable[(&ELF_SymbolTable->SymbolTableEntries[i])->st_name], SymbolName) == 0) {
            return(&ELF_SymbolTable->SymbolTableEntries[i]);
        }
    }
    return(NULL);
}

/* This function returns TRUE if the specified symbol exists in the Symbol Table and FALSE if the specified symbol
 * does not exist in the Symbol Table */
boolean ELF_SymbolExists(char *SymbolName)
{
    return(ELF_FindSymbolByName(&ELF_SymbolTable, SymbolName) != NULL);
}

/* This function searches the Section Header Table looking for the specified section name.  If the
 * symbol is found then a pointer is returned to the found entry in the Section Header Table, otherwise a
 * NULL pointer is returned */
Elf32_Shdr *ELF_FindSectionHeaderByName(ELF_SectionHeaderTable_t *ELF_SectionHeaderTable, char *SectionName)
{
    uint32 i;

    for (i=1; i < ELF_SectionHeaderTable->NumberOfSections; i++) {
        if (strcmp(&ELF_SectionHeaderTable->StringTable[(&ELF_SectionHeaderTable->SectionHeaderTableEntries[i])->sh_name], SectionName) == 0) {
            return(&ELF_SectionHeaderTable->SectionHeaderTableEntries[i]);
        }
    }
    return(NULL);
}

/* This function returns TRUE if the specified section exists in the Section Header Table and FALSE if the specified section
 * does not exist in the Section Header Table */
boolean ELF_SectionExists(char *SectionName)
{
    return(ELF_FindSectionHeaderByName(&ELF_SectionHeaderTable, SectionName) != NULL);
}

/* This function prints the contents of the ELF File Header, Section Header Table, and Symbol Table */
void ELF_PrintFile(void)
{
    uint32     i;

    printf("\nELF Filename: %s\n", ELF_Filename);
    printf("ELF Header:\n");
    printf("   e_ident[EI_MAG0..3]    = 0x%02x,%c%c%c\n",  ELF_FileHeader.e_ident[EI_MAG0], ELF_FileHeader.e_ident[EI_MAG1],
                                                           ELF_FileHeader.e_ident[EI_MAG2], ELF_FileHeader.e_ident[EI_MAG3]);
    printf("   e_ident[EI_CLASS]      = %d\n",             ELF_FileHeader.e_ident[EI_CLASS]);
    printf("   e_ident[EI_DATA]       = %d\n",             ELF_FileHeader.e_ident[EI_DATA]) ;
    printf("   e_ident[EI_VERSION]    = %d\n",             ELF_FileHeader.e_ident[EI_VERSION]);
    printf("   e_ident[EI_PAD]        = %d\n",             ELF_FileHeader.e_ident[EI_PAD]);
    printf("   e_type                 = %d\n",             ELF_FileHeader.e_type);
    printf("   e_machine              = %d\n",             ELF_FileHeader.e_machine);
    printf("   e_version              = %d\n",             ELF_FileHeader.e_version);
    printf("   e_entry                = 0x%08lx\n",(uint32)ELF_FileHeader.e_entry);
    printf("   e_phoff                = %d\n",             ELF_FileHeader.e_phoff);
    printf("   e_shoff                = %d\n",             ELF_FileHeader.e_shoff);
    printf("   e_flags                = 0x%08x\n",         ELF_FileHeader.e_flags);
    printf("   e_ehsize               = %d\n",             ELF_FileHeader.e_ehsize);
    printf("   e_phentsize            = %d\n",             ELF_FileHeader.e_phentsize);
    printf("   e_phnum                = %d\n",             ELF_FileHeader.e_phnum);
    printf("   e_shentsize            = %d\n",             ELF_FileHeader.e_shentsize);
    printf("   e_shnum                = %d\n",             ELF_FileHeader.e_shnum);
    printf("   e_shstrndx             = %d\n",             ELF_FileHeader.e_shstrndx);

    for (i=1; i < ELF_SectionHeaderTable.NumberOfSections; i++) {

        printf("Section Header #%lu:\n", i);
        printf("   sh_name                = 0x%08x - %s\n",    (&ELF_SectionHeaderTable.SectionHeaderTableEntries[i])->sh_name, &ELF_SectionHeaderTable.StringTable[(&ELF_SectionHeaderTable.SectionHeaderTableEntries[i])->sh_name]);
        printf("   sh_type                = 0x%08x\n",         (&ELF_SectionHeaderTable.SectionHeaderTableEntries[i])->sh_type);
        printf("   sh_flags               = 0x%08x\n",         (&ELF_SectionHeaderTable.SectionHeaderTableEntries[i])->sh_flags);
        printf("   sh_addr                = 0x%08lx\n",(uint32)(&ELF_SectionHeaderTable.SectionHeaderTableEntries[i])->sh_addr);
        printf("   sh_offset              = 0x%08x\n",         (&ELF_SectionHeaderTable.SectionHeaderTableEntries[i])->sh_offset);
        printf("   sh_size                = 0x%08x\n",         (&ELF_SectionHeaderTable.SectionHeaderTableEntries[i])->sh_size);
        printf("   sh_link                = 0x%08x\n",         (&ELF_SectionHeaderTable.SectionHeaderTableEntries[i])->sh_link);
        printf("   sh_info                = 0x%08x\n",         (&ELF_SectionHeaderTable.SectionHeaderTableEntries[i])->sh_info);
        printf("   sh_addralign           = 0x%08x\n",         (&ELF_SectionHeaderTable.SectionHeaderTableEntries[i])->sh_addralign);
        printf("   sh_entsize             = 0x%08x\n",         (&ELF_SectionHeaderTable.SectionHeaderTableEntries[i])->sh_entsize);

    }

    for (i=1; i < ELF_SymbolTable.NumberOfSymbols; i++) {
        printf("Symbol #%lu:\n", i);
        printf("   st_name                = 0x%08x - %s\n",    (&ELF_SymbolTable.SymbolTableEntries[i])->st_name, (&ELF_SymbolTable.StringTable[(&ELF_SymbolTable.SymbolTableEntries[i])->st_name]));
        printf("   st_value               = 0x%08lx\n",(uint32)(&ELF_SymbolTable.SymbolTableEntries[i])->st_value);
        printf("   st_size                = 0x%08x\n",         (&ELF_SymbolTable.SymbolTableEntries[i])->st_size);
        printf("   st_info                = 0x%02x\n",         (&ELF_SymbolTable.SymbolTableEntries[i])->st_info);
        printf("   st_other               = 0x%02x\n",         (&ELF_SymbolTable.SymbolTableEntries[i])->st_other);
        printf("   st_shndx               = 0x%04x\n",         (&ELF_SymbolTable.SymbolTableEntries[i])->st_shndx);
    }
}

/* This function byte swaps the ELF File Header */
void ELF_ByteSwapFileHeader(Elf32_Ehdr *ELF_FileHeader)
{
    ELF_SwapUInt16((uint16 *)&ELF_FileHeader->e_type);
    ELF_SwapUInt16((uint16 *)&ELF_FileHeader->e_machine);
    ELF_SwapUInt32((uint32 *)&ELF_FileHeader->e_version);
    ELF_SwapUInt32((uint32 *)&ELF_FileHeader->e_entry);
    ELF_SwapUInt32((uint32 *)&ELF_FileHeader->e_phoff);
    ELF_SwapUInt32((uint32 *)&ELF_FileHeader->e_shoff);
    ELF_SwapUInt32((uint32 *)&ELF_FileHeader->e_flags);
    ELF_SwapUInt16((uint16 *)&ELF_FileHeader->e_ehsize);
    ELF_SwapUInt16((uint16 *)&ELF_FileHeader->e_phentsize);
    ELF_SwapUInt16((uint16 *)&ELF_FileHeader->e_phnum);
    ELF_SwapUInt16((uint16 *)&ELF_FileHeader->e_shentsize);
    ELF_SwapUInt16((uint16 *)&ELF_FileHeader->e_shnum);
    ELF_SwapUInt16((uint16 *)&ELF_FileHeader->e_shstrndx);
}

/* This function byte swaps a Section Header Table entry */
void ELF_ByteSwapSectionHeader(Elf32_Shdr *SectionHeader)
{
    ELF_SwapUInt32((uint32 *)&SectionHeader->sh_name);
    ELF_SwapUInt32((uint32 *)&SectionHeader->sh_type);
    ELF_SwapUInt32((uint32 *)&SectionHeader->sh_flags);
    ELF_SwapUInt32((uint32 *)&SectionHeader->sh_addr);
    ELF_SwapUInt32((uint32 *)&SectionHeader->sh_offset);
    ELF_SwapUInt32((uint32 *)&SectionHeader->sh_size);
    ELF_SwapUInt32((uint32 *)&SectionHeader->sh_link);
    ELF_SwapUInt32((uint32 *)&SectionHeader->sh_info);
    ELF_SwapUInt32((uint32 *)&SectionHeader->sh_addralign);
    ELF_SwapUInt32((uint32 *)&SectionHeader->sh_entsize);
}

/* This function byte swaps a Symbol Table entry */
void ELF_ByteSwapSymbol(Elf32_Sym *Symbol)
{
    ELF_SwapUInt32((uint32 *)&Symbol->st_name);
    ELF_SwapUInt32((uint32 *)&Symbol->st_value);
    ELF_SwapUInt32((uint32 *)&Symbol->st_size);
    ELF_SwapUInt16((uint16 *)&Symbol->st_shndx);
}

/* This function byte swaps a 16 bit integer */
void ELF_SwapUInt16(uint16 *ValueToSwap)
{
    uint8 *BytePtr = (uint8 *)ValueToSwap;
    uint8  TempByte = BytePtr[1];
    BytePtr[1] = BytePtr[0];
    BytePtr[0] = TempByte;
}

/* This function byte swaps a 32 bit integer */
void ELF_SwapUInt32(uint32 *ValueToSwap)
{
    uint8 *BytePtr = (uint8 *)ValueToSwap;
    uint8  TempByte = BytePtr[3];
    BytePtr[3] = BytePtr[0];
    BytePtr[0] = TempByte;
    TempByte   = BytePtr[2];
    BytePtr[2] = BytePtr[1];
    BytePtr[1] = TempByte;
}

/* This function checks to see if the endianess of this machine and
 * the target machine are different, if so then some data structures will
 * require byte swapping */
boolean ELF_ByteSwapRequired(void)
{
    if (ELF_FileHeader.e_ident[EI_DATA] != ELF_ThisMachineDataEncoding())
        return(TRUE);
    else
        return(FALSE);
}

/* This function examines the byte ordering of a 32 bit word to determine
 * if this machine is a Big Endian or Little Endian machine */
uint32 ELF_ThisMachineDataEncoding(void)
{
    uint32   DataEncodingCheck = 0x01020304;

    if (((uint8 *)&DataEncodingCheck)[0] == 0x04 &&
        ((uint8 *)&DataEncodingCheck)[1] == 0x03 &&
        ((uint8 *)&DataEncodingCheck)[2] == 0x02 &&
        ((uint8 *)&DataEncodingCheck)[3] == 0x01) {
        return(ELFDATA2LSB);  /* Little Endian */
    }
    else {
        return(ELFDATA2MSB);  /* Big Endian */
    }
}

