CC = gcc
CFLAGS = -g -c -Wall 
.c.o:
	$(CC) $(CFLAGS) $< -o $@

all:
	make ush
	make loop
	make lsPipedToSort

ush: wrappers.o ush.o parser.o jobs.o

ush.o: wrappers.h parser.h

wrappers.o: wrappers.h

parser.o: parser.h

jobs.o: jobs.h parser.h

loop: 
	$(CC) loop.c -o loop1
	cp loop1 loop2
	cp loop1 loop3

lsPipedToSort:
	$(CC) lsPipedToSort.c -o lsPipedToSort

clean:
	rm ush *.o loop1 loop2 loop3 lsPipedToSort	
