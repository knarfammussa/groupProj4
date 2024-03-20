#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "libDisk.h" // Include the disk emulator library
#include "tinyFS.h"

FileTableEntry fileTable[FILE_TABLE_SIZE]; // file table to track open files
int recycle_fd[FILE_TABLE_SIZE] = {0};
//I think the table needs to be dynamic, not a fixed size. I'm not sure how to implement this at the moment.
int nextFileDescriptor = 0; // next available file descriptor

static int mounted = 0; // flag to indicate whether a file system is mounted
static int mounted_disk = -1; // variable to store the disk number of the mounted file system

char superblock[BLOCKSIZE] = {0};

int tfs_mkfs(char *filename, int nBytes) {
    // check if nBytes is valid
    if (nBytes < BLOCKSIZE) {
        return -1; // failure (nBytes should be at least BLOCKSIZE) so return neg
    }
    
    // open the disk file using the disk emulator library
    int disk = openDisk(filename, nBytes);
    if (disk == -1) {
        return -1; // failure (unable to open disk file) so return neg
    }
    
    // create and initialize the superblock
    // superblock.magicNumber = 0x44; // magic # for TinyFS
    // superblock.rootInodeBlock = 1; // root inode block starts from block 1
    // superblock.numFreeBlocks = DEFAULT_DISK_SIZE / BLOCKSIZE - 2;
    // int freeBlockIndex = 2;
    // superblock.freeBlocks = (unsigned char*)malloc(sizeof(char)*superblock.numFreeBlocks); // first free block starts from block 2
    // for (int i = 0; i < superblock.numFreeBlocks; i++) {
    //     superblock.freeBlocks[i] = 0;
    // }
    // empty block filled with 0s
    superblock[_BLOCK_TYPE] = 1;
    superblock[_MAGIC_NUMBER] = 0x44;
    // superblock[_ROOT_INODE_BLOCK] = 1;
    superblock[_FREE_BLOCK_INDEX] = 1; //is this 1 or 2?
    superblock[_NUM_FREE_BLOCKS] = DEFAULT_DISK_SIZE / BLOCKSIZE - 2;
    
    // write the superblock to the disk
    if (writeBlock(disk, 0, &superblock) != 0) {     // TODO &superblock
        closeDisk(disk);
        return -1; // failure (unable to write superblock) so return neg
    }

    printf("superblock info: %d %d %d %d %d\n", superblock[_BLOCK_TYPE], superblock[_MAGIC_NUMBER], superblock[_ROOT_INODE_BLOCK], superblock[_FREE_BLOCK_INDEX], superblock[_NUM_FREE_BLOCKS]);
    
    // initialize and write root inode (assume it's a single block?)
    // char rootInode[BLOCKSIZE] = {0}; // empty block filled with 0s
    // rootInode[_BLOCK_TYPE] = 2; // inode block type
    // rootInode[_MAGIC_NUMBER] = 0x44; // magic number for inode block
    // strcpy(&rootInode[_NAME], "root"); // Copy the string "root" into the inode block

    // // printing to verify the contents of the inode block
    // printf("rootInode[_BLOCK_TYPE]: %d\n", rootInode[_BLOCK_TYPE]);
    // printf("rootInode[_MAGIC_NUMBER]: %x\n", rootInode[_MAGIC_NUMBER]);
    // printf("rootInode[_NAME]: %s\n", &rootInode[_NAME]);

    //we do not need to make an inode called "root" at the start of block... we can just start adding inodes
    // strcpy(&rootInode[_NAME], "root"); 
    // rootInode[_SIZE] = 0; // root directory size (empty for now)
    // rootInode[_DATA_BLOCK] = -1; // no data blocks allocated for root directory yet
    
    // write the root inode to the disk
    // if (writeBlock(disk, 1, &rootInode) != 0) {
    //     closeDisk(disk);
    //     return -1; // failure (unable to write root inode) so return neg
    // }
    
    // initialize and write free blocks (remaining blocks after superblock and root inode)
    char emptyBlock[BLOCKSIZE] = {0}; // empty block filled with 0s
    
    emptyBlock[0] = 4; // Signify free block
    emptyBlock[1] = 0x44; // Signify magic number
    
    //printf("mkfs empty block for loop spans range [2, %d]\n", nBytes / BLOCKSIZE);
    for (int i = 1; i < nBytes / BLOCKSIZE; i++) {
        //printf("mkfs about to write empty block for disk %d\n", disk);
        if (writeBlock(disk, i, emptyBlock) != 0) {
            closeDisk(disk);
            return -1; // failure (unable to write free block) so return neg
        }
        //printf("mkfs wrote empytblock\n");
    }
    
    // close disk file (should this be here?)
    closeDisk(disk);
    
    return 0; // success
}

int tfs_mount(char *diskname) {
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
    printf("in mount, opened disk %d\n", disk);

    // read superblock from disk via libDisk
    Superblock superblock;  // 13 bytes made
    if (readBlock(disk, SUPERBLOCK_BLOCK_NUM, &superblock) != 0) {
        closeDisk(disk);
        printf("Failed to read superblock.\n");
        return -1; // failure (unable to read superblock)
    }

    //printf("superblock info: %d %d %d %d %d %s\n", superblock.blockType, superblock.magicNumber, superblock.rootInodeBlock, superblock.freeBlockIndex, superblock.numFreeBlocks, superblock.freeSpace);

    // check if magic number is correct
    if (superblock.magicNumber != 0x44) {
        closeDisk(disk);
        printf("Incorrect magic number. Not a TinyFS filesystem.\n");
        return -1; // failure (not a TinyFS filesystem) so return neg
    }

    // update mounted flag and disk number
    mounted = 1;
    mounted_disk = disk;
    printf("tfs_mount: mounted_disk = %d\n", mounted_disk);

    printf("File system mounted successfully.\n");

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
    if (!mounted) {
        printf("No file system is currently mounted.\n");
        return -1; // failure (no file system mounted)
    }

    if (name == NULL) {
        printf("Empty name.\n");
        return -1;
    }

    if (strnlen(name, 8) == 0) {
        printf("Name too short.\n");
        return -1;
    }

    // check if file already exists
    char inode[BLOCKSIZE] = {0};
    int inodeIndex = -1;
    for (int i = 0; i < BLOCK_COUNT-1; i++) {    // should run through all inodes
        // to find if file already exists, we want to run through each name and compare to our input name
        if (readBlock(mounted_disk, 1 + i, inode) != 0) { //inode and file extent blocks start from block 1
            printf("Failed to read root inode block.\n");
            return -1; // failure (unable to read root inode block)
        }
        printf("] tfs_openFile SUPERBLOCK INODE INFO:\n");      // was originally root
        printf("    ] rootInode[_BLOCK_TYPE]: %d\n", inode[_BLOCK_TYPE]);
        printf("    ] rootInode[_MAGIC_NUMBER]: %x\n", inode[_MAGIC_NUMBER]);
        printf("    ] rootInode[_NAME]: %s\n", &inode[_NAME]);
        printf("] END ROOT INODE INFO\n");
        //its block loaded is in fact an inode block (and not a data block)
       
        // look for a free inode from root inode
        if (inode[_BLOCK_TYPE] == 2 && inode[_MAGIC_NUMBER] == 0x44){  // first root inode     
            char temp_name[9] = {0};
            memcpy(&temp_name, &inode[_NAME], 9);
            if (strcmp(temp_name, name) == 0) {     // compare names to see if file alreay exists 
                inodeIndex = i+1;
                break;
            }
            if (inodeIndex != -1) {
                break;
            }
        }
    }
    // create a new inode for file since it doesn't exist, put at freeblock
    if (inodeIndex == -1) {
        // find free inode block, then free inode within that block
        char superblock[BLOCKSIZE];
        int rb = readBlock(mounted_disk, 0, superblock);
        if (rb == -1) {
            printf("Failed to read superblock.\n");
            return -1; // failure (unable to read superblock)
        }
        if (superblock[_NUM_FREE_BLOCKS] < 2) {
            printf("Not enough blocks to create a new inode and file data.\n");
            return -1;
        }

        char emptyBlock[BLOCKSIZE] = {0}; // empty block filled with 0s
        emptyBlock[_BLOCK_TYPE] = 2; // inode block type
        emptyBlock[_MAGIC_NUMBER] = 0x44; // magic number for inode block
        
        memcpy(&(emptyBlock[_NAME]), name, 8);
        emptyBlock[_NAME + 8] = '\0';      
        emptyBlock[_SIZE]= 0;
        emptyBlock[_DATA_BLOCK] = -1; // no data yet
        inodeIndex = superblock[_FREE_BLOCK_INDEX];
        
        printf("file: %s, freeblockindex: %d\n", name, superblock[_FREE_BLOCK_INDEX]);
        writeBlock(mounted_disk, inodeIndex, emptyBlock);

        superblock[_FREE_BLOCK_INDEX]++;
        superblock[_NUM_FREE_BLOCKS]--;
        writeBlock(mounted_disk, 0, superblock); //add updated write block

        // printf("] tfs_openFile INODE INFO:\n");
        // printf("    ] inode[_BLOCK_TYPE]: %d\n", emptyBlock[_BLOCK_TYPE]);
        // printf("    ] inode[_MAGIC_NUMBER]: %x\n", emptyBlock[_MAGIC_NUMBER]);
        // printf("    ] inode[_NAME]: %s\n", &emptyBlock[_NAME]);
        // printf("] END tfs_openFile INODE INFO\n");
    }

    // check if file table is full
   

    // add entry to file table
    fileTable[nextFileDescriptor].inodeBlock = inodeIndex; // 1 = root inode block
    fileTable[nextFileDescriptor].inodeIndex = -1; //inodeIndex % INODES_PER_BLOCK; dont need
    fileTable[nextFileDescriptor].filePointer = 0;

    printf("fileTable inodeBlock: %d\n", fileTable[nextFileDescriptor].inodeBlock);
    printf("fileTable inodeIndex: %d\n", fileTable[nextFileDescriptor].inodeIndex);
    printf("fileTable filePointer: %d\n", fileTable[nextFileDescriptor].filePointer);


    // return file descriptor
    if (nextFileDescriptor < FILE_TABLE_SIZE){
        return nextFileDescriptor++;
    }

    else{
        for (int i = 0; i < FILE_TABLE_SIZE; i++){
            if (recycle_fd[i] < 0){
                recycle_fd[i] = 0;
                return i;
            }
        }
    }

    printf("File table is full.\n");
    return -1; // failure (File table full)
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
    recycle_fd[FD] = -1;

    return 0; // success
}

