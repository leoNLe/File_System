#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../h_files/IOInterface.h"
#include "../h_files/fileSystem.h"
#include "../h_files/helper.h"

#define OFT_SIZE 4
#define NUM_PER_DESCRIPTOR 4
#define N_BLOCK_FOR_DESCRIPTOR 6
#define N_DESCRIPTOR 24
#define NAME_SIZE 4
#define LEN_OF_DESCR 16
#define INT_PER_DIR_ENTRY 2
#define SIZE_DIR_INNER_BLOCK 4
#define SIZE_OF_A_DIR 8
#define BM_SIZE 2
//TODO need to do somethign with current position
//OFT declaration;  TODO move it into a struct?
//TODO update write to update filelength in FILE_DESCR too not just OFT_FILE_LEN
char OFT[OFT_SIZE][BLOCK_SIZE];
//current position of the file 0- 191
int OFT_CUR_POS[OFT_SIZE]= {-1};
int OFT_FILE_DESCR[OFT_SIZE] = {-1};
int OFT_FILE_LEN[OFT_SIZE] = {0};

int BM[BM_SIZE];
char FILE_DESCR[BLOCK_SIZE * N_BLOCK_FOR_DESCRIPTOR] = {0};

void error_exit(){
	printf("Error\n");
	exit(1);
}

void update_file_descr(int start, int stop, char *arr_to_copy) {
	for(int i = start, j = 0; i < stop; ++i, ++j) {
		FILE_DESCR[i] = arr_to_copy[j];
	}
}

void  build_mask(int MASK[]) {
	MASK[31] = 1;
	for(int i = 30; i >= 0; --i) {
		MASK[i] = (MASK[i +1]) <<1;
		// printf("%x\n",MASK[i] );
	}

}
int Block_Idx(int pos) {
	return ((pos + BLOCK_SIZE) / BLOCK_SIZE );
}

int get_a_free_block() {
	int MASK[32];
	build_mask(&MASK[0]);
	int free_block= -1;
	int i = 0;
	for( ; i < 2; ++i) {
		for (int j = 0; j < 32 ; ++j){
			int test = BM[i] & MASK[j];
			if(test == 0) {
				free_block = j + 32 * i;
				break;
			}
		}
		if(free_block >=0)
			break;
	}
	return free_block;
}

void use_block(int block) {
	int MASK[32];
	build_mask(&MASK[0]);
	int i = block / 32;
	// printf("i = %i\n", i);
	// printf("BM %x\n", BM[i]);
	// printf("mask #%x\n", MASK[block % 32]);
	BM[i] = BM[i] | MASK[block %32];


}

void free_block(int block) {
	int MASK[32];
	build_mask(&MASK[0]);
	int i = block / 32;

	BM[i] = BM[i] & ~MASK[block];
}

int descr_idx_to_int(int descr_idx){
	return descr_idx * LEN_OF_DESCR;
}


//return -1 if new block was not assigned;
int assign_new_block(int descr, int oft_idx) {
	int nxt_inner_idx = Block_Idx(OFT_CUR_POS[oft_idx]);
	//TODO remove printf
	printf("cur_inner_idx %i\n", nxt_inner_idx);
	int new_block = -1;
	if(nxt_inner_idx + 1 <=3) {
		new_block = get_a_free_block();
		if(new_block > 0) {
			int pos_to_write = descr_idx_to_int(descr) + (nxt_inner_idx+1) * 4;
			int_to_char_arr(new_block, &FILE_DESCR[pos_to_write]);
			//TODO mark block as use here?
		}
	}
	return new_block;
}
int Get_Block_Num_In_Descriptor(
		int FILE_DESCR_idx, int block_idx_in_descr) {
	//file_block_number_idx = what the is index inside the file descriptor;
	// [length(b0), b1, b2, b3]
	return char_arr_to_int(
		&FILE_DESCR[FILE_DESCR_idx * LEN_OF_DESCR
		 				+ (block_idx_in_descr* 4)], 4);
}
void write_cur_oft(int idx ) {
	int cur_inner_idx = Block_Idx(OFT_CUR_POS[idx]);
	int block = Get_Block_Num_In_Descriptor(OFT_FILE_DESCR[idx],
				cur_inner_idx);
	Write_Block(block, OFT[idx]);
}

