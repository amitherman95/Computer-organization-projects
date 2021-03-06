#include <stdio.h>
#include "files.h"
#include "definitions.h"
#include "instructions.h"


//main memory array
int RAM [RAM_LENGTH];


//file variables
FILE * asm_file; //*.asm
FILE * o_file;// memin.txt


//instances of each structs
label* head;
components instance;


int main(int argc, char *argv[])
{
	//Creates the head of the list
	head = alloc_label();
	
	char*data=NULL;

	//Read the assembly file
	asm_file = fopen(argv[1], "r");
	if (ferror(asm_file) != 0) {
		printf("An error has occurred\n");
		goto end;
	}
	data = read_file(asm_file);
	fclose(asm_file);


	//Load the labels into the linked list.
	load_labels(data, head);

	//translate the instructions into machine code abd store it in the main memory array
	build_memory_array(data, head, RAM, &instance);
	
	//Output file generation
	o_file = fopen(argv[2], "w");
	if (ferror(o_file) != 0) {
		printf("An error has occurred\n");
		goto end;
	}
	write_o_file(o_file, RAM);
	fclose(o_file);



	//Free allocated data and lists
	end:
	free(data);
	free_list(head);
    return 0;
}

