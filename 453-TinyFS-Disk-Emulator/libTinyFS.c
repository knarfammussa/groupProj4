// tiinyFS interface file. 
// this file will access libDisk for disk emulator funcitonality

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "libDisk.h"
#include "tinyFS.h"

FileTableEntry fileTable[FILE_TABLE_SIZE]; // file table to track open files
int nextFileDescriptor = 0; // next available file descriptor

static int mounted = 0; // flag to indicate whether a file system is mounted
static int mounted_disk = -1; // variable to store the disk number of the mounted file system

Superblock superblock;

int tfs_mkfs(char *filename, int nBytes) {
    // check if nBytes is valid
    printf("Running tfs_mkfs\n");
    if (nBytes < BLOCKSIZE) {
        return -1; // failure (nBytes should be at least BLOCKSIZE) so return neg
    }
    
    // open the disk file using the disk emulator library
    int disk = openDisk(filename, nBytes);
    if (disk == -1) {
        return -1; // failure (unable to open disk file) so return neg
    }
    
    // create and initialize the superblock
    superblock.magicNumber = 0x44; // magic # for TinyFS
    superblock.rootInodeBlock = 1; // root inode block starts from block 1
    superblock.numFreeBlocks = DEFAULT_DISK_SIZE / BLOCKSIZE - 2;
    int freeBlockIndex = 2;
    superblock.freeBlocks = (unsigned char*)malloc(sizeof(char)*superblock.numFreeBlocks); // first free block starts from block 2
    for (int i = 0; i < superblock.numFreeBlocks; i++) {
        superblock.freeBlocks[i] = 0;
    }
    
    // write the superblock to the disk
    if (writeBlock(disk, 0, &superblock) != 0) {
        closeDisk(disk);
        return -1; // failure (unable to write superblock) so return neg
    }
    
    // initialize and write root inode (assume it's a single block?)
    Inode rootInode;
    strcpy(rootInode.name, "root"); // root directory name
    rootInode.size = 0; // root directory size (empty for now)
    rootInode.dataBlock = -1; // no data blocks allocated for root directory yet
    
    // write the root inode to the disk
    if (writeBlock(disk, 1, &rootInode) != 0) {
        closeDisk(disk);
        return -1; // failure (unable to write root inode) so return neg
    }
    
    // initialize and write free blocks (remaining blocks after superblock and root inode)
    char emptyBlock[BLOCKSIZE] = {0}; // empty block filled with 0s
    
    for (int i = 2; i < nBytes / BLOCKSIZE; i++) {
        if (writeBlock(disk, i, emptyBlock) != 0) {
            closeDisk(disk);
            return -1; // failure (unable to write free block) so return neg
        }
    }
    
    // close disk file (should this be here?)
    //closeDisk(disk);
    
    return 0; // success
}

int tfs_mount(char *diskname) {
    printf("Running tfs_mount\n");
    if (mounted) {
        printf("A file system is already mounted.\n");
        return -1; // failure (file system already mounted) so return neg
    }

    // open the disk file using libDisk
    int disk = openDisk(diskname, 0);
    if (disk == -1) {
        printf("Failed to open disk.\n");
        return -1; // failure (unable to open disk file) so return neg
    }

    // read superblock from disk via libDisk
    //Superblock superblock;
    printf("size of superblock: %d\n", sizeof(superblock));
    if (readBlock(disk, SUPERBLOCK_BLOCK_NUM, &superblock) != 0) {
        closeDisk(disk);
        printf("Failed to read superblock.\n");
        return -1; // failure (unable to read superblock)
    }

    // check if magic number is correct
    if (superblock.magicNumber != 0x44) {
        closeDisk(disk);
        printf("Incorrect magic number. Not a TinyFS filesystem.\n");
        return -1; // failure (not a TinyFS filesystem) so return neg
    }

    // update mounted flag and disk number
    mounted = 1;
    mounted_disk = disk;

    printf("File system mounted successfully : %d.\n", disk);
    printf("superblock info: %d, %d, %d, %d\n", superblock.magicNumber, superblock.rootInodeBlock, superblock.freeBlockIndex, superblock.numFreeBlocks);
    return 0; // success
}

