#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "libDisk.h"

#define BLOCKSIZE 256

int openDisk(char *filename, int nBytes) {
    //printf("entered func\n");
    //printf("nBytes: %d, BLOCKSIZE: %d\n", nBytes, BLOCKSIZE);
    if (nBytes < BLOCKSIZE && nBytes != 0) {
        printf("returning -1\n");   // TODO - Rewrite error codes
        return -1; // failure (nBytes should be at least BLOCKSIZE) so return neg
    }
    
    int flags = O_RDWR | O_CREAT;
    if (nBytes == 0) {
        flags = O_RDWR; // open in read-only mode if nBytes is 0
    }
    
    int fd = open(filename, flags, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        // perror("Failed to open file");
        return -1; // failure (unable to open file) so return neg
    }
    
    if (nBytes > 0) {
        // truncate the file to the required size
        int diskSize = nBytes - (nBytes % BLOCKSIZE); // adjusting for BLOCKSIZE
        //printf("diskSize: %d\n", diskSize);
        if (ftruncate(fd, diskSize) == -1) {
            perror("Failed to truncate file");
            close(fd);
            return -1; // failure, so return negative
        }
    }
    
    return fd; // success, so return file descriptor as disk number
}

int closeDisk(int disk) {
    return close(disk); // close the file descriptor
}

int readBlock(int disk, int bNum, void *block) {
    off_t offset = bNum * BLOCKSIZE;
    if (lseek(disk, offset, SEEK_SET) == -1) {
        perror("Failed to seek file");
        return -1; // failure (unable to seek file) so return negative
    }
    
    ssize_t bytesRead = read(disk, block, BLOCKSIZE);
    if (bytesRead == -1) {
        perror("Failed to read from file");
        return -1; // failure (unable to read from file) so return negative
    } else if (bytesRead < BLOCKSIZE) {
        return -1; // failure (read fewer bytes than expected) so return negative
    }
    
    return 0; // success
}

int writeBlock(int disk, int bNum, void *block) {
    //printf("entered writeblock() with fd %d\n", disk);
    //printf("entered writeBlock()\n");
    off_t offset = bNum * BLOCKSIZE;
    //printf("offest: %lld\n", offset);
    if (lseek(disk, offset, SEEK_SET) == -1) {
        perror("Failed to seek file");
        return -1; // failure (unable to seek file) so return neg
    }

    //char* data = block;
    
    //printf("about to call write() with disk %d, BLOCKSIZE %d\n", disk, BLOCKSIZE);
    // for(int i = 0; i < 256; i++) {
    //     printf("%c\n", data[i]);
    // }
    
    ssize_t bytesWritten = write(disk, block, BLOCKSIZE);
    //printf("succefully did write()\n");
    if (bytesWritten == -1) {
        perror("Failed to write to file");
        return -1; // failure (unable to write to file) so return neg
    } else if (bytesWritten < BLOCKSIZE) {
        return -1; // failure (wrote fewer bytes than expected) so return neg
    }
    printf("exiting writeBlock() with disk = %d\n", disk);
    //printf("exiting writeblock() with success\n");
    return 0; // success
}

// -----------------------------------
// BELOW IS FOR TESTING
// -----------------------------------

// int main() {
//     char* filename = "hello.txt";

//     //Test 1: Bad # bytes
//     int nBytes = 5;
//     int res = openDisk(filename, nBytes);
//     printf("Test 1 (Open File) Result: %d\n", res);

//     //Test 2: Base # Bytes + writing/reading
//     nBytes = 256;
//     res = openDisk(filename, nBytes);
//     printf("Test 2 (Open File) Result: %d\n", res);
//     int blockNum = 0;
//     char data[BLOCKSIZE] = "Hello, world!";
//     int writeRes = writeBlock(res, blockNum, data);
//     if (writeRes == 0) {
//         printf("Data written successfully to block %d\n", blockNum);
//     } else {
//         printf("Failed to write data to block %d\n", blockNum);
//     }
//     char readData[BLOCKSIZE];
//     int readRes = readBlock(res, blockNum, readData);
//     if (readRes == 0) {
//         printf("Data read successfully from block %d\n", blockNum);
//     } else {
//         printf("Failed to read data from block %d\n", blockNum);
//     }
//     printf("Data read: %s\n", readData);
//     int closeRes = closeDisk(res);
//     printf("Disk closed. Close result: %d\n", closeRes);

//     //Test 3: Rounded-down # bytes
//     nBytes = 589;
//     int openRes3 = openDisk(filename, nBytes);
//     printf("Test 3 (Open File) Result: %d\n", openRes3);
//     int blockNum1 = 0;
//     char data3_1[BLOCKSIZE] = "Hello, world!";
//     int writeRes3 = writeBlock(openRes3, blockNum1, data3_1);
//     if (writeRes3 == 0) {
//         printf("Data written successfully to block %d\n", blockNum1);
//     } else {
//         printf("Failed to write data to block %d\n", blockNum1);
//     }
    
//     int blockNum2 = 1;
//     char data3_2[BLOCKSIZE] = "This is a new block!";
//     int writeRes3_1 = writeBlock(openRes3, blockNum2, data3_2);
//     if (writeRes3_1 == 0) {
//         printf("Data written successfully to block %d\n", blockNum2);
//     } else {
//         printf("Failed to write data to block %d\n", blockNum2);
//     }
    
//     char readData3_1[BLOCKSIZE];
//     int readRes3_1 = readBlock(openRes3, blockNum1, readData3_1);
//     if (readRes3_1 == 0) {
//         printf("Data read successfully from block %d\n", blockNum1);
//     } else {
//         printf("Failed to read data from block %d\n", blockNum1);
//     }
//     printf("Data read: %s\n", readData3_1);

//     char readData3_2[BLOCKSIZE];
//     int readRes3_2 = readBlock(openRes3, blockNum2, readData3_2);
//     if (readRes3_2 == 0) {
//         printf("Data read successfully from block %d\n", blockNum2);
//     } else {
//         printf("Failed to read data from block %d\n", blockNum2);
//     }
//     printf("Data read: %s\n", readData3_2);

//     int closeRes3 = closeDisk(openRes3);
//     printf("Disk closed. Close result: %d\n", closeRes3);

//     return 0;
// }