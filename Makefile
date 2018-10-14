CC = gcc
EXECUTABLES = project2

all:$(EXECUTABLES)

project1: project2.c
	$(CC) project2.c -o project2


clean:
	rm $(EXECUTABLES) *.o
