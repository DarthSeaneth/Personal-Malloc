all: memgrind test err
	
memgrind: memgrind.c
	gcc -g -Wall -Werror mymalloc.c memgrind.c -o memgrind

test: test.c
	gcc -g -Wall -Werror mymalloc.c test.c -o test

err: err.c
	gcc -g -Wall -Werror mymalloc.c err.c -o err
	
clean:
	rm -rf memgrind test err