int tfs_writeFile(fileDescriptor FD, char *buffer, int size) {
    printf("entered tfs writefile with fd %d\n", FD);
    printf("will write %s\n", buffer);
    
    // Implement writing to a file in the TinyFS filesystem
    if (FD < 0 || FD >= FILE_TABLE_SIZE) {
        printf("Invalid file descriptor.\n");
        return -1; // failure (Invalid file descriptor)
    }

    //read in inode from inode block on disk (to get name of the file of FD)
    char inode[BLOCKSIZE];  // ptr to inode block
    if (readBlock(mounted_disk, fileTable[FD].inodeBlock, inode) == -1){ //read in inode block
        printf("Failed to read inode block.\n");
        return -1; // failure (unable to read inode block)
    }

    // for(int i = 0; i < sizeof(inode); i++) {
    //     printf("%c\n", inode[i]);
    // }
    
    //get the name of the file for FD
    char name[9] = {0};
    printf("name is %s\n", name);
    printf("we want to copy %s\t", &(inode[_NAME]));
    printf("into source %s\n", name);
    memcpy(name, &(inode[_NAME]), 9);
    
    printf("calling delete\n");
    tfs_deleteFile(FD); //delete the file (so we can clear the blocks FD was using, sets them to free blocks, shifts everything over
    printf("buffer after deletion: %s\n", buffer);
    FD = tfs_openFile(name); //readd the file (so we can write to it again, and it will be at the end of the file table and contiguous!)
    printf("fd after opening %d\n", FD);
    //read in superblock to check if there are enough free blocks to write the file
    char superblock[BLOCKSIZE] = {0};
    int rb = readBlock(mounted_disk, 0, superblock);
    if (rb == -1) {
        printf("Failed to read superblock.\n");
        return -1; // failure (unable to read superblock)
    }

    int blocks_avail = superblock[_NUM_FREE_BLOCKS];
    int blocks_needed = (sizeof(int) * size) / (BLOCKSIZE - 4);
    if ((sizeof(int) * size) % (BLOCKSIZE - 4) != 0) {
        blocks_needed++;
    }

    if (blocks_needed > blocks_avail) {
        printf("Not enough free blocks to write file.\n");
        return -1; // failure (Not enough free blocks to write file)
    }

    char* current_buffer = buffer;
    printf("blocks_needed: %d\n", blocks_needed);
    printf("current_buffer: %s\n", current_buffer);


    for (int i = 0; i<blocks_needed; i++) {
        char block_data[BLOCKSIZE];
        block_data[0] = 3;
        block_data[1] = 0x44;
        block_data[2] = 0;
        block_data[3] = 0;


        if (i != blocks_needed -1){ //copy 252 bytes into buffer
            printf("destination: %s\n", &block_data[4]);
            printf("source: %s\n", current_buffer);
            memcpy(&block_data[4], current_buffer, 252);
            current_buffer += 252;
        }
        else{ //copy only the remaining bytes 
            printf("destination: %s\n", &block_data[4]);
            printf("source: %s\n", current_buffer);
            memcpy(&block_data[4], current_buffer, (size % (BLOCKSIZE - 4)));
        }
        
        printf("block # %d, text %c\n", i, current_buffer[0]);

        writeBlock(mounted_disk, superblock[_FREE_BLOCK_INDEX], block_data);
        superblock[_FREE_BLOCK_INDEX]++;
        superblock[_NUM_FREE_BLOCKS]--;
    }

    writeBlock(mounted_disk, 0, superblock); //add updated write block
    
    // int inode_offset = fileTable[FD].inodeIndex; not using anymore
    inode[_SIZE] = size;
    inode[_DATA_BLOCK] = superblock[_FREE_BLOCK_INDEX] - blocks_needed;
    writeBlock(mounted_disk, fileTable[FD].inodeBlock, inode);

    fileTable[FD].filePointer = 0;
    return 0;   // added to compile TODO change
}

