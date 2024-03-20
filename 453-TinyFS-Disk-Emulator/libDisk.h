#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#define BLOCKSIZE 256

int openDisk(char *, int);
int closeDisk(int);
int readBlock(int, int, void *);
int writeBlock(int, int, void *);
