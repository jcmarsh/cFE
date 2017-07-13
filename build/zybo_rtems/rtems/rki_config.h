/*
** rki_config.h
**
**  Author:  Alan Cudmore
**
**  This contains the configuration settings for an RTEMS Application
**
*/

/*
** Basic configuration settings for the RTEMS Kernel Image ( RKI )
*/

/*
** Include the RTEMS shell and startup script?
*/
#define RKI_INCLUDE_SHELL

/*
** Define the shell init script to run
*/
#define RKI_SHELL_INIT "/shell-init"

/*
** Include the TAR file system to initialze IMFS
** This is used to setup the base directories and startup scripts
*/
#define RKI_INCLUDE_TARFS

/*
** Include the Static ELF module loader
*/
#define RKI_INCLUDE_STATIC_LOADER

/*
** Include support for the "regular" RAM disk
*/
#define RKI_INCLUDE_RAMDISK

/*
** Define RAM Disk 0 base address and size
*/
// TODO: Just throwing this to the 256MB mark to avoid conflicts.
// TODO: How big is a block? How much is needed?
#define RAM_DISK_0_BASE_ADDR 0x10000000
#define RAM_DISK_0_BLOCKS    512
