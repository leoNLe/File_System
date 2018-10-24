#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "../h_files/helper.h"
#include "../h_files/fileSystem.h"

struct input {
  char input [300];
  char *token[5];
  int token_number;
};

void create_a_file(char *name) {
  if(name != NULL) {
	if (create(name) > 0)
    	printf("%s created\n",name );
	else
		printf("Error\n");
  } else {
    printf("Error\n");
  }
}

void to_seek(char * index, char * position) {
	//convert char t int
	int idx = char_arr_to_int(index, strlen(index));
	int pos = char_arr_to_int(position, strlen(position));

  	int cur_pos = lseek(idx, pos);

	if(cur_pos >= 0)
	 	printf("Current position is %d\n", cur_pos );
	else
		printf("error\n");
}

void open_file(char *file_name) {
	if(file_name != NULL) {
		int oft_idx = open(file_name);
		printf("%s was open at %d\n", file_name, oft_idx);
	} else {
		printf("Errsor in opening file\n");
	}
}

void close_file(char *idx_arr) {
	if(idx_arr != NULL) {
		int idx = char_arr_to_int(idx_arr, 4);
		int result = close(idx);
		if(result ){
			printf("%i was closed\n", result);
		} else {
			printf("%i ws NOT closed\n", result);
		}
	} else {
		printf("Enter an index\n");
	}
}

void to_read(char *idx_arr, char * count_arr) {

}

void to_write(char *idx_arr, char *letter, char * count_arr){
	if(idx_arr == NULL || letter == NULL || count_arr == NULL) {
			printf("Error\n");
	} else {
		int idx = char_arr_to_int(idx_arr, 4);

		int count = char_arr_to_int(count_arr, 4);
		printf("here\n");
		if (count > 192 || idx > 4 || idx < 1) {
			printf("Error\n" );
		} else {
			int byte_wrote = write(idx, letter, count);
			if(byte_wrote >= 0) {
				printf("%d bytes was written\n", byte_wrote);
			}
		}
	}
}

void to_destroy(char *name) {
	if(name == NULL) {
		printf("Error\n");
	} else {
		destroy(name);
	}
}

void check_input(struct input *user_input) {
	//TODO remove before submit
	if(user_input->token[0] != NULL) {
		int first_toke_ln = (int) strlen(user_input->token[0]);
		char token_lowered[first_toke_ln];
		memset(token_lowered, '\0', sizeof(first_toke_ln));
		strcpy(token_lowered, user_input->token[0]);

		if(strcmp(token_lowered, "cr" ) == 0) {
			create_a_file(user_input->token[1]);
		} else if(strcmp(token_lowered, "de") == 0) {

			to_destroy(user_input->token[1]);
		} else if(strcmp(token_lowered, "op") == 0) {

			open_file (user_input->token[1]);
		} else if(strcmp(token_lowered, "cl") == 0) {

			close_file(user_input->token[1]);
		} else if(strcmp(token_lowered, "rd") == 0) {

			to_read(user_input->token[1], user_input->token[2]);
		} else if(strcmp(token_lowered, "wr") == 0) {

			to_write(user_input->token[1], user_input->token[2],user_input->token[3]);
		} else if(strcmp(token_lowered, "sk") == 0) {

			to_seek(user_input->token[1], user_input->token[2]);
		} else if(strcmp(token_lowered, "dr") == 0) {

			directory();
		} else if(strcmp(token_lowered, "in") == 0) {

			init(user_input->token[1]);
		} else if(strcmp(token_lowered, "sv") == 0) {

			save(user_input->token[1]);
		} else {
		printf("Command not found.\n" );
		}
	 }

}

void tokenize_input(struct input *user_input) {
  int i = 0;
  char  delim[2] = " ";
  user_input->token[i] = strtok(user_input->input, delim);

  while(user_input->token[i] != NULL && i <= 4) {
    i++;
    user_input->token[i] = strtok(NULL, delim);
  }
  user_input->token_number = i;
}


int main() {
	start_ldisk();
	printf("Shell started.\n");
	struct input user_input;
    while(fgets(user_input.input, 300, stdin) != '\0') {
		remove_new_line(user_input.input);
		printf("%s\n", user_input.input);
		tokenize_input(&user_input);
		check_input(&user_input);
    }

	return 0;
}
