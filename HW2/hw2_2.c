#include <stdio.h>  /* printf */
#include <stdlib.h> /* EXIT_SUCCESS, EXIT_FAILURE, malloc, free */
#include <string.h> /* strlen, strcpy, etc. */
#include <linux/limits.h> /* PATH_MAX */
#include <unistd.h> /* getcwd */ /*lstat*/
#include <sys/types.h> /*lstat*/
#include <sys/stat.h> /*lstat*/
#include <sys/types.h> /*waitpid*/
#include <sys/wait.h> /*waitpid*/



int string_parse(char* user_input, char*** argv, char delimiter) 
{
	/* parse user input */
	/* Each element of array corresponds to an argv ( argv[0], argv[1], etc.) */
	/* char** argv = calloc(1, sizeof( char* )); */
	int arguments_num = 0;
	
	printf("length of user input is %ld\n", strlen(user_input));
	
	/*ATTACH DELIMETER AT END OF USER_INPUT_ AND THIS FUNCTION SHOULD WORK*/
	/*USER_INPUT = calloc(user_input_ size + 1, sizeof(char)), user_input[strlen(user_input)] = delimeter */
	/*
	char* user_input = calloc((strlen(user_input_)+1), sizeof(char));
	strcpy(user_input, user_input_);
	user_input[strlen(user_input_)] = delimiter;
		
	printf("Old user input is %s\n", user_input_);
	printf("New user input is %s\n", user_input);
	*/
	/* buffer for argument */
	char* arg = calloc(65, sizeof( char ));
	for (int i = 0; i < strlen(user_input); i++) 
	{
	   int arg_index = strlen(arg);
      /* space character seperates the arguments/command*/
      /* space means the end of an argument. Also last letter means end of an argument*/
      
		if ((user_input[i] == delimiter) || ( i == (strlen(user_input) - 1)))
		//if (user_input[i] == delimiter)
		{
		/*
			
			if  ( i == (strlen(user_input)) - 1) {
				arg[arg_index] = user_input[i];
				arg_index = strlen(arg);
			}
		*/
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
	
	free(arg);
	return arguments_num;
}

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

		int arguments_num = string_parse(user_input, &argv, ' ');
		
		if (arguments_num == 0)
		{
			free(user_input);
			free(current_path);
			free(argv);
			continue;
		}
		
		free(user_input);
		
		#ifdef DEBUG_MODE
			printf("number of arguments: %d \n", arguments_num);
			printf("arguments are: \n");
			for (int i = 0; i < arguments_num; i++)
			{
				printf("%s\n", argv[i]);
			}
			printf("\n");
		#endif
		
		/* parse paths that we will be searching: mypath is if it exists and default if it doesnt */
		char* mypath = getenv("MYPATH");
		char* defaultpath = "/bin#.";
		char* searchingpath;
		if (mypath != NULL) 
		{
			searchingpath = mypath;
		}
		else
		{
			searchingpath = defaultpath;
		}
		char **searchingpaths = NULL;
		int paths_num = string_parse(searchingpath, &searchingpaths, '#');
		#ifdef DEBUG_MODE
			printf("MYPATH is %s\n", mypath);
			printf("Default Path is %s\n", defaultpath);
			printf("Number of possible paths %d\n", paths_num);
			printf("Path that will be searched is %s\n", searchingpath);
			for (int i = 0; i < paths_num; i++) 
			{
				printf("One Searching Path is %s\n", searchingpaths[i]);
			}
			printf("\n");
		#endif
		
		/* By the end of this loop, will have correct location of inputted command */
		/* or return not found */
		int found = 0;
		char* searchingpath_and_file = NULL;
		for (int i = 0; i < paths_num; i++) 
		{
			if (searchingpath_and_file == NULL) 
			{
				/* +1 for the '/' character that will connect searchingpaths[i] and argv[0] */
				/* additional +1 for nul terminator */
				searchingpath_and_file = calloc((strlen(searchingpaths[i])+strlen(argv[0])+1+1), sizeof(char));
				//sprintf(searchingpath_and_file, "%s/%s", searchingpaths[i], argv[0]);
			}
			else 
			{
				searchingpath_and_file = realloc(searchingpath_and_file, (strlen(searchingpaths[i])+strlen(argv[0])+1+1)*sizeof(char));
				searchingpath_and_file[(strlen(searchingpaths[i])+strlen(argv[0])+1)] = '\0';
			}
			/*sets searchingpath_and_file to directory../arv[0] which searches for first command*/
			sprintf(searchingpath_and_file, "%s/%s", searchingpaths[i], argv[0]);
			#ifdef DEBUG_MODE
			printf("Currently searching for directory %s\n", searchingpath_and_file);
			printf("Directory is %ld long\n", strlen(searchingpath_and_file));
			#endif
			
			//see if path in searchingpath_and_file exists using lstat
			//if it does, break out of loop. Fork. in Child, use exec. In parent wait?
			struct stat fileinfo;
    		int rc = lstat( searchingpath_and_file, &fileinfo);  
    		if ( rc == -1 )
    		{
    			#ifdef DEBUG_MODE
    			printf("Command not found in %s\n", searchingpath_and_file);
    			#endif
			}
			else 
			{
				found = 1;
				#ifdef DEBUG_MODE
				printf("Command found in %s\n", searchingpath_and_file);
				#endif
				break;
			}
		}
		
		if (found) {
			#ifdef DEBUG_MODE
			printf("Will fork and execute %s\n", searchingpath_and_file);
			#endif
			pid_t pid;
			pid = fork();
			if ( pid == -1 )
			{

				perror( "fork() failed\n" );
				
				return EXIT_FAILURE;
			}
			if ( pid == 0 )
			{				
				#ifdef DEBUG_MODE
				printf( "THIS IS THE CHILD\n");
				#endif
  				execv(searchingpath_and_file, argv);
  			}
  			else /* pid > 0 */
  			{
  				int status;
  				#ifdef DEBUG_MODE
  				printf("THIS IS THE PARENT\n");
  				#endif
  				pid_t child_pid = waitpid( pid, &status, 0 );  /* block indefinitely */
  			}
		}
		else if (strcmp(argv[0], "exit") == 0) {
			free(searchingpath_and_file);
			free(current_path);
			for (int i = 0; i < (paths_num + 1); i++) {
				free(searchingpaths[i]);
			}
			free(searchingpaths);
			/* free up arguments memory */
			for (int i = 0; i < (arguments_num + 1); i++)
			{
				free(argv[i]);
			}
			free(argv);
			printf("bye\n");
			exit(0);
		}
		else if (strcmp(argv[0], "cd") == 0) {
			if (arguments_num > 2) {
				printf("bash: cd: too many arguments\n");
			}
			else if (arguments_num > 1) {
				int cd = chdir(argv[1]);
				if (cd == -1)
				{
					printf("bash: cd: \"%s\": No such file or directory\n", argv[1]);
				}
			}
			else {
					char* homepath = getenv("HOME");
					int cd = chdir(homepath);
			}
		}
		else {
			fprintf(stderr, "ERROR: command \"%s\" not found\n", argv[0]);
		}
		
		free(searchingpath_and_file);
		free(current_path);
		
		for (int i = 0; i < (paths_num + 1); i++) {
			free(searchingpaths[i]);
		}
		free(searchingpaths);
		
		/* free up arguments memory */
		for (int i = 0; i < (arguments_num + 1); i++)
		{
			free(argv[i]);
		}
		free(argv);
	}
}