int load_next_oft(int idx, int pos) {
	int block_idx  = Block_Idx(pos);
	int next_block =  Get_Block_Num_In_Descriptor(OFT_FILE_DESCR[idx],
				block_idx);
	if (next_block  == 0)
		return -1;
	return Read_Block(next_block, OFT[idx]);
}

//return 0 if not needed
// 		 1 if needed and new block was assigned
//		-1 if disk is full
int is_new_block_needed(int index, int cur_pos) {
	int new_block_needed = 0;
	int cur_block_idx = Block_Idx(OFT_CUR_POS[index]);
	int nxt_block_idx = Block_Idx(cur_pos);


	if(cur_block_idx != nxt_block_idx){
		write_cur_oft(index);

		new_block_needed = 1;
		int nxt_block = load_next_oft(index, cur_pos);

		if(nxt_block < 0){
			nxt_block = assign_new_block(OFT_FILE_DESCR[index], index);
			//TODO might cause error
			OFT_CUR_POS[index] = cur_pos;
			if(nxt_block < 0){
				printf("Error. Ldisk full\n");
				new_block_needed = -1;
			}
		}
	}
	return new_block_needed;
}

void load_reserved_blocks() {
	  //Get the bitmap
	 // Read_Block(0, &BIT_MAP[0]);
	  //Loop to pull all file descriptor into
	  int byte_read = 0, j = 0;
	  for(int i = 1; i <= N_BLOCK_FOR_DESCRIPTOR; ++i ) {
		byte_read = Read_Block(i, &FILE_DESCR[j]);

		// if(byte_read == 0)
			// error_exit();
		printf("block %d read %d\n",i, byte_read);

		j += BLOCK_SIZE;
	  }
	  //get first block of the directory
	  // char arr[5] = {0};
	  // strncpy(&arr[0], &FILE_DESCR[4], 4 );
	  // printf("arr has %s whicch is %i\n", arr, char_arr_to_int(&arr[0], 4));
	  int dir_block = char_arr_to_int(&FILE_DESCR[4], 4);
	  Read_Block(dir_block, &OFT[0][0]);
	  OFT_FILE_LEN[0] = char_arr_to_int(&FILE_DESCR[0], 4);
	  OFT_CUR_POS[0] = 0;
	  OFT_FILE_DESCR[0] = 0;
}

void init_reserved_block(){
	//Read_Block()
	memset(FILE_DESCR, '-', sizeof(char));
}

void start_ldisk() {
	for(int i = 0; i < OFT_SIZE; ++i) {
		OFT_FILE_LEN[i] = -1;
		OFT_FILE_DESCR[i] = -1;
		OFT_CUR_POS[i] = -1;
		memset(&OFT[i], '\0', sizeof(OFT[i]));
	}

	for(int i = 0; i < BM_SIZE;++i) {
		if(i ==0)
			BM[i] = 0xfe000000;
		else
			BM[i] = 0x00000000;
	}
	//start Directory
	OFT_FILE_DESCR[0]  = 0;
	OFT_CUR_POS[0] = 0;
	OFT_FILE_LEN[0] = 0;

	init_reserved_block();

	int_to_char_arr(0, &FILE_DESCR[0]);

	//TODO somethign with the ldisk ?


	printf("Disk Initialized\n");
}
//will have to do some error checking
//return 0 if arr points to null and 0;
int char_arr_to_int(char * arr, int length){
	int i = 0;
	int j= length - 1;
	int k = 1;
	while(j >= 0) {
		int a = arr[j] - '0';
		if(a >= 0 && a <= 9) {
			i +=  k * a;
			k *= 10;

		}
		--j;
	}
	return i;
}

int dir_idx_to_int(int i) {
	return i * SIZE_OF_A_DIR;
}


//Return from 1 - x, works b/c index 0 is length;



void int_to_char_arr(int num, char *arr){
	int i = num;
	int temp = 0;
	int j = 0;
	char temp_arr[SIZE_DIR_INNER_BLOCK] ={0};
	while(i != 0) {
		temp = i % 10;
		i = i / 10;
		if(temp == 0) {
			temp_arr[j++] = 48;
		} else {
			temp_arr[j++] = temp + '0';
		}
	}
	if(temp_arr[0] != 0) {
		for(int k = 0; k < SIZE_DIR_INNER_BLOCK; ++k ) {
			arr[k] = temp_arr[SIZE_DIR_INNER_BLOCK-k-1];
		}
	} else {
		arr[j] = 0 +'0';
	}
}

//TODO remove
void print_arr(char *arr, int from, int to) {
	for(int i = from; i < to; ++i) {
		printf("i = %i char is %c \n",i, arr[i]);
	}
	printf("\n");
}

void writing(int OFT_idx, char * arr, int length) {
	for(int i = 0; i < length; ++i) {
		write(OFT_idx, &arr[i], 1);
	}
}

//return negative if no open slot found
int Get_Open_OFT() {
	int open_oft = -1;
	for (int i = 1; i < OFT_SIZE; ++i) {
		if(OFT_FILE_DESCR[i] < 0){
			open_oft = i;
			break;
		}
	}
	return open_oft;
}

int Get_Block_From_File_Idx(int FILE_DESCR_idx, int cur_pos) {
	int block_idx = Block_Idx(cur_pos);
	return char_arr_to_int(
		&FILE_DESCR[FILE_DESCR_idx * LEN_OF_DESCR
		 				+ (block_idx * 4)], 4);
}


int lseek_dir(int pos) {
	int new_pos = -1;
	printf("OFT_FILE_LEN %i\n" , OFT_FILE_LEN[0]);
	if(pos == 0)
		return new_pos;

	int cur_pos_block_idx = Block_Idx(OFT_CUR_POS[0]);
	int new_pos_block_idx = Block_Idx(pos);

	//If pos's block is not the same block loaded in the OFT
	if( cur_pos_block_idx != new_pos_block_idx) {
		write_cur_oft(0);
		//load new block
		if(load_next_oft(0, pos) < 0)
			return -1;
	}
	OFT_CUR_POS[0] = pos;
	printf("lseek_dir CUR_POS %d\n", OFT_CUR_POS[0]);
	new_pos = pos;

	return new_pos;

}

int write_dir(char * mem_area) {
	int byte_wrote = 0;
	int cur_position = OFT_CUR_POS[0];

	//TODO FILE_LENGTH ISSUE
	while( byte_wrote < 4) {
		//if need to change block
		// printf("byte_wrote != count %i\n", byte_wrote != i);
		if(mem_area != 0) {
			OFT[0][cur_position] =  mem_area[byte_wrote];
		} else {
			OFT[0][cur_position] = 0;
		}
		printf("[cur_position] %i byte_wrote %c \n", cur_position,byte_wrote);
		printf("OFT[0][cur_position] %c mem_area[byte_wrote] %c \n", OFT[0][cur_position],mem_area[byte_wrote]);
		byte_wrote++;
		cur_position++;
		//udpate file length
	}
	//TODO Update length in FILE_DESCR.. is this needed? update at close?
	// printf("cur_postion is %i OFT_FILE_LEN is %i OFT_CUR_POS[0] %i\n", cur_position, OFT_FILE_LEN[0], OFT_CUR_POS[0]);
	if(cur_position > OFT_FILE_LEN[0]) {
		//length alwasy increase by 8;
		OFT_CUR_POS[0] = cur_position;
		OFT_FILE_LEN[0] = cur_position;
	}
	return byte_wrote;
}
//always read 4
int read_dir(char * mem_area) {
	int count = 4;
	int byte_read = 0;
	int done = 0;
	int cur_position = OFT_CUR_POS[0];

	while(!done && count > 0) {
		// printf("OTF %c\n", OFT[0][cur_position]);
		// printf("read_dir %i\n", cur_position);
		mem_area[byte_read] = OFT[0][cur_position];
		cur_position++;
		byte_read++;
		if(count == byte_read || cur_position == OFT_FILE_LEN[0]){
			done = 1;
			OFT_CUR_POS[0] = cur_position;
		}
	}
	return byte_read;
}
//Return negative number if file was not found
//Return the index of the directory entry 1-23;
//will not chec for the directory
int Get_Idx_From_Dir(char *name) {
	int file_idx = -1;
	int i = 0;
	if(OFT_CUR_POS[0]!= 0 ) {
		if(OFT_CUR_POS[0] > BLOCK_SIZE) {
			write_cur_oft(0);
			load_next_oft(0,0);
		} else {
			// printf("setting pos back to 0\n");
			OFT_CUR_POS[0] = 0;
		}

	}
	// int cur_pos_idx = 1;

	char file_name[NAME_SIZE] = {0};
	int descr_loc = i * INT_PER_DIR_ENTRY * 4;
	while(file_idx == -1 && i < N_DESCRIPTOR && descr_loc < OFT_FILE_LEN[0]) {
		// printf("descr_loc is %i\n", descr_loc);
		lseek_dir(descr_loc);
		read_dir(&file_name[0]);
		// printf("file_name %s \n",file_name);

		if(strncmp(name, file_name, NAME_SIZE -1) == 0) {
			// printf("equals %i\n", i );
			file_idx = i;
		}
		memset(file_name, 0, sizeof(file_name));
		i++;
		descr_loc = i * INT_PER_DIR_ENTRY * 4;
	}
	return file_idx;
}

