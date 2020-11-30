all: 
	gcc -Wall -Werror -Wpedantic server.c

run: 
	./a.out 5000 .

memcheck: 
	valgrind --leak-check=full --show-leak-kinds=all ./a.out 4000 .

clean: 
	rm a.out
