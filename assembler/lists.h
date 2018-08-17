#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "definitions.h"

//new node
label* alloc_label()
{
	label* new_node;
	new_node = (label*)malloc(sizeof(label));
	new_node->next = NULL;

	return new_node;
}


//This functions search if some label already exists and returns the corresponding node of this label
label* search_label(label * first_node, char* label_name) {
	int out = 0;
	label *temp;
	label *output = NULL;
	int flag=0;
	temp = first_node;

	while (temp != NULL && !flag)
		{
		flag = (strcmp(label_name, temp->label_name)==0);
		if (flag) { output = temp; }
		temp = temp->next;
		}
	return output;

}

//This function add new label to the linked list, it recieves the tail of the list.
//it returns the new node. the tail is replaced by the new node
void add_label( label*tail,  char* new_label, int instruction_num)
{
	
	if (tail != NULL)
	{

		strcpy_s(tail->label_name, new_label);
		tail->number = instruction_num;
		tail->next = alloc_label();
	}
}

void free_list(label* head)
{
	label* next;
	if (head != NULL) {
		next = head->next;
		free(head);
		free_list(next);
	}


}

//this function is for debugging just ignore it.
void print_labels(label*head)
{
	while (head->next != NULL)
	{
		printf("%s:%d\n", head->label_name, head->number);
		head = head->next;
	}

}

