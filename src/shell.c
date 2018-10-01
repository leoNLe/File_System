#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "../h_files/helper.h"

struct input {
  char input [300];
  char *token[5];
  int token_number;
};

void check_input(struct input *user_input) {
  //TODO remove before submit
  for (int i = 0; i < user_input->token_number; ++i) {
    printf("%s ", user_input->token[i]);
  }
  int first_toke_ln = (int) strlen(user_input->token[0]);
  char token_lowered[first_toke_ln];
  memset(token_lowered, '\0', sizeof(first_toke_ln));
  strcpy(token_lowered, user_input->token[0]);

  if(strcmp(token_lowered, "cr" ) == 0) {
      printf("found\n" );
  // } else if(strcmp(token_lowered, "de", first_toke_ln) == 0) {
  //
  // } else if(strcmp(token_lowered, "op", first_toke_ln) == 0) {
  //
  // } else if(strcmp(token_lowered, "cl", first_toke_ln) == 0) {
  //
  // } else if(strcmp(token_lowered, "rd", first_toke_ln) == 0) {
  //
  // } else if(strcmp(token_lowered, "wr", first_toke_ln) == 0) {
  //
  // } else if(strcmp(token_lowered, "sk", first_toke_ln) == 0) {
  //
  // } else if(strcmp(token_lowered, "dr", first_toke_ln) == 0) {
  //
  // } else if(strcmp(token_lowered, "in", first_toke_ln) == 0) {
  //
  // } else if(strcmp(token_lowered, "sv", first_toke_ln) == 0) {
  //
  } else {
    printf("Command not found.\n" );
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

void get_user_input(struct input *user_input) {

  printf("%%");
  fgets(user_input->input, 300, stdin);
  remove_new_line(user_input->input);
  tokenize_input(user_input);
  check_input(user_input);
  printf("\n");
}

void start_shell() {
  struct input user_input;
  printf("Shell started.\n");
  while(1) {
    get_user_input(&user_input);
  }
}

int main () {
  start_shell();
  return 0;
}
