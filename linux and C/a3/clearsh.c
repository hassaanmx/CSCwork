#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>

static int eflag = 0;
static int num = 10;
static int var = 0;
extern void getaddress(char **args);
extern void execute(char *command, char **args);
extern void freememory(char ** args);
extern char *parsecommand(char *command);
static char *envp[] ={NULL};


int main(int argc, char **argv){


	if(argc == 2){
		if(strcmp(argv[1],"-e") != 0){
		fprintf(stderr, "usage: %s [-e]\n", argv[0]);
		return(1);
		}
		else{
			eflag = 1;
		}
	}
	else if (argc > 2){
		fprintf(stderr, "usage: %s [-e]\n", argv[0]);
		return(1);
	}
	char **args = malloc(sizeof(char *) * 10);

	while(1){
		if(feof(stdin)){
			// printf("feof reached\n");
			fflush(stdout);
			break;
		}
		getaddress(args);
		// int i;
		// for(i = 0; i < var;i++){
		// 	printf("%s ",args[i]);
		// }
		// printf("var = %d\n", var);
		if(feof(stdin)){
			// printf("feof reached\n");
			fflush(stdout);
			break;
		}
		char *string = parsecommand(args[0]);
		if(string == NULL){
			continue;
		}
		execute(string, args);
		// printf("executed\n");
		var = 0;
		free(string);


	}
	freememory(args);
	return(0);

}

void getaddress(char **args){
	while(1){
		if(var == num){
			num+=10;
			args = realloc(args, num  * sizeof (char *));
		}

		args[var] = malloc(sizeof(char)*1024);
		if(fgets(args[var],1024,stdin) != NULL){
			strtok(args[var], "\n");
			// printf("%s\n", args[var]);
			if(strcmp(args[var], "\n") == 0){
				args[var] = NULL;
				// printf("enter reached\n");
				break;
			}
			
		}
		else{
			args[var] = NULL;
			break;
		}


		var++;
	}
}

void execute(char *command, char **args){
	// printf("prog reached here\n");
	int x = fork();
	    if (x == -1) {
	        perror("fork");
	        return;
	    } else if (x == 0) {
	        /* child */
	        execve(command,args,envp);
	        fflush(stdout);
			perror(args[0]);
			return;
	    } else {
	    	/* parent */
	    	int returnStatus;    
    		waitpid(x, &returnStatus, 0);
	        if(eflag == 1 && returnStatus != 0){
	        	printf("exit status %d\n", WEXITSTATUS(returnStatus));
	        }
	        return;
	    }
}

void freememory(char **args){
	// printf("freeing memory\n");
	int i;
	for(i = 0;i < num; i++){
		free(args[i]);
	}
	free(args);
}

char *parsecommand(char *command){
	char *retstrng = malloc(sizeof(char)*1034);
	if(feof(stdin) || (command == NULL)){
		return NULL;
	}

	if (strchr(command, '/')){
		return command;
		}
	else{
		struct stat test;
		char string[1034] = "/bin/";
		strncat(string, command, 1024);
		if(stat(string,&test) == 0){
			strcpy(retstrng,string);
			return retstrng;
		}
		strcpy(string,"/user/bin/");
		strncat(string, command, 1024);
		if(stat(string,&test) == 0){
			strcpy(retstrng,string);
			return retstrng;
		}
	}
	free(retstrng);
	return command;

}
