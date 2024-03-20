# CC = gcc
# CFLAGS = -Wall -g -std=c99
# #PROG = tinyFSDemo
# PROG = libTinyFS
# #OBJS = tinyFSDemo.o libTinyFS.o libDisk.o
# OBJS = libDisk.o diskTest.o libTinyFS.o

# $(PROG): $(OBJS)
# 	$(CC) $(CFLAGS) -o $(PROG) $(OBJS)

# # tinyFsDemo.o: tinyFSDemo.c libTinyFS.h tinyFS.h TinyFS_errno.h
# # 	$(CC) $(CFLAGS) -c -o $@ $<

# libTinyFS.o: tinyFS.h libDisk.h libDisk.o libTinyFS.c #libTinyFS.h tinyFS.h libDisk.h libDisk.o #TinyFS_errno.h
# 	$(CC) $(CFLAGS) -c -o $@ $<

# #libDisk.o: libDisk.c libDisk.h tinyFS.h TinyFS_errno.h

# libDisk.o: libDisk.c libDisk.h
# 	$(CC) $(CFLAGS) -c -o $@ $<

# # diskTest.o: diskTest.c libDisk.h
# # 	$(CC) $(CFLAGS) -c -o $@ $<

CC = gcc
CFLAGS = -Wall -g -std=c99
PROG = tinyTest
OBJS = libDisk.o tinyFS.o tinyTest.o

$(PROG): $(OBJS)
	$(CC) $(CFLAGS) -o $(PROG) $(OBJS)

tinyFS.o: tinyFS.c tinyFS.h libDisk.h
	$(CC) $(CFLAGS) -c -o $@ $<

libDisk.o: libDisk.c libDisk.h
	$(CC) $(CFLAGS) -c -o $@ $<

tinyTest.o: tinyTest.c tinyFS.h
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(PROG) $(OBJS)
