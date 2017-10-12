#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv){
	if (argc != 2){
	fprintf(stderr,"2 arguments need to be persent\n");
	return 1;
	}	
	
	else{
	int n = atoi(argv[1]);
	if (n > 47){
		fprintf(stderr,"Number cannot be larger than 47\n");
		return 1;
	}	
	int fib[48];
	fib[0] = 0;
	fib[1] = 1;
	int i;
	for (i = 2; i < 47; i++){
		fib[i] = 0;
		fib[i] += fib[i-1];
		fib[i] += fib[i-2];
		}

	printf("%d\n", fib[n]);
	}

	return 0;
}
