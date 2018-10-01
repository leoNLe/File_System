#include "../h_files/helper.h"
#include <ctype.h>
#include <string.h>
#include <stdio.h>

void string_lowered(char *  string) {
  int i = 0;
  while (string[i]) {
    string[i] = tolower(string[i]);
    i++;
  }
}

void remove_new_line(char string[]) {
  int size = (int) strlen(string);
  for(int i = 0; i < size;   i++) {
    if(string[i] == '\n') {
      string[i] = '\0';
    }
  }
}
