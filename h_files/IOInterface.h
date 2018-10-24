#define BLOCK_SIZE  64
#define NO_OF_BLOCK  64

char ldisk[NO_OF_BLOCK][BLOCK_SIZE];

int Read_Block(int i , char *p);
int Write_Block(int i ,char *p);
