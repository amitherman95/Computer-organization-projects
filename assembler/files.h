#pragma once
#include "definitions.h"
#include "lists.h"
#include <stdlib.h>
#include "string_helper.h"
#include "instructions.h"
//this library deals with file reading and writing

//load all the labels into linked list.


//this function reads the whole file into string and returns a pointer to this string.
char* read_file(FILE*fp)
{
	char * string=NULL;
	int count = 0;
	char c;
	while ((c = getc(fp)) != EOF) 
	{
		string = (char*)realloc(string, count+1); 
		string[count] = c;
		count++;
	}	
	string = (char*)realloc(string, count + 1);
	string[count] = '\0';

	return string;
}

//copy the first line in data to buffer and returns a pointer to the next line.
char* read_line(char*data, char*buffer) {
	int length;
	char * temp;

		temp = strchr(data, '\n');
		if (temp == NULL)
		{
			strcpy(buffer, data);
			return data + strlen(data);
		}
		else
		{
			length = temp - data;
			strncpy(buffer, data, length);
			buffer[length] = '\0';

		}
		return (temp + 1);
	

}
//load labels into linked list
void load_labels(char*data, label *head)
{
	char *ptr;// we'll use it to detect labels
	int count = 0;
	label* label_temp = NULL;
	char* temp;
	char buffer[501];
	char line[501];
	temp = data;

	while (*temp != '\0')
	{
		//	temp = read_line(temp, buffer);
		temp = read_line(temp, buffer);
		clean_line(line, buffer);
		ptr = strchr(line, ':');
		if (ptr != NULL)
		{
			
			*ptr = '\0';
			add_label(head,  line, count);
			head = head->next;
			if (*(ptr + 1) != '\0')
				count++;
		}
		else if (line[0] != '\0' && line[0] != '.')
			count++;

	}

}

void free_comp(components* object)
{
	free(object->arg1);
	free(object->arg2);
	free(object->arg3);
	free(object->arg4);
	free(object->arg5);

	object->arg1 = NULL;
	object->arg2 = NULL;
	object->arg3 = NULL;
	object->arg4 = NULL;
	object->arg5 = NULL;
	return;
}




//there is not miuch to say about this one. for each line of instruction insert a code to the array of memory.
void build_memory_array(char*data, label *head, int memory[], components*instance)
{
	//////////////////////////////////////////////////////
	registers[0] = "$zero";
	registers[1] = "$at";
	registers[2] = "$v0";
	registers[3] = "$a0";
	registers[4] = "$a1";
	registers[5] = "$t0";
	registers[6] = "$t1";
	registers[7] = "$t2";
	registers[8] = "$t3";
	registers[9] = "$s0";
	registers[10] = "$s1";
	registers[11] = "$s2";
	registers[12] = "$gp";
	registers[13] = "$sp";
	registers[14] = "$fp";
	registers[15] = "$ra";

	instruct[0] = "add";
	instruct[1] = "sub";
	instruct[2] = "and";
	instruct[3] = "or";
	instruct[4] = "sll";
	instruct[5] = "sra";
	instruct[6] = "limm";
	instruct[7] = "beq";
	instruct[8] = "bgt";
	instruct[9] = "ble";
	instruct[10] = "bne";
	instruct[11] = "jal";
	instruct[12] = "lw";
	instruct[13] = "sw";
	instruct[14] = "jr";
	instruct[15] = "halt";

	///////////////////////////////////////////////
	char* temp;
	int count = 0;
	char* line =(char*) malloc(501);
	char*ptr;
	char*ptr2;
	int instruct_array[5] = { 0,0,0,0,0 };//basically each element in this array is the proper machine code number of one part of the instruction, i.e. instruct_array[0] = opcode number
		//for each line in file.
	temp = data;
	while (*temp != '\0')
	{
		
		temp = read_line(temp, line);

		//remove comments
		ptr = strchr(line, '#');
		if (ptr != NULL) *ptr = '\0';

		ptr2 = strchr(line, ':');
		if (ptr2 != NULL) //if there is label
			line = ptr2 + 1;

		ptr = find_non_white_char(line);

		if ( *ptr != '\0')// if everything is good and line is not empty 
		{
			process_line(line, instance);
			interpretate(instruct_array, instance, head);
			count += assign_memory(memory, instruct_array, count); //assign_memory detecst if we are dealing with .word command or actual assembly insutrctuction and returns 0  and 1 accordingly, this is the reason for the "count +="
			//printf("%d\n", memory[count-1]);
			free_comp(instance);
		}
		
	}

}


//writes final output file
void write_o_file(FILE*output, int memory[])
{

	int i;

	for (i = 0; i < RAM_LENGTH; i++)
	{
		fprintf(output, "%08x\n", memory[i]);
	}

}