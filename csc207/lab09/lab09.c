#include <stdio.h>

int main(int argc, char **argv)
{
    FILE *fp;
    char c;

    if (argc != 2){
    	printf("usage: progname filename");
    	return(1);
    }
	

    fp = fopen(argv[1], "r+");
    if(fp == NULL){
    	perror("fopen");
    	return(1);
    }
    while ((c = getc(fp)) != EOF)
	putchar(c);

    return(0);
}