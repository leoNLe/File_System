//TODO move the global here?

void start_ldisk();

int load_reserved_block();

void int_to_char_arr(int num, char *arr);

int char_arr_to_int(char * arr, int length);

int create(char name[]);

int destroy(char name[]);

int open(char name[]);

int close(int index);

//count = number of bytes that need to be read
int read(int index, char *mem_area, int count);

int write(int index, char *mem_area, int count);

int  lseek(int index, int pos);

void directory();

void save(char *file_name);

void init(char *file_name);

void writing(int OFT_idx, char * arr, int length);
