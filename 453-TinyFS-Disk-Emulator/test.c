#include "tinyFS.h"
#include "libDisk.h"

int main() {
    printf("about to make fs\n");
    tfs_mkfs("test1.txt", 1024);    // 4 blcoks long
    printf("about to call mount\n");
    tfs_mount("test1.txt");

    // by now, should have created 4 block disk w superblock as block 0
    int disk = tfs_get_mounted_disk();
    char block[256];
    readBlock(disk, 0, &block);  // reaed superblock
    if (block[0] != 1) {
        printf("WRONG. Superblock byte 0 is %d\n", block[0]);
    }
    if (block[1] != 0x44) {
        printf("WRONG. Superblock byte 1 is %d\n", block[1]);
    }

}


int createFile(char *filename) {
   int block[BLOCKSIZE];
//    INodeBlockHDR *inbh = NULL;
//    SuperBlockHDR *sbh = NULL;
   int blockIdx = -1;

   // check params
   if (filename == NULL) {
      return -1;
   }

   // check disk
   if (diskFD == -1) {
      return -1;
   }

   // get superblock
   if (readBlock(diskFD, 0, block) == -1) {
      return -1;
   }

   sbh = (SuperBlockHDR *)block;

   // check number of free blocks (must be greater than 2 in order to hold the
   //    inode and at least 1 block of data)
   if (sbh->freeCount < 2) {
      return -1;
   }

   // clean up
   memset(block, 0, BLOCK_SIZE); 

   // get a free block
   if ((blockIdx = getFreeBlocks(diskFD, 1)) == -1) {
      return -1;
   }

   inbh = (INodeBlockHDR *)block;

   // package the inode data
   inbh->blockHdr.type = INODE;
   inbh->blockHdr.magicNum = BLOCK_MAGIC_NUM;
   inbh->blockHdr.addressNext = 0;
   if (strncpy(inbh->filename, filename, MAX_FILENAME_LEN) == NULL) {
      return -1;
   }
   inbh->creationTime = time(NULL);
   inbh->totalFileSize = 0;

   // write the inode data
   if (writeBlock(diskFD, blockIdx, block) == -1) {
      return -1;
   }

   return blockIdx;
}

int tfs_openFile(char *name) {
   int inodeBlockIdx = -1;
   FileTableEntry *fte = NULL;

   // check params
   if (name == NULL) {
      return -1;
   }

   // check for zero length
   if (strnlen(name, MAX_FILENAME_LEN) == 0) {
      return -1;
   }

   // check the disk
   if (diskFD == -1) {
      if ((diskFD = mountFS()) == -1) {
         return -1;
      }

      // initialize the file description table (aka. linked list)
      if (linkedList == NULL) {
         if (InitLL(&linkedList, comparator) == -1) {
            return -1;
         }
      }
   }

   int i;
   for (i = 0; i < LLSize(linkedList); i++) {
      fte = LLGet(linkedList, i);
      if (fte == NULL) {
         return -1;
      }
      if (fte->fileName == NULL || name == NULL) {
         return -1;
      }
      if (strncmp(fte->fileName, name, MAX_FILENAME_LEN) == 0) {
         // this file is already open
         return -1;
      }
   }

   // check for the file
   if ((inodeBlockIdx = findFileByName(name)) == -1) { // error
      return -1;
   }
   else if (inodeBlockIdx == 0) { // file not found
      if ((inodeBlockIdx = createFile(name)) == -1) {
         return -1;
      }
   }

   // allocate file table entry
   if ((fte = (FileTableEntry *)calloc(1, sizeof(FileTableEntry))) == NULL) {
      perror("calloc failed");
      return -1;
   }

   // initialize file table entry
   fte->fd = currentFD++;
   fte->inodeIdx = inodeBlockIdx;
   fte->curByteOffset = 0;
   strncpy(fte->fileName, name, MAX_FILENAME_LEN);

   LLInsertTail(linkedList, fte);

   return fte->fd;
}
<<<<<<< HEAD
=======

int tfs_writeFile(int FD, char *buffer) {
   uint8_t inodeBlock[BLOCK_SIZE] = "";
   uint8_t block[BLOCK_SIZE] = "";
   FileTableEntry *ftePtr = NULL;
   INodeBlockHDR *inbh = NULL;
   BlockHDR *bh = NULL;
   int fdIndex = -1;
   int blockIdx = -1;
   uint32_t dataLength = -1;
   int numBlocksNeeded = -1;

   // check params
   if (FD < 0 || buffer == NULL) {
      return -1;
   }

   // check the disk
   if (diskFD == -1) {
      return -1;
   }

   // truncate old data
   if (tfs_truncate(FD) == -1) {
      return -1;
   }

   // get the index of the fd in the "table" (aka linked list)
   if ((fdIndex = LLIndexOf(linkedList, FD)) == -1) {
      return -1;
   }

   // get the metadata - need the inode index
   ftePtr = LLGet(linkedList, fdIndex);

   // get inode block
   if (readBlock(diskFD, ftePtr->inodeIdx, inodeBlock) == -1) {
      return -1;
   }

   inbh = (INodeBlockHDR *)inodeBlock;

   // calculate number of free blocks
   dataLength = strlen(buffer);
   numBlocksNeeded = (int)ceil((double)dataLength / BLOCK_DATA_SIZE);

   // get the free blocks
   if ((blockIdx = getFreeBlocks(diskFD, numBlocksNeeded)) == -1) {
      return -1;
   }

   // update the inode
   inbh->blockHdr.type = INODE;
   inbh->blockHdr.addressNext = blockIdx;
   inbh->totalFileSize = dataLength;

   // write the updated inode block
   if (writeBlock(diskFD, ftePtr->inodeIdx, inodeBlock) == -1) {
      return -1;
   }

   int i = 0;
   for (i = 0; i < numBlocksNeeded; i++) {
      // read the block
      if (readBlock(diskFD, blockIdx, block) == -1) {
         return -1;
      }
      bh = (BlockHDR *)block;

      // update the block
      bh->type = FILE_EXTENDS;
      bh->magicNum = BLOCK_MAGIC_NUM;
      strncpy((char *)(block+sizeof(BlockHDR)), &buffer[i * BLOCK_DATA_SIZE],
               BLOCK_DATA_SIZE);

      // write the block
      if (writeBlock(diskFD, blockIdx, block) == -1) {
         return -1;
      }

      // update blockIdx and offset
      blockIdx = bh->addressNext;
   }

   // set the file point to the start of the file
   ftePtr->curByteOffset = 0;

   return 0;
}
>>>>>>> 961bb0ce168295df57051a9596d64e20f27fd439
