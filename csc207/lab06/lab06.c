#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char **argv){
	if (argc != 3){
		fprintf(stderr, "Usage:filename file1 file2\n" );
		exit(1);
	}
	struct stat filec1, filec2;

	if (stat(argv[1],&filec1) != 0){
		perror("stat");
		exit(1);
	}
	if (stat(argv[2],&filec2) != 0){
		perror("stat");
		exit(1);
	}

	int file_time1 = filec1.st_mtime;
	int file_time2 = filec2.st_mtime;

	if (file_time1 > file_time2){
		printf("%s\n", argv[1]);
	}
	else{
		printf("%s\n", argv[2]);
	}
	return 0;

}
