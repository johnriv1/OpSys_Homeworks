#include <stdio.h>  /* printf */
#include <stdlib.h> /* EXIT_SUCCESS, EXIT_FAILURE, malloc, free */
#include <string.h> /* strlen, strcpy, etc. */
#include <linux/limits.h> /* PATH_MAX */
#include <unistd.h> /* getcwd */ /*lstat*/
#include <sys/types.h> /*lstat*/
#include <sys/stat.h> /*lstat*/
#include <sys/types.h> /*waitpid*/
#include <sys/wait.h> /*waitpid*/

int string_parse(char* user_input_, char*** argv, char delimiter) 
{
	/* parse user input */
	/* Each element of array corresponds to an argv ( argv[0], argv[1], etc.) */
	/* char** argv = calloc(1, sizeof( char* )); */
	int arguments_num = 0;
	
	/*ATTACH DELIMETER AT END OF USER_INPUT_ AND THIS FUNCTION SHOULD WORK*/
	char* user_input = calloc((strlen(user_input_)+1+1), sizeof(char));
	strcpy(user_input, user_input_);
	user_input[strlen(user_input_)] = delimiter;
	
	#ifdef DEBUG_MODE
	printf("length of user input is %ld\n", strlen(user_input_));	
	printf("Old user input is %s\n", user_input_);
	printf("New user input is %s\n", user_input);
	#endif
	
	/* buffer for argument */
	char* arg = calloc(65, sizeof( char ));
	for (int i = 0; i < strlen(user_input); i++) 
	{
	   int arg_index = strlen(arg);
      /* space character seperates the arguments/command*/
      /* space means the end of an argument. Also last letter means end of an argument*/
      
		if (user_input[i] == delimiter)
		{
			/*only store argument if argument was recorded (arg has something stored)*/	
			if (strlen(arg) > 0) 
			{
			   arguments_num += 1;
				/*if this is the first argument/command, allocate*/
				if (*argv == NULL) 
				{
					*argv = calloc(arguments_num, sizeof( char* ));
					if ( *argv == NULL )
					{
						fprintf( stderr, "ERROR: calloc() failed\n" );
						return EXIT_FAILURE;
					}
				}
				/*else, reallocate*/
				else
				{
					*argv = realloc(*argv, arguments_num*sizeof(char*));
					if ( *argv == NULL )
					{
						fprintf( stderr, "ERROR: realloc() failed\n" );
						return EXIT_FAILURE;
					}
				}
				(*argv)[arguments_num - 1] = calloc(strlen(arg)+1, sizeof( char ));
				if ( (*argv)[arguments_num - 1] == NULL )
				{
					fprintf( stderr, "ERROR: calloc() failed\n" );
					return EXIT_FAILURE;
				}
				strcpy((*argv)[arguments_num - 1], arg);
				memset(arg, 0, 65);
			}
		}
		else
		{
			arg[arg_index] = user_input[i];
		}
	}
	
	if (arguments_num > 0)
	{
		*argv = realloc(*argv, (arguments_num+1)*sizeof(char*));
		(*argv)[arguments_num] = NULL;
	}
	
	free(user_input);
	free(arg);
	return arguments_num;
}

int main()
{

		char* current_path = calloc(PATH_MAX, sizeof( char ));
		if (getcwd(current_path, PATH_MAX)!= NULL)
		{
			printf("%s$ ", current_path);
		}
		else
		{
			fprintf(stderr, "ERROR: getcwd() failed");
		} 
		char* user_input = calloc(1025, sizeof( char ));
		fgets(user_input, 1025, stdin);
		/* gets rid of superflous newline character from input */
		user_input[strlen(user_input)-1] = '\0';
		
		#ifdef DEBUG_MODE
			printf("%s\n", user_input);
		#endif
		
		/* parse user input */
		/* Each element of array corresponds to an argv ( argv[0], argv[1], etc.) */
		/* char** argv = calloc(1, sizeof( char* )); */
		char** argv = NULL;

		int arguments_num = string_parse(user_input, &argv, ' ');
		int argv_mem_allocd = arguments_num + 1;
		
		int background = 0;
		if (strcmp(argv[arguments_num-1], "&") == 0) {
			background = 1;
			//get rid of '&' for future operations
			printf("HERE:%s\n", argv[arguments_num]);
			printf("ARGUMENTS_NUM BEFORE IS %d\n", arguments_num);
			printf("CONTENTS OF ARGV BEFORE REDUCTION BY 1\n");
			for (int i = 0; i < (arguments_num+1); i++)
			{
				printf("arguments are %s\n", argv[i]);
			}
			
			free(argv[arguments_num]); //this was NULL
			arguments_num--;
			printf("ARGUMENTS_NUM AFTER IS %d\n", arguments_num);
			argv = realloc(argv, (arguments_num+1)*sizeof(char*));
			printf("CONTENTS OF ARGV AFTER REDUCTION BY 1\n");
			for (int i = 0; i < (arguments_num + 1); i++)
			{
				printf("arguments are %s\n", argv[i]);
			}
			argv[arguments_num] = NULL;
			printf("CONTENTS OF ARGV AFTER CONVERSION\n");
			for (int i = 0; i < (arguments_num + 1); i++)
			{
				printf("arguments are %s\n", argv[i]);
			}
			#ifdef DEBUG_MODE
			printf("\nRUN IN BACKGROUND\n");
			printf("AFTER GETTING RID OF &\n");
			printf("number of arguments: %d \n", arguments_num);
			printf("arguments are: \n");
			for (int i = 0; i < arguments_num; i++)
			{
				printf("%s\n", argv[i]);
			}
			printf("\n");
			#endif
			
		}
		
		free(user_input);
		free(current_path);
		/*
		for (int i = 0; i < (arguments_num + 1); i++)
		{
			printf("arguments are %s\n", argv[i]);
		}
		*/
		/* free up arguments memory */
		for (int i = 0; i < (arguments_num + 1); i++)
		{
			free(argv[i]);
		}
		free(argv);
}
