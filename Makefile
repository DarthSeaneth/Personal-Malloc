all: memgrind test
	
memgrind: memgrind.c
	gcc -g -Wall -Werror mymalloc.c memgrind.c -o memgrind

test: test.c
	gcc -g -Wall -Werror mymalloc.c test.c -o test
	
clean:
	rm -rf memgrind test
