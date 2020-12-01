CC=gcc
EXEC=server
CFLAGS=-Wall -Werror -Wpedantic
OBJS=server.o 

all: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(EXEC)

server.o: server.c util.h
	$(CC) $(CFLAGS) -c server.c

run: 
	./a.out 5000 .

memcheck: all
	valgrind --leak-check=full --show-leak-kinds=all ./$(EXEC) 5555 serverdir

clean: 
	rm server.o server