int check_open_dir(){
	char name[4] = {0};
	int descr = -1;
	descr = Get_Idx_From_Dir(&name[0]);
	int nxt_descr = (OFT_FILE_LEN[0]+1)/ 8;
	printf("descr is %i nxt_descr %i  with OFT_FILE_LEN %d\n", descr, nxt_descr, OFT_FILE_LEN[0]);
	if(descr < 0 && nxt_descr < N_DESCRIPTOR ) {
		// printf("descr %i nxt_descr %i\n", descr, nxt_descr);
		descr = nxt_descr;
		// printf("after descr %i\n", descr);
	}
	return descr;
}

int get_open_file_descr() {

	int descr_idx = 1;
	int found = 0;
	while( !found && descr_idx <   N_DESCRIPTOR) {
		if(FILE_DESCR[descr_idx * LEN_OF_DESCR] == '\0') {
			found = 1;
			break;
		} else {
			descr_idx++;
		}
	}
	if(!found) {
		descr_idx = -1;
	}
	// printf("descri %i\n", descr_idx);
	return descr_idx;
}

int create(char name[]) {
  	int is_success = 0;
	//TODO may check as needed bitmap
	if(strlen(name) >= NAME_SIZE) {
		//TODO change to error
		printf("Name has to be less then 4\n");
	} else {
		//check if name is already in the file
		if(Get_Idx_From_Dir(name) >= 0) {
			printf("%s already in file\n", name);
			return is_success;
		}
		//check the file descriptor for an open entry
		//check the directory for open entry
		//fill the file descriptor with length = 0; b1,2,3 = 0 block;
		//fill directory with name and the idx of the file descriptor
	    int descr_idx = get_open_file_descr();

		if(descr_idx < 0) {
			printf("No open file descriptor.\n");
			descr_idx = 0;
		} else {
			//Get file name that start with null;
			char block_arr[NAME_SIZE ] = {0};
			int dir_idx = check_open_dir();
			if(dir_idx < 0) {
				printf("No open slot in dir.\n" );
				descr_idx = 0;
			} else {
				//Attempt to go to the correct directory;
				// printf("dir_idx %i\n", dir_idx)
				printf("Before 1 lseek dir_idx pos %d\n", dir_idx);
				printf("Before 1 lseek OFT Current pos %d\n", OFT_CUR_POS[0]);
				if(lseek_dir(dir_idx * 8) < 0) {
					int new_block = assign_new_block(0, 0);
					if(new_block < 0)
						return is_success;

					lseek_dir(dir_idx * 8);
				}

				// printf("OFT Current %d\n", OFT_CUR_POS[0]);
				printf("dir_idx %i\n", dir_idx);
				name[NAME_SIZE-1] = '\0';
				printf("Before 1 write OFT Current pos %d\n", OFT_CUR_POS[0]);
				write_dir(name);
				printf("After 1 write OFT Current pos %d\n", OFT_CUR_POS[0]);


				int_to_char_arr(descr_idx, &block_arr[0]);
				printf("Before 2 write OFT Current pos %d\n", OFT_CUR_POS[0]);

			    write_dir(block_arr);
				printf("After 2 write OFT Current pos %d\n", OFT_CUR_POS[0]);

				int_to_char_arr(0, &FILE_DESCR[descr_idx * LEN_OF_DESCR]);
				is_success = descr_idx;
			}
		}
		//TODO remove when done testing
			// printf("OFT \n");
			// print_arr(OFT[0],0, 64);
			// print_arr(&FILE_DESCR[0], 16 , 64);
			// printf("placed in %d descriptor\n", descr_idx);
			// printf("OFT Current pos %d\n", OFT_CUR_POS[0]);
			// printf("OFT File length %d\n", OFT_FILE_LEN[0]);


	}
  	return is_success;
}
//
int destroy(char name[]) {
	int dir_idx = Get_Idx_From_Dir(&name[0]);
	if( dir_idx < 0) {
		printf("error\n");
	} else  {
		//get the descr_idx from dir entry
		char temp_arr[SIZE_OF_A_DIR]= {'\0'};
		int dir_idx_loc = dir_idx_to_int(dir_idx);
		lseek(0, dir_idx_loc + 4);
		read(0, temp_arr, 4);
		int descr_idx = char_arr_to_int(temp_arr, strlen(temp_arr));

		//remove dir entry
		memset(temp_arr, '\0', sizeof(temp_arr));
		lseek(0, dir_idx_loc);

		writing(0, temp_arr, 4);

		//remove descr entry
		//update bit BIT_MAP
		int descr_loc = descr_idx_to_int(descr_idx);
		for(int i = 1; i < NUM_PER_DESCRIPTOR; ++i) {
			int block = char_arr_to_int(&FILE_DESCR[descr_loc + (i*4)], 4);
			int_to_char_arr(0, &FILE_DESCR[descr_loc+ (i*4)]);
			//TODO free block on bitmap
			printf("free block %d\n", block);

		}
	}
	return 0;
}

int open(char name[]) {
  //check directory for the file descriptor no.
  //go to the file descriptor to get [length, b1, b2, and b3]
  //place b1 and file descriptor into an open OFT slot
  //return non-zero if success. 0 if failure.
	int open_oft = -1;
	int file_idx = -1;

	if(strlen(name) > NAME_SIZE) {
		printf("File Name cannot be bigger than 4.\n" );
		return open_oft;
	} else {
		file_idx = Get_Idx_From_Dir(&name[0]);
	}

	if(file_idx >= 0) {
		open_oft = Get_Open_OFT();
		printf("open %i\n", open_oft);
		if(open_oft > 0) {
			OFT_FILE_DESCR[open_oft] = file_idx;
			//4 is the length of an int [...0, 1, 2,3..] = > 123;
			OFT_FILE_LEN[open_oft] = char_arr_to_int(
					&FILE_DESCR[file_idx * LEN_OF_DESCR], 4);
			OFT_CUR_POS[open_oft] = 0;
			//TODO Remove print
			printf("length of %s is %d at descriptor %d\n", name, OFT_FILE_LEN[open_oft], file_idx);
			//check if length isnt = 0 then read the first block
			if (OFT_FILE_LEN[open_oft]!=0) {
				int first_block =  char_arr_to_int(
							&FILE_DESCR[file_idx * LEN_OF_DESCR + 4], 4);
				Read_Block(first_block, &OFT[open_oft][0]);
			}
		} else {
			printf("No open file slot\n");
		}
	} else {
		printf("File does not exist\n");
	}
	return open_oft;

}