// int tfs_deleteFile(fileDescriptor FD) {
//     // Implement deleting a file in the TinyFS filesystem
//     if (FD < 0 || FD >= FILE_TABLE_SIZE) {
//         printf("Invalid file descriptor.\n");
//         return -1; // failure (Invalid file descriptor)
//     }

//     //read in inode from inode block on disk (to get size of FD)
//     char inode[BLOCKSIZE];
//     if (readBlock(mounted_disk, fileTable[FD].inodeBlock, inode) == -1){ //read in inode block
//         printf("Failed to read inode block.\n");
//         return -1; // failure (unable to read inode block)
//     }

//     // int inode_start = fileTable[FD].inodeIndex; //gets the offset of the inode in the inode block
//     int file_bytes =  inode[_SIZE]; //i think size is in ints

//     int file_blocks = (file_bytes / (BLOCKSIZE - 4)); //number of blocks the file takes up
//     if ( file_bytes % (BLOCKSIZE - 4) != 0) { //round up (if it does not fit block perfectly
//         file_blocks++;
//     }

//     file_blocks++; //add one for the inode block! because were also getting rid of the inode block

//     //delete from inode and the following file_blocks 
//     int first_block = fileTable[FD].inodeBlock ; // block # of first data block for FD
//     printf("DELETE_FILE: deleting this many blocks %d\n", file_blocks);

//     //read in super, to get root inode used in for-loop
//     char superblock[BLOCKSIZE];
//     int rb = readBlock(mounted_disk, 0, superblock);
//     if (rb == -1) {
//         printf("Failed to read superblock.\n");
//         return -1; // failure (unable to read superblock)
//     }

//     // printf("free is at %d, first is at %d and is this long %d", superblock[_FREE_BLOCK_INDEX], first_block, file_blocks );

//     //move all the files after the deleted file up (removes external fragmentation)
//     for (int i = first_block; i < superblock[_FREE_BLOCK_INDEX]-file_blocks; i++) {
//         printf("deleting block %d\n", i);
//         if (i + file_blocks < FILE_TABLE_SIZE){
//             fileTable[i] = fileTable[i + file_blocks];
//             //read block from disk that we want to move up (aka the data right after FD)
//             char block_data[BLOCKSIZE];
//             readBlock(mounted_disk, superblock[_ROOT_INODE_BLOCK] + file_blocks + i, block_data);         
//             writeBlock(mounted_disk, superblock[_ROOT_INODE_BLOCK] + i, block_data);
//         }
//     }
//     // printf("flag - delete function \n");

//     //free all the blocks that should now be free
//     char freeBlock[BLOCKSIZE] = {0}; // empty block filled with 0s
//     freeBlock[0] = 4; // Signify free block
//     freeBlock[1] = 0x44; // Signify magic number
//     for (int i = superblock[_FREE_BLOCK_INDEX]; i < superblock[_FREE_BLOCK_INDEX] + file_blocks; i++) {
//         writeBlock(mounted_disk, i, freeBlock);
//     }

