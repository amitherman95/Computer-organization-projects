#pragma once
#include <string.h>
// This library is meant to help with string processing


//this function takes a line and removes all the spaces and tabs in the string
void clean_line(char*line, char*buffer)
{
	int i=0;
	int k = 0;
	int length = strlen(buffer);
	for (i = 0; i < length; i++)
	{

		if (buffer[i] == '#')
		{
			break;
		}
		else if (buffer[i] != ' ' && buffer[i] != '	')
		{
			line[k] = buffer[i];
			k++;
		}
		
	}
	line[k] = '\0';

}

//takes string of decimal characters or hex string and converts it int
int string_to_int(char* imm)
{
	return strtol(imm, NULL, 0);
}


// find the first non whitespace character
char* find_non_white_char(char*string)
{
	char*temp;
	temp = string;
	int i;
	int len = strlen(string);
	for (i = 0; i < len; i++)
	{
		if (temp[i] != ' ' && temp[i] != '	' && temp[i] != ',' )
			return (temp + i);
	}
	return temp + i;
}

char* find_first_white_char(char*string)
{
	char*temp;
	temp = string;
	int i;
	int len = strlen(string);
	for (i = 0; i < len; i++)
	{
		if (temp[i] == ' ' || temp[i] == '	' || temp[i] == ',')
			return (temp + i);
	}
	return temp + i;
}