int tfs_unmount(void) {
    if (!mounted) {
        printf("No file system is currently mounted.\n");
        return -1; // failure (no file system mounted) so return neg
    }

    // Close the disk file
    if (closeDisk(mounted_disk) != 0) {
        printf("Failed to close disk.\n");
        return -1; // failure (unable to close disk)
    }

    // reset mounted flag and disk number
    mounted = 0;
    mounted_disk = -1;

    printf("File system unmounted successfully.\n");

    return 0; // success
}

fileDescriptor tfs_openFile(char *name) {
    printf("opening file\n");
    printf("mounted: %d, superBlock: %d %d %d %d, nextFileDescriptor: %d\n", mounted, superblock.magicNumber, superblock.rootInodeBlock, superblock.freeBlockIndex, superblock.numFreeBlocks, nextFileDescriptor);
    if (!mounted) {
        printf("No file system is currently mounted.\n");
        return -1; // failure (no file system mounted)
    }

    // Check if file table is full
    if (nextFileDescriptor == FILE_TABLE_SIZE) {
        printf("File table is full.\n");
        return -1; // failure (File table full)
    }

    char buffer[BLOCKSIZE];

    Inode inode;
    int inodeIndex = -1;
    //printf("Inode Index: %d\n", inodeIndex);

    // Search for existing file
    for (int i = 0; i < INODE_BLOCK_SIZE; i++) {
        //printf("Inode Index: %d\n", inodeIndex);
        if (readBlock(mounted_disk, superblock.rootInodeBlock + i, buffer) != 0) {
            printf("Failed to read root inode block.\n");
            return -1; // failure (unable to read root inode block)
        }
        memcpy(&inode, buffer, sizeof(inode));
        printf("inode: %s %d %d\n", inode.name, inode.size, inode.dataBlock);
        printf("name : %s\n", name);
        //printf("Inode Index: %d\n", inodeIndex);
        if (strcmp(inode.name, name) == 0) {
            printf("identical\n");
            inodeIndex = i;
            break;
        }
    }

    printf("AFTER SEARCH FOR EXISTING FILE\n");
    printf("Inode Index: %d\n", inodeIndex);

    // create a new file if it doesn't exist
    if (inodeIndex == -1) {
        // find a free inode
        for (int i = 0; i < INODE_BLOCK_SIZE; i++) {
            if (readBlock(mounted_disk, superblock.rootInodeBlock + i, buffer) != 0) {
                printf("Failed to read root inode block.\n");
                return -1; // failure (unable to read root inode block)
            }
            for (int j = 0; j < INODES_PER_BLOCK; j++) {
                // copy the inode from the buffer
                memcpy(&inode, buffer + j * sizeof(Inode), sizeof(Inode));
                printf("inode: %s %d %d\n", inode.name, inode.size, inode.dataBlock);

                // check if the inode is empty
                if (inode.name[0] == '\0') {
                    // Found an empty inode, use it to create the new file
                    inodeIndex = i * INODES_PER_BLOCK + j;
                    strcpy(inode.name, name);
                    inode.size = 0;
                    inode.dataBlock = -1; // no data blocks allocated initially
                    printf("New inode: %s %d %d\n", inode.name, inode.size, inode.dataBlock);
                    memcpy(buffer + j * sizeof(Inode), &inode, sizeof(Inode));
                    printf("%s\n", buffer + j * sizeof(Inode));
                    // write the updated inode back to disk
                    if (writeBlock(mounted_disk, superblock.rootInodeBlock + i, buffer) != 0) {
                        printf("Failed to write new inode.\n");
                        return -1; // failure (unable to write new inode)
                    }

                    break; // exit the inner loop once an empty inode is found
                }
            }
        }
    }

    printf("AFTER CREATE NEW FILE\n");

    // add entry to file table
    fileTable[nextFileDescriptor].inodeBlock = superblock.rootInodeBlock + inodeIndex / INODES_PER_BLOCK;
    fileTable[nextFileDescriptor].inodeIndex = inodeIndex % INODES_PER_BLOCK;
    fileTable[nextFileDescriptor].filePointer = 0;

    printf("BEFORE RETURN: %d\n", nextFileDescriptor);
    // return file descriptor
    return nextFileDescriptor++;
}



