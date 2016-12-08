GCC = gcc -g

all: resources.c
	$(GCC) resources.c -o resources

