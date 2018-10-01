INC_DIR = ../h_files/
SRC_DIR = ../src/
CFLAGS = -c -Wall 

main: shell.o helper.o
	gcc shell.o helper.o -o main
helper.o: ./h_files/helper.h ./src/helper.c
	gcc $(CFLAGS) ./src/helper.c
shell.o: ./src/shell.c ./h_files/helper.h
	gcc $(CFLAGS) ./src/shell.c

clean:
	rm -rf *o all	