//     //update superblock
//     superblock[_NUM_FREE_BLOCKS] += file_blocks;
//     superblock[_FREE_BLOCK_INDEX] -= file_blocks;
//     writeBlock(mounted_disk, 0, superblock); //add updated write block


//     //do we also remove entry from file table??
//     recycle_fd[FD] = -1;
//     fileTable[FD].inodeBlock = -1;
//     fileTable[FD].inodeIndex = -1;
//     fileTable[FD].filePointer = -1;
//     return 0; // TODO added to compile
// }

int tfs_deleteFile(fileDescriptor FD) {
    // Implement deleting a file in the TinyFS filesystem
    if (FD < 0 || FD >= FILE_TABLE_SIZE) {
        printf("Invalid file descriptor.\n");
        return -1; // failure (Invalid file descriptor)
    }
    //read in inode from inode block on disk (to get size of FD)
    char inode[BLOCKSIZE];
    if (readBlock(mounted_disk, fileTable[FD].inodeBlock, inode) == -1){ //read in inode block
        printf("Failed to read inode block.\n");
        return -1; // failure (unable to read inode block)
    }

    // int inode_start = fileTable[FD].inodeIndex; //gets the offset of the inode in the inode block
    int file_bytes = inode[_SIZE]; //i think size is in ints

    int file_blocks = (file_bytes / (BLOCKSIZE - 4)); //number of blocks the file takes up
    if ( file_bytes % (BLOCKSIZE - 4) != 0) { //round up (if it does not fit block perfectly
        file_blocks++;
    }

    file_blocks++; //add one for the inode block! because were also getting rid of the inode block

    //delete from inode and the following file_blocks 
    int first_block = fileTable[FD].inodeBlock ; // block # of first data block for FD
    printf("DELETE_FILE: deleting this many blocks %d\n", file_blocks);

    //read in super, to get root inode used in for-loop
    char superblock[BLOCKSIZE];
    int rb = readBlock(mounted_disk, 0, superblock);
    if (rb == -1) {
        printf("Failed to read superblock.\n");
        return -1; // failure (unable to read superblock)
    }

    //move all the files after the deleted file up (removes external fragmentation)
    for (int i = first_block; i < superblock[_FREE_BLOCK_INDEX]; i++) {
        if (i + file_blocks < FILE_TABLE_SIZE){
            fileTable[i] = fileTable[i + file_blocks];
            //read block from disk that we want to move up (aka the data right after FD)
            char block_data[BLOCKSIZE];
            readBlock(mounted_disk, superblock[_ROOT_INODE_BLOCK] + file_blocks + i, block_data);         
            writeBlock(mounted_disk, superblock[_ROOT_INODE_BLOCK] + i, block_data);
        }
    }

    //free all the blocks that should now be free
    char freeBlock[BLOCKSIZE] = {0}; // empty block filled with 0s
    freeBlock[0] = 4; // Signify free block
    freeBlock[1] = 0x44; // Signify magic number
    for (int i = superblock[_FREE_BLOCK_INDEX]; i < FILE_TABLE_SIZE; i++) {
        writeBlock(mounted_disk, i, freeBlock);
    }

    //update superblock
    superblock[_NUM_FREE_BLOCKS] += file_blocks;
    superblock[_FREE_BLOCK_INDEX] -= file_blocks;
    writeBlock(mounted_disk, 0, superblock); //add updated write block

    inode[_NAME] = '\0'; //clear the name of the file
    inode[_SIZE] = 0; //clear the size of the file
    inode[_DATA_BLOCK] = -1; //clear the data block of the file
    writeBlock(mounted_disk, 1+fileTable[FD].inodeBlock, inode); //1 is the root inode block


    //do we also remove entry from file table??
    fileTable[FD].inodeBlock = -1;
    fileTable[FD].inodeIndex = -1;
    fileTable[FD].filePointer = -1;
    recycle_fd[FD] = -1;

    return 0; // TODO added to compile
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

// DEBUGGING
int tfs_get_mounted_disk( ) {
    return mounted_disk;
}

/* Read the Superblock */
//   char superblock[BLOCKSIZE];
//   rb = readBlock(MOUNTED_DISK, 0, superblock);

//   Inode inode;
// (readBlock(mounted_disk, entry.inodeBlock, &inode) != 0)