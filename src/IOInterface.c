#include "../h_files/IOInterface.h"

char ldisk[NO_OF_BLOCK][BLOCK_SIZE];
//Read block i with ldisk[i][0] == char *p;
int Read_Block(int i , char *p) {
	  int j = 0;
	  while(j < BLOCK_SIZE) {
		p[j] = ldisk[i][j];
	    j++;
	  }
	  return j;
}

int Write_Block(int i ,char *p) {
	  int j = 0;
	  while(j < BLOCK_SIZE) {
	    ldisk[i][j] = p[j];
	    j++;
	  }
	  return j;
}
