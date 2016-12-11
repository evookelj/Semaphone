GCC = gcc -g

all: resources.c telephone.c
	$(GCC) resources.c -o resources
	$(GCC) telephone.c -o telephone

