all:
	gcc main.c -o exe -Wall
san:
	gcc -fsanitize=address main.c -o exe -Wall
chf:
	./checkpatch.pl -f -no-tree main.c
chs:
	cppcheck --enable=all --inconclusive --std=posix main.c
