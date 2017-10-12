#include <stdio.h>
#include <ctype.h>
#include <string.h>

char* mystrstr(char *str, char *substring){
	while(*str != '\0'){
		char *start = str;
		char *sub = substring;
		while(*str && *sub && *str == *sub){
			str++;
			sub++;
		}

		if(*sub == '\0'){
			return start;			
		}

		str = start + 1;

	}

	return NULL;
}

