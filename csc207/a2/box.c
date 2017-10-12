#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <ctype.h>
#include <sys/types.h>

int main(int argc, char **argv){

	FILE *fp;
	extern void process(FILE *fp, int width, char *c_char, int e_flag);

    char **file_path;
	int e_flag = 0,width = 70;
	char *c_char = "*";
	int i,index;
    while ((i = getopt (argc, argv, "c:ew:")) != -1)
    switch (i)
      {
      case 'e':
        e_flag = 1;
        break;
      case 'c':
        c_char = optarg;
        break;
      case 'w':
        width = atoi(optarg);
        break;
      case '?':
        if (optopt == 'c' || optopt == 'w')
          	fprintf (stderr, "Option -%c requires an argument.\n", optopt);
        else if (isprint (optopt))
          	fprintf (stderr, "Unknown option `-%c'.\n", optopt);
        else
          	fprintf (stderr,
                   "Unknown option character `\\x%x'.\n",
                   optopt);
        	return 1;
    	default:
        	abort ();
    }

    printf ("eflag = %d, wvalue = %d, cvalue = %s\n",
          e_flag, width, c_char);

    int size = argc - optind;
    // printf("This is the size of arg %d\n", size);
    i = 0;
    if(size == i){
        process(stdin, width,c_char,e_flag);
    }
    else{
    file_path = malloc(size * sizeof(char *));
  	for (index = optind; index < argc; index++){
  		file_path[i] = argv[index];
  		printf ("Filepath argument: %s\n", file_path[i]);
  		i++;

  	}

  	for (i = 0; i < width; i++){
  		printf("%s", c_char);
  	}
  	printf("%s\n", c_char);

  	for(i = 0; i < size; i++){
  		if(strcmp(file_path[i], "-") == 0){
  			process(stdin, width,c_char,e_flag);
  		}
  		else{
  			if ((fp = fopen(file_path[i], "r")) == NULL) {
                perror("fopen");
                return(1);
            }
            process(fp, width,c_char,e_flag);
  		}
  	}
    for (i = 0; i < width; i++){
        printf("%s", c_char);
    }
    printf("%s\n", c_char);

  }

  	return 0;
}

void process(FILE *fp, int width, char *c_char, int e_flag)
{
    int i,c;
    int width_left = width;
    while ((c = getc(fp)) != EOF) {
        if(width_left == width){
            printf("%s ", c_char);
            width_left-=2;    
        }
        if(width_left > 2 && c != '\n'){
            putchar(c);
            width_left--;
            // printf("WIDTH IS LESS THICC\n");
            continue;
        }
        if (width_left > 2 && c == '\n'){
            for(i = 0; i < width_left; i++){
                printf(" ");
            }
            printf(" %s\n", c_char);
            width_left = width;
            continue;
        }
        if(width_left == 2 && e_flag == 1 && c != '\n'){
            putchar(c);
            continue;
        }
        if(width_left == 2 && e_flag == 1 && c == '\n'){
            printf("\n");
            width_left = width;
            continue;
        }
        
        if(width_left == 2 && e_flag == 0){
            printf(" %s\n", c_char);
            width_left = width;
            continue;            
        }
        
        

    }

    
}