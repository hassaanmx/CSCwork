/*
 * squ -- "squeeze" adjacent blank lines.  That is, the output has no more
 *        than one blank line in a row, but is otherwise a copy of the input.
 */

#include <stdio.h>

int main()
{
    int c;
    enum { START, SAW_NL, SAW_TWO_NL } state = START;

    while ((c = getchar()) != EOF) {
    	if (c == '\n'){
    		if(state == START){
    			state = SAW_NL;
    			putchar(c);
    		}
    		else if(state == SAW_NL){
    			state = SAW_TWO_NL;
    			putchar(c);
    		}
    	}
    	else{
    		putchar(c);
    		state = START;
    	}
    }
}
