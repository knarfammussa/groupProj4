#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "libDisk.h" // Include the disk emulator library
#include "TinyFS_errno.h"

#define BLOCKSIZE 256
#define DEFAULT_DISK_SIZE 10240
#define BLOCK_COUNT (DEFAULT_DISK_SIZE / BLOCKSIZE)
#define DEFAULT_DISK_NAME "tinyFSDisk"
#define SUPERBLOCK_BLOCK_NUM 0

#define INODE_BLOCK_SIZE 1
#define INODE_SIZE sizeof(Inode)
#define INODES_PER_BLOCK (BLOCKSIZE / INODE_SIZE)

//macros for super block (which is at block 0)
#define _BLOCK_TYPE 0
#define _MAGIC_NUMBER 1 
//bytes 2 and 3 empty
// #define _ROOT_INODE_BLOCK 4 //int, where the inode blocks start (inodes could be mixed in w data blocks)
#define _FREE_BLOCK_INDEX 8 //int, where the free blocks start
#define _NUM_FREE_BLOCKS  12 //int, total free blocks

//macros for inode
// #define _BLOCK_TYPE 0
// #define _MAGIC_NUMBER 1 
#define _NAME 4 //char[9], file name
#define _SIZE 13 //int, file size
#define _DATA_BLOCK 17 //int, block number of the first data block
// #define _INODE_SIZE 17 // 9 + 4 + 4

#define _TIME_CREATE_INDEX 25
#define _TIME_MODIFY_INDEX 45
#define _TIME_ACCESSED_INDEX 65



typedef struct {
    int inodeBlock; // block number of the inode block containing this file's inode
    //int fd; // index of the inode within its block
    int filePointer; // current position of the file pointer
} FileTableEntry;

#define FILE_TABLE_SIZE 10 // max # of open files

// superblock structure (I think this is how to implement it?)
typedef struct {
    unsigned char blockType;    // DO NOT DELETE. STRUCT READ FROM TOP TO BOTTOM. THIS IS NEEDED
    unsigned char magicNumber; // magic # for file system detection
    int rootInodeBlock; // block # of the root inode
    int freeBlockIndex; // index of the next free block
    int numFreeBlocks; 
    unsigned char freeSpace[BLOCKSIZE]; //need enough space to read a full block
} Superblock; 


// inode structure (I think this is how to implement it?)
typedef struct {
    char name[9]; // file name (up to 8 char)
    int size; // file size
    int dataBlock; // block number of the first data block
    time_t time_created;        
    time_t time_last_accessed;
    time_t time_last_modified;
} Inode;

typedef int fileDescriptor;

int tfs_mkfs(char *filename, int nBytes);
int tfs_mount(char *diskname);
int tfs_unmount(void);
fileDescriptor tfs_openFile(char *name);
int tfs_closeFile(fileDescriptor FD);
int tfs_writeFile(fileDescriptor FD, char *buffer, int size);
int tfs_deleteFile(fileDescriptor FD);
int tfs_readByte(fileDescriptor FD, char *buffer);
int tfs_seek(fileDescriptor FD, int offset);

