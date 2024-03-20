#include <stdio.h>
#include "tinyFS.h"

int main() {
    char* filename = "tinyFSDisk-Test.txt"; // file name for the disk
    int diskSize = DEFAULT_DISK_SIZE; // default disk size
    fileDescriptor fd;
    int result;

    // create a TinyFS file system
    printf("Creating TinyFS file system...\n");
    result = tfs_mkfs(filename, diskSize);
    if (result == 0) {
        printf("TinyFS file system created successfully.\n\n");
    } else {
        printf("Failed to create TinyFS file system.\n");
        return 1;
    }

    // mount the TinyFS file system
    printf("\n\nMounting TinyFS file system...\n");
    result = tfs_mount(filename);
    if (result == 0) {
        printf("TinyFS file system mounted successfully.\n");
    } else {
        printf("Failed to mount TinyFS file system.\n");
        return 1;
    }

    //open a file in the TinyFS file system
    printf("\n\nOpening a file...\n");
    fd = tfs_openFile("test.txt");
    printf("AFTER RETURNING\n");
    if (fd != -1) {
        printf("File opened successfully with file descriptor: %d\n", fd);
    } else {
        printf("Failed to open file.\n");
        return 1;
    }

    // // write data to the file
    printf("\n\nWriting data to the file...\n");
    char data[] = "Hello, TinyFS!";
    result = tfs_writeFile(fd, data, sizeof(data));
    if (result == 0) {
        printf("Data written to the file successfully.\n");
    } else {
        printf("Failed to write data to the file.\n");
        return 1;
    }

    //open another file in the TinyFS file system
    printf("\n\nOpening another file...\n");
    fd = tfs_openFile("t2.txt");
    printf("AFTER RETURNING\n");
    if (fd != -1) {
        printf("File opened successfully with file descriptor: %d\n", fd);
    } else {
        printf("Failed to open file.\n");
        return 1;
    }

    // // write data to the file
    printf("\n\nWriting another file...\n");
    char newdata[] = "I love Pepper!";
    result = tfs_writeFile(fd, newdata, sizeof(newdata));
    if (result == 0) {
        printf("Data written to the file successfully.\n");
    } else {
        printf("Failed to write data to the file.\n");
        return 1;
    }

    //open a third file in the TinyFS file system
    printf("\n\nOpening a third file...\n");
    fd = tfs_openFile("t3.txt");
    printf("AFTER RETURNING\n");
    if (fd != -1) {
        printf("File opened successfully with file descriptor: %d\n", fd);
    } else {
        printf("Failed to open file.\n");
        return 1;
    }

    // // write data to the file
    printf("\n\nWriting another file...\n");
    char newnewdata[] = "I love coding!";
    result = tfs_writeFile(fd, newnewdata, sizeof(newnewdata));
    if (result == 0) {
        printf("Data written to the file successfully.\n");
    } else {
        printf("Failed to write data to the file.\n");
        return 1;
    }

    // write more data to first file
    printf("\n\nWriting more data to first file...\n");
    char moredata[] = "I love sleeping!";
    result = tfs_writeFile(1, moredata, sizeof(moredata));
    if (result == 0) {
        printf("Data written to the file successfully.\n");
    } else {
        printf("Failed to write data to the file.\n");
        return 1;
    }

    // // close the file
    // printf("Closing the file...\n");
    // result = tfs_closeFile(fd);
    // if (result == 0) {
    //     printf("File closed successfully.\n");
    // } else {
    //     printf("Failed to close the file.\n");
    //     return 1;
    // }

    // // unmount the TinyFS file system
    // printf("Unmounting TinyFS file system...\n");
    // result = tfs_unmount();
    // if (result == 0) {
    //     printf("TinyFS file system unmounted successfully.\n");
    // } else {
    //     printf("Failed to unmount TinyFS file system.\n");
    //     return 1;
    // }

    // return 0;
}
