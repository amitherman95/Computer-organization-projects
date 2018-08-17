#pragma once
//This header holds definitions of constants and structors

#define RAM_LENGTH 65536


struct label { //The assembler saves the labels and their numbers into linked lists
	char label_name[51];
	int number;
	struct label * next;
};
const char* registers[16];
const char* instruct[16];

//The program seperates the different components in one instruction line.
struct components
{
	char*arg1; //usally op code
	char *arg2;// usually rd
	char *arg3;//rs
	char *arg4;//rt
	char* arg5;// imm
};
