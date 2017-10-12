#include <stdio.h>
#include <ctype.h>
#include <string.h>

int main(int argc, char **argv){
	if(argc != 2){
		fprintf(stderr, "usage: progname palindrome\n" );
		return(1);
	}
	char *start = *(argv + 1);
	char *end = *(argv + 1);
	while(*end != '\0'){
		end++;
	}
	end--;

	while(start < end){
		if(!(isalnum(*start))){
			start++;
			continue;
		}
		if(!(isalnum(*end))){
			end--;
			continue;
		}

		if (tolower(*start) == tolower(*end)){
			start++;
			end--;
		}
		else{
			return(1);
		}
	}

	return(0);

}

