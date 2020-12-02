#Makefile for CSD Board Project
#Use make memcheck to build the server and run with valgrind

CC=gcc
EXEC=server
CFLAGS=-Wall -Werror -Wpedantic
OBJS=server.o util.o
PORT=5555
DIR=serverdir

all: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(EXEC)

server.o: server.c util.h
	$(CC) $(CFLAGS) -c server.c 

util.o: util.c util.h
	$(CC) $(CFLAGS) -c util.c

memcheck: all
	valgrind --leak-check=full --show-leak-kinds=all ./$(EXEC) $(PORT) $(DIR)

clean: 
	rm server.o server
