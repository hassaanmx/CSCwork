#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>

int main(int argc, char **argv){
	extern int check(FILE *dp);
	extern void traverse(DIR *dir, char *dirname);
	extern char *makepath(char *dirname, char *filename);
	if (argc != 2){
		fprintf (stderr, "Usage: progname dirname");
		return(1);
	}
	struct stat checker;
	if(lstat(argv[1],&checker) == -1){
			perror("lstat");
			return(1);
		}
	if(S_ISREG(checker.st_mode)){
		fprintf (stderr, "Usage: progname dirname");
		return(1);
	}
	DIR *dir = opendir(argv[1]);
	if (dir == NULL){
		perror("opendir");
		return(1);
	}

	traverse(dir, argv[1]);
 	return 0;

}

int check(FILE *file){
	// printf("check started\n");
	int retval;
	unsigned char buf[4] = {0};
	unsigned char check[4] = {127, 'E', 'L', 'F'};
	retval = fread(&buf, 1, 4, file);
		if(retval == 4){
			int i;
			for(i = 0; i < 4;i++){
				if(buf[i] != check[i]){
					return(1);
				}
			}
		}
		else if(ferror(file)){
			printf("Error occured while reading file.\n");
			return(1);
		}
	return(0);	
}

char *makepath(char *dirname, char *filename){
	char *final = malloc(sizeof(char) * 2000);
	sprintf( final, "%s/%s", dirname, filename);
	// printf("%struct\n", final);
	return final;
}

void traverse(DIR *dir, char *dirname){
	struct dirent *dp;
	struct stat checker;
	while((dp = readdir(dir)) != NULL){
		// printf("%s\n", dp->d_name);
		if (dp->d_name[0] != '.') {
			char *path = makepath (dirname, dp->d_name);
			if(lstat(path,&checker) == -1){
				perror("lstat");
				return;
			}
			if(S_ISREG(checker.st_mode)){
				FILE *fp = fopen(path, "r");
				if (fp == NULL){
					perror("fopen");
					return;
				}
				if(check(fp) == 0){
					printf("%s\n", path);
				} 
				if(fclose(fp) != 0){
					perror("fclose");
					return;
				}
			}
			else if(S_ISDIR(checker.st_mode)){
				DIR *dir = opendir(path);
				if (dir == NULL){
					perror("opendir");
					return;
				}
				traverse(dir, path);
			}
		}

	}

}