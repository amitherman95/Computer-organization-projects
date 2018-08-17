#pragma once
#include <string.h>
#include "definitions.h"
#include "string_helper.h"
#include <stdlib.h>
#include <cctype>
#include "lists.h"
//this library deals with instructions and their numerical interpretation

//gets register or instruction and retrieves its offset
//if it fails it just returns -1;
int get_offset(char*item, const char* list[])
{
	
	int i;

	for (i = 0; i < 16; i++)
	{
		if (strcmp(item, list[i]) == 0)
			return i;
	}
	return -1;
}
//creates the machine code for each instruction
int assemble_instruction(int opcode, int rd, int rs, int rt, unsigned short imm)
{

	int result;
	result = (opcode << 28) | (rd << 24) | (rs << 20) | (rt << 16) | imm;
		return result;
}


//assign the proper machine code into the memory array, it returns wether we need to move to next block in the memory array.
int assign_memory(int mem_array[], int instruct_array[], int address)
{
	//[0]  - .word
	if (instruct_array[0] == -1)
	{
		//[1]- address [2] - data
		mem_array[instruct_array[1]] = instruct_array[2];
		return 0; // no, count in parent function remains the same
	}
	//the rest
	else
	{
		mem_array[address] = assemble_instruction(instruct_array[0], instruct_array[1], instruct_array[2], instruct_array[3], instruct_array[4]);
		return 1; // yes count in parent function increases by one.
	}
}




//seperates different part of the instruction
void process_line(char*line, components*inst)
{
	char*temp;
	char*temp2;
	int number;

	//first of all it deals with .word, in this case this macro just handle its two arguments, address and data.
	temp = strstr(line, ".word");
	if (temp != NULL)
	{
		//.word
		inst->arg1 = (char*)malloc(6 * sizeof(char));
		strcpy(inst->arg1, ".word");
		inst->arg1[5] = '\0';

		//address
		temp = find_non_white_char(temp + 5);
		temp2 = find_first_white_char(temp);
		number = temp2 - temp +1;//length
		inst->arg2 = (char*)malloc(number * sizeof(char));
		strncpy(inst->arg2, temp, number-1);
		inst->arg2[number-1] = '\0';

		//data
		temp = find_non_white_char(temp2);
		temp2 = find_first_white_char(temp);
		number = temp2 - temp+1;//length
		inst->arg3 = (char*)malloc(number * sizeof(char));
		strncpy(inst->arg3, temp, number-1);
		inst->arg3[number-1] = '\0';

	}
		// for the rest:
	else
	{
		//opcode
		temp = find_non_white_char(line);
		temp2 = find_first_white_char(temp);
		number = temp2 - temp+ 1;//length
		inst->arg1 = (char*)malloc(number*sizeof(char));
		strncpy(inst->arg1, temp, number-1);
		inst->arg1[number-1] = '\0';

		//rd
		if (*temp2 == '\0')
			goto end;

		temp = find_non_white_char(temp2);
		temp2 = find_first_white_char(temp);
		number = temp2 - temp+1;//length
		inst->arg2 = (char*)malloc(number * sizeof(char));
		strncpy(inst->arg2, temp, number-1);
		inst->arg2[number-1] = '\0';

		//rs
		if (*temp2 == '\0')
			goto end;

		temp = find_non_white_char(temp2);
		temp2 = find_first_white_char(temp);
		number = temp2 - temp + 1;//length
		inst->arg3 = (char*)malloc(number * sizeof(char));
		strncpy(inst->arg3, temp, number-1);
		inst->arg3[number-1] = '\0';

		//rt
		if (*temp2== '\0')
			goto end;

		temp = find_non_white_char(temp2);
		temp2 = find_first_white_char(temp);
		number = temp2 - temp + 1;//length
		inst->arg4 = (char*)malloc(number * sizeof(char));
		strncpy(inst->arg4, temp, number-1);
		inst->arg4[number-1] = '\0';


		//imm
		if (*temp2 == '\0')
			goto end;

		temp = find_non_white_char(temp2);
		temp2 = find_first_white_char(temp);
		number = temp2 - temp + 1;//length
		inst->arg5 = (char*)malloc(number * sizeof(char));
		strncpy(inst->arg5, temp, number-1);
		inst->arg5[number-1] = '\0';

	
	}
end:
	return;

}

void interpretate(int result[], components*inst, label*head)
{
	label* helper;
		int offset;
		if (strcmp(inst->arg1, ".word")==0)
		{
			result[0] = -1;
			result[1] = string_to_int(inst->arg2);
			result[2] = string_to_int(inst->arg3);
			return;
		}


		//arg1, opcode
	if (inst->arg1 != NULL)
	{
		offset = get_offset(inst->arg1, instruct);
		result[0] = offset;

	}

	//arg2, rd
	if (inst->arg2 != NULL)
	{
			offset = get_offset(inst->arg2, registers);
			result[1] = offset;
		

	}
	//arg3, rs
	if (inst->arg3 != NULL)
	{
		offset = get_offset(inst->arg3, registers);
		result[2] = offset;
	}

	//arg4, rt
	if (inst->arg4 != NULL)
	{
		offset = get_offset(inst->arg4, registers);
		result[3] = offset;
	}

	// arg5, imm
		if (inst->arg5 != NULL)
		{
			//if label
			if (isalpha(inst->arg5[0]))
			{
				helper = search_label(head, inst->arg5);
				result[4] = helper->number;
			}
			//if normal value
			else
				result[4] = string_to_int(inst->arg5);
		}



	}
