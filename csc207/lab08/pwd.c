#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <dirent.h>


int main(int argc, char **argv){
	struct stat check;
	
	struct dirent *dp;
	DIR *directory;
	for(;;){
		// While not at root
		if(lstat(".",&check) == -1){
			perror("lstat");
			return(1);
		}
		
		if(check.st_ino == 2){
			printf("we're at the root directory.\n");
			return(0);
		}

		
		if(chdir("..") == -1){
			perror("chdir");
			return(1);
		}
		
		directory = opendir(".");
		if (directory == NULL){
			perror("opendir");
			return(1);
		}
		
		
		dp = readdir(directory);

		if(dp == NULL){
			perror("readdir");
			return(1);
		}
		else{
			if(dp->d_ino == check.st_ino){
				printf("%s\n",dp->d_name);
			}
		}

		while ((dp = readdir(directory)) != NULL){
			if(dp->d_ino == check.st_ino){
				printf("%s\n",dp->d_name);
			}
		}
		
		if (lstat(".",&check) == -1){
			perror("lstat");
			return(1);
		}

	}
	
	
	return 0;
}