//Should be done
int close(int idx) {
	//Remember to zero out the OFT[idx] when done writing
	//get file
	int block_number = Get_Block_From_File_Idx(OFT_FILE_DESCR[idx], OFT_CUR_POS[idx]);
	//TODO update file_length;
	printf("OFT in index %i saving to block %i %s\n", idx, block_number, OFT[idx]);
	Write_Block(block_number, &OFT[idx][0]);
	printf("OFT %i has file_length of %d\n", idx, OFT_FILE_LEN[idx]);
	int_to_char_arr(OFT_FILE_LEN[idx],&FILE_DESCR[OFT_FILE_DESCR[idx] * LEN_OF_DESCR]);
	memset(OFT[idx], '\0', sizeof(OFT[idx]));

	OFT_FILE_DESCR[idx] = -1;
	OFT_CUR_POS[idx] = -1;
	OFT_FILE_LEN[idx] = -1;
	return 1;
}

//Read from OFT to a mem_area;
int read(int index, char *mem_area, int count){
	//Compute position in r/w buffer
	int byte_read = 0;
	int done = 0;
	int cur_position = OFT_CUR_POS[index];

	while(!done && count > 0) {

		mem_area[0] = OFT[index][cur_position];
		cur_position++;

		if(count == byte_read || cur_position == OFT_FILE_LEN[index]){
			done = 1;
			OFT_CUR_POS[index] = cur_position;
		} else if(cur_position % BLOCK_SIZE  == 0 && byte_read != count) {
			int next_block_idx_in_descr = Block_Idx(cur_position);

			int cur_block = Get_Block_Num_In_Descriptor(OFT_FILE_DESCR[index],
							Block_Idx(OFT_CUR_POS[index]));
			int next_block = Get_Block_Num_In_Descriptor(OFT_FILE_DESCR[index],
							next_block_idx_in_descr);
		    if(next_block != 0 ) {
				Write_Block(cur_block, &OFT[index][0]);
				Read_Block(next_block, &OFT[index][0]);
			} else{
				done = 1;
			}
		}

	}
	return byte_read;
}



int write(int index, char *mem_area, int count){
	int byte_wr = 0;
	int cur_pos = OFT_CUR_POS[index];
	int completed = 0;

	if( index > 3 || index <= 0  ||OFT_FILE_DESCR[index] < 0 ) {
		printf("error\n");
		return -1;
	}
	if(count == 0){
		return byte_wr;
	}

	while(!completed) {
		//check position if new block needed;
		if(is_new_block_needed(index, cur_pos) == -1){
			return byte_wr;
		}

		OFT[index][cur_pos % 64] = mem_area[0];
		byte_wr ++;
		cur_pos++;
		//update file descriptor length
		if(cur_pos > OFT_FILE_LEN[index]){
			OFT_FILE_LEN[index] = cur_pos;
			int len_pos = descr_idx_to_int(OFT_FILE_DESCR[index]);
			int_to_char_arr(OFT_FILE_LEN[index], &FILE_DESCR[len_pos]);
		}

		if(count == byte_wr || OFT_FILE_LEN[index] == (BLOCK_SIZE * 3)) {
			OFT_CUR_POS[index] = cur_pos;
			completed = 1;
		}
	}
	return byte_wr;
}

