#include "stdlib.h"
#include "stdio.h"
#include "string.h"

struct input {
  char input [300];
  char *token[5];
  int token_number;
};
void check_input(struct input *user_input) {
  for (int i = 0; i < user_input->token_number; ++i) {
    printf("%s ", user_input->token[i]);
  }
}
void parsed_input(struct input *user_input) {
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
  parsed_input(user_input);
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
