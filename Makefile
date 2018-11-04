INC_DIR = ./h_files/
SRC_DIR = ./src/
CFLAGS = -c -Wall

main: shell.o helper.o fileSystem.o IOInterface.o
	gcc shell.o helper.o fileSystem.o IOInterface.o -o main
helper.o: $(INC_DIR)helper.h $(SRC_DIR)helper.c
	gcc $(CFLAGS) $(SRC_DIR)helper.c
shell.o: $(SRC_DIR)shell.c $(INC_DIR)helper.h $(INC_DIR)fileSystem.h
	gcc $(CFLAGS) $(SRC_DIR)shell.c
fileSystem.o: $(SRC_DIR)fileSystem.c $(INC_DIR)fileSystem.h $(INC_DIR)IOInterface.h
	gcc $(CFLAGS) $(SRC_DIR)fileSystem.c
IOInterface.o: $(SRC_DIR)IOInterface.c $(INC_DIR)IOInterface.h
	gcc $(CFLAGS) $(SRC_DIR)IOInterface.c


clean:
	rm -rf *o all