// -1 if Error
// -2 end of block set position to file_length TODO should this be?
// else return new position
int lseek(int index, int pos) {

	int new_pos = -1;
	// make sure there is something in the OFT idx
	if(OFT_FILE_DESCR[index] >= 0 && index < OFT_SIZE && index >= 0) {
		//check if pos want to seek to is not larger than file;
		if(pos  >  OFT_FILE_LEN[index]) {
			OFT_CUR_POS[index] = OFT_FILE_LEN[index];
			return -1;
		}
		int cur_pos_block_idx = Block_Idx(pos);
		int new_pos_block_idx = Block_Idx(OFT_CUR_POS[index]);

		//If pos's block is not the same block loaded in the OFT
		if( cur_pos_block_idx != new_pos_block_idx) {

			int f_descr_idx = OFT_FILE_DESCR[index];

			int block_to_load = Get_Block_Num_In_Descriptor(f_descr_idx,
								new_pos_block_idx);
			int block_to_write = Get_Block_Num_In_Descriptor(f_descr_idx,
								cur_pos_block_idx);
			// printf("lseek cur_pos_block_idx %i\n",cur_pos_block_idx);
			// printf("lseek new_pos_block_idx %i\n",new_pos_block_idx);
		    // printf("lseek block to load %i\n",block_to_load);
			// printf("lseek block to write %i\n",block_to_write);

			//if nothing is there to load
			if(block_to_load == 0){
				return -1;
			}

			if(Write_Block(block_to_write, &OFT[index][0]) < 0) {
				printf("Unable to save in lseek\n");
				return 0;
			}
			if(Read_Block(block_to_load, &OFT[index][0]) < 0) {
				printf("Unable to lseek new block\n");
				return 0;
			}
			//TODO remove printf
			// printf("block %d was loaded\n", block_to_load);

		}
			OFT_CUR_POS[index] = pos;
			new_pos = pos;

	} else if( OFT_FILE_DESCR[index] < 0) {
		printf("No file loaded in that index\n");
	} else {
		printf("Error in index \n");
	}
	return new_pos;
}

//TODO check if format is correct;
void directory() {
	//print none empty file Name
	if (lseek(0, 8) == -1) {
		printf("Something wrong in reading dir\n");
	} else {
		char name[4]= {0};
		for(int i = 0; i < N_DESCRIPTOR; ++i) {
			strncpy(name,&OFT[0][i * 8], 4);
			if(name[0] != '\0' ) {
				printf("file: %s\n", name);
			}
			lseek(0, descr_idx_to_int(i));
		}
	}
}

int load_file(FILE * file) {
	char arr[BLOCK_SIZE]= {0};
	int read = 0;

	for(int i = 0;i < NO_OF_BLOCK; ++i) {
		read += fread(arr, sizeof(arr), 1, file);
		printf("Block %i\n%s\n", i, arr);
		for(int j = 0; j < BLOCK_SIZE; ++j) {
			if(arr[j] == ' ')
				arr[j] = 0;
		}
		Write_Block(i, &arr[0]);
		memset(arr,0, sizeof(arr));
	}
	//TODO Load the data into OFT
	load_reserved_blocks();
	return read;
}

void init(char * file_name) {
	if(file_name != NULL) {
		FILE *file = fopen(file_name, "r");
		if(file == NULL) {
			printf("Unable to load file start new ldisk\n");
			start_ldisk();
		} else {
			int size = load_file(file);
			if(size == 0) {
				start_ldisk();
			}
		}
	} else {
		start_ldisk();
	}
}

void save(char *file_name) {
	FILE *file =  NULL;
	//Close all OFT
	for(int i = 0; i < OFT_SIZE; ++i) {
		if(OFT_FILE_DESCR[i] >=0) {
			close(i);
		}
	}

	//write descriptor back to ldisk
	for(int i = 1 , j = 0; i < N_BLOCK_FOR_DESCRIPTOR; ++i){
		Write_Block(i, &FILE_DESCR[j]);
		j += 64;
	}

	char temp[BLOCK_SIZE] = {0};
	if(file_name != NULL) {
		//Open file and save it
		file = fopen(file_name, "w+");
		printf("null\n");
	} else {
		file = fopen("file1.txt", "w+");
	}
	for(int i = 0; i < NO_OF_BLOCK; ++i) {
		Read_Block(i, &temp[0]);
		for(int j  = 0; j < BLOCK_SIZE; ++j) {
			if(temp[j] == 0 ){
				char temp_1 = ' ';
				fwrite(&temp_1, sizeof(char), 1, file);
			} else {
				fwrite(&temp[j], sizeof(char), 1, file);
			}
			fflush(0);
		}
		memset(temp,'\0', sizeof(temp));
	}
	printf("dumped\n");
	fflush(0);
	fclose(file);
}
