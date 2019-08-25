#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>

#include <sys/select.h>			/* <===== */

#define BUFFER_SIZE 1024
#define MAX_CLIENTS 32			/* <===== */

int string_parse(char* user_input_, char*** argv, char delimiter, int del_limit) 
{
	/* parse user input */
	/* Each element of array corresponds to an argv ( argv[0], argv[1], etc.) */
	/* char** argv = calloc(1, sizeof( char* )); */
	int arguments_num = 0;
	
	/*ATTACH DELIMETER AT END OF USER_INPUT_ AND THIS FUNCTION SHOULD WORK*/
	char* user_input = calloc((strlen(user_input_)+1+1), sizeof(char));
	strcpy(user_input, user_input_);
	//user_input[strlen(user_input_)] = delimiter;
	
	#ifdef DEBUG_MODE
	printf("length of user input is %ld\n", strlen(user_input_));	
	printf("Old user input is %s\n", user_input_);
	printf("New user input is %s\n", user_input);
	#endif
	int del_count = 0;
	/* buffer for argument */
	char* arg = calloc(65, sizeof( char ));
	for (int i = 0; i < strlen(user_input); i++) 
	{
	   int arg_index = strlen(arg);
      /* space character seperates the arguments/command*/
      /* space means the end of an argument. Also last letter means end of an argument*/
      
		if (user_input[i] == delimiter && del_count < del_limit)
		{
			del_count += 1;
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
	
	if (strlen(arg) > 0)
	{
		arguments_num += 1;
		*argv = realloc(*argv, arguments_num*sizeof(char*));
		(*argv)[arguments_num - 1] = calloc(strlen(arg)+1, sizeof( char ));
		strcpy((*argv)[arguments_num - 1], arg);
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
	char* string = calloc(17, sizeof(char));
	strcpy(string, "Hellomynameis ");
	char** string_split = NULL;
	int arguments_num = string_parse(string, &string_split, ' ', 4);
	for (int i = 0; i < arguments_num; i++)
	{
		printf("|%s|",string_split[i]);
	}
	printf("this many arguments: %d\n", arguments_num);
	printf("\n");
}