int tfs_closeFile(fileDescriptor FD) {
    // Implement closing a file in the TinyFS filesystem
    if (!mounted) {
        printf("No file system is currently mounted.\n");
        return -1; // failure (no file system mounted)
    }

    if (FD < 0 || FD >= FILE_TABLE_SIZE) {
        printf("The FD is invalid.\n");
        return -1; // failure (Invalid file descriptor)
    }

    //remove entry from file table
    fileTable[FD].inodeBlock = -1;
    fileTable[FD].inodeIndex = -1;
    fileTable[FD].filePointer = -1;
    return 0; // success
}

int tfs_writeFile(fileDescriptor FD, char *buffer, int size) {
    // Implement writing to a file in the TinyFS filesystem
    if (FD < 0 || FD >= FILE_TABLE_SIZE) {
        printf("Invalid file descriptor.\n");
        return -1; // failure (Invalid file descriptor)
    }


    int blocks_needed = (sizeof(int) * size) / (BLOCKSIZE - 4);
    if ((sizeof(int) * size) % (BLOCKSIZE - 4) != 0) {
        blocks_needed++;
    }

    if (blocks_needed > superblock.numFreeBlocks) {
        printf("Not enough free blocks to write file.\n");
        return -1; // failure (Not enough free blocks to write file)
    }

    char* current_buffer = buffer;

    for (int i = 0; i<blocks_needed; i++) {
        char block_data[BLOCKSIZE];
        block_data[0] = 3;
        block_data[1] = 0x44;
        block_data[2] = 0;
        block_data[3] = 0;
        memcpy(&block_data[4], current_buffer, 252);
        current_buffer += 252;

        writeBlock(mounted_disk, superblock.freeBlockIndex, block_data);
        superblock.freeBlockIndex++;
        superblock.numFreeBlocks--;
    }
        
    fileTable[FD].filePointer = 0;
    fileTable[FD].size = size;

}

int tfs_deleteFile(fileDescriptor FD) {
    // Implement deleting a file in the TinyFS filesystem
    if (FD < 0 || FD >= FILE_TABLE_SIZE) {
        printf("Invalid file descriptor.\n");
        return -1; // failure (Invalid file descriptor)
    }

    int file_blocks = (sizeof(int) * fileTable[FD].size) / (BLOCKSIZE - 4);
        if ((sizeof(int) * fileTable[FD].size) % (BLOCKSIZE - 4) != 0) {
            file_blocks++;
    }   

    //move all the files after the deleted file up (removes external fragmentation)
    for (int i = FD; i < FILE_TABLE_SIZE - 1; i++) {
        fileTable[i] = fileTable[i + file_blocks];
        char block_data[BLOCKSIZE];
        readBlock(mounted_disk, superblock.rootInodeBlock + i + file_blocks, block_data);
        writeBlock(mounted_disk, superblock.rootInodeBlock + i, block_data);
    }
    superblock.numFreeBlocks += file_blocks;
    superblock.freeBlockIndex -= file_blocks;
}

int tfs_readByte(fileDescriptor FD, char *buffer) {
    // Implement reading a byte from a file in the TinyFS filesystem
    if (FD < 0 || FD >= FILE_TABLE_SIZE) {
        printf("Invalid file descriptor.\n");
        return -1; // failure (Invalid file descriptor)
    }

    int offset = fileTable[FD].filePointer % BLOCKSIZE;

    if (offset == BLOCKSIZE - 1) {
        printf("End of file reached.\n");
        return -1;
    }

    int block_num = fileTable[FD].filePointer / BLOCKSIZE;

    char block[BLOCKSIZE];
    if (readBlock(mounted_disk, block_num, block) != 0) {
        printf("Failed to read block.\n");
        return -1; // failure (unable to read block)
    }
    *buffer = block[offset];
    return 0;
    
    //I'm not sure if this is the correct way to read the file
}

int tfs_seek(fileDescriptor FD, int offset) {
    // Implement seeking within a file in the TinyFS filesystem
    if (FD < 0 || FD >= FILE_TABLE_SIZE) {
        printf("Invalid file descriptor.\n");
        return -1; // failure (Invalid file descriptor)
    }
    fileTable[FD].filePointer = offset;
    return 0; // success
}