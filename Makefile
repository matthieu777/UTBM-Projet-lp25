CC=gcc
CFLAGS=-O2 -Wall
LDFLAGS=-L/path/to/openssl -lssl -lcrypto
INC=-I.

all: lp25-backup

%.o: %.c %.h
	$(CC) $(CFLAGS) $(INC) -c $< -o $@

file-properties.o: file-properties.c file-properties.h
	$(CC) $(CFLAGS) -std=c11 $(INC) -c $< -o $@

lp25-backup: main.c files-list.o sync.o configuration.o file-properties.o processes.o messages.o utility.o
	$(CC) $(CFLAGS) $(INC) -o $@ $^ $(LDFLAGS)

clean:
	rm -f *.o lp25-backup
