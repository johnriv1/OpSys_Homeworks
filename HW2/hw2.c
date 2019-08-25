#include <stdio.h>  /* printf */
#include <stdlib.h> /* EXIT_SUCCESS, EXIT_FAILURE, malloc, free */
#include <string.h> /* strlen, strcpy, etc. */
#include <linux/limits.h> /* PATH_MAX */
#include <unistd.h> /* getcwd */

/*void string_parse(char* string_input, char** argv) {

*/}

int main()
{
	while (1) 
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
		
		#ifdef DEBUG_MODE
			printf("%s", user_input);
		#endif
		
		/* parse user input */
		/* Each element of array corresponds to an argv ( argv[0], argv[1], etc.) */
		/* char** argv = calloc(1, sizeof( char* )); */
		char** argv = NULL;
		int arguments_num = 0;
		
		/* buffer for argument */
		char* arg = calloc(65, sizeof( char ));
		for (int i = 0; i < strlen(user_input); i++) 
		{
		   int arg_index = strlen(arg);
         /* space character seperates the arguments/command*/
         /* space means the end of an argument. Also last letter means end of an argument*/
			if ((user_input[i] == ' ') || ( i == (strlen(user_input) - 1)))
			{
				/*only store argument if argument was recorded (arg has something stored)*/	
				if (strlen(arg) > 0) 
				{
				   arguments_num += 1;
					/*if this is the first argument/command, allocate*/
					if (argv == NULL) 
					{
						argv = calloc(arguments_num, sizeof( char* ));
						if ( argv == NULL )
  						{
							fprintf( stderr, "ERROR: calloc() failed\n" );
							return EXIT_FAILURE;
						}
					}
					/*else, reallocate*/
					else
					{
						argv = realloc(argv, arguments_num*sizeof(char*));
						if ( argv == NULL )
  						{
							fprintf( stderr, "ERROR: realloc() failed\n" );
							return EXIT_FAILURE;
						}
					}
					argv[arguments_num - 1] = calloc(strlen(arg)+1, sizeof( char ));
					if ( argv[arguments_num - 1] == NULL )
  					{
						fprintf( stderr, "ERROR: calloc() failed\n" );
						return EXIT_FAILURE;
					}
					strcpy(argv[arguments_num - 1], arg);
					memset(arg, 0, 65);
				}
			}
			else
			{
				arg[arg_index] = user_input[i];
			}
		}
		
		free(user_input);
		free(arg);
		
		#ifdef DEBUG_MODE
			printf("number of arguments: %d \n", arguments_num);
			printf("arguments are: \n");
			for (int i = 0; i < arguments_num; i++)
			{
				printf("%s\n", argv[i]);
			}
		#endif
		
		char* mypath = getenv("MYPATH");
		if (mypath != NULL) 
		{
		
		}
		
		#ifdef DEBUG_MODE
		printf("Path is %s\n", mypath);
		#endif
		
		for (int i = 0; i < arguments_num; i++)
		{
			free(argv[i]);
		}
		free(argv);

	}	
}
   
