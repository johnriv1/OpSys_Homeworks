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

void remove_element(int* array, int index, int *array_size)
{
   for (int i = index; i < ((*array_size) - 1); i++)
   { 
   	array[i] = array[i + 1];
   }
  	(*array_size) --;
}

void get_directory(int* found, char** searchingpath_and_file, char* argv0, int paths_num, char*** searchingpaths)
{
	//int found = 0;
	//char* searchingpath_and_file = NULL;
	for (int i = 0; i < paths_num; i++) 
	{
		if ((*searchingpath_and_file) == NULL) 
		{
			/* +1 for the '/' character that will connect searchingpaths[i] and argv[0] */
			/* additional +1 for nul terminator */
			(*searchingpath_and_file) = calloc((strlen((*searchingpaths)[i])+strlen(argv0)+1+1), sizeof(char));
			//sprintf(searchingpath_and_file, "%s/%s", searchingpaths[i], argv[0]);
		}
		else 
		{
			(*searchingpath_and_file) = realloc(*searchingpath_and_file, (strlen((*searchingpaths)[i])+strlen(argv0)+1+1)*sizeof(char));
			(*searchingpath_and_file)[(strlen((*searchingpaths)[i])+strlen(argv0)+1)] = '\0';
		}
		/*sets searchingpath_and_file to directory../arv[0] which searches for first command*/
		sprintf(*searchingpath_and_file, "%s/%s", (*searchingpaths)[i], argv0);
		#ifdef DEBUG_MODE
		printf("Currently searching for directory %s\n", *searchingpath_and_file);
		printf("Directory is %ld long\n", strlen(*searchingpath_and_file));
		#endif
		
		//see if path in searchingpath_and_file exists using lstat
		//if it does, break out of loop. Fork. in Child, use exec. In parent wait?
		struct stat buf;
 		int rc = lstat( *searchingpath_and_file, &buf);  
 		if ( rc == -1 )
 		{
 			#ifdef DEBUG_MODE
 			printf("Command not found in %s\n", *searchingpath_and_file);
 			#endif
		}
		else 
		{
			/* check if file is executable */				
			if (buf.st_mode & S_IXUSR) {
				*found = 1;
				#ifdef DEBUG_MODE
				printf("Command found in %s\n", *searchingpath_and_file);
				#endif
				break;
			}
			else {
				#ifdef DEBUG_MODE
				printf("Found in %s, but is not executable\n", *searchingpath_and_file);
				#endif
			}
		}
	}
}

int main()
{
	
	setvbuf( stdout, NULL, _IONBF, 0 );
		
	int background_pids[128];
	int background_pids_size = 0;
	while (1) 
	{
		//CHECK FOR BACKGROUND PROCESSES ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		int status;
  		pid_t child_pid;
		if (background_pids_size > 0) 
		{
			for (int i = 0; i < background_pids_size; i++) 
			{
				child_pid = waitpid( background_pids[i], &status, WNOHANG );
  			   /* if child_pid = 0, that means that no child process terminated */
  			   if (child_pid != 0) 
  			   {
  			   	printf("[process %d terminated with exit status %d]\n", child_pid, status);
  			   	remove_element(background_pids, i, &background_pids_size);
  			   }
  			   
  			   /*in case there are 2 process that finish (ie piping) */
  			   if (background_pids_size > 0)
  			   {
	  			   /*in case a second one finishes */
	  			   child_pid = waitpid( background_pids[i], &status, WNOHANG );
	  			   if (child_pid != 0) 
	  			   {
	  			   	printf("[process %d terminated with exit status %d]\n", child_pid, status);
	  			   	remove_element(background_pids, i, &background_pids_size);
	  			   }
  			   }
			}
		}
		#ifdef DEBUG_MODE
			printf("THERE ARE %d MANY BACKGROUND PIDS\n", background_pids_size);
		#endif
	
		//DISPLAY CURRENT PATH AND GET USER INPUT //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
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
		
		/* parse user input */////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/* Each element of array corresponds to an argv ( argv[0], argv[1], etc.) */
		/* char** argv = calloc(1, sizeof( char* )); */
		char** argv = NULL;

		int arguments_num = string_parse(user_input, &argv, ' ');
		
		/* if only enter is pressed, restart to command line prompt again */
		if (arguments_num == 0)
		{
			free(user_input);
			free(current_path);
			free(argv);
			continue;
		}
		
		//free(user_input);
		
		#ifdef DEBUG_MODE
			printf("number of arguments: %d \n", arguments_num);
			printf("arguments are: \n");
			for (int i = 0; i < arguments_num; i++)
			{
				printf("%s\n", argv[i]);
			}
			//printf("\n");
		#endif
		
		/* parse paths that we will be searching: mypath is if it exists and default if it doesnt *//////////////////////////////////////////////////////////////////////////////////////////
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
		
		/*check for '|' are we piping?*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		int piping = 0;
		for (int i = 0; i < (arguments_num); i++) 
		{
			if (strcmp(argv[i], "|") == 0) 
			{
				piping = 1;
			}	
		}
		
		/* split argv into argv1 and argv2 for piping */
		char** argv1 = NULL;
		char** argv2 = NULL;
		char** argv_1_2 = NULL;
		int argv_1_2_num = 0;
		int argv_1_num = 0;
		int argv_2_num = 0;
		if (piping == 1)
		{
			argv_1_2_num = string_parse(user_input, &argv_1_2, '|');
			argv_1_num = string_parse(argv_1_2[0], &argv1, ' ');
			argv_2_num = string_parse(argv_1_2[1], &argv2, ' ');
			/*print this out, see if its right */
			#ifdef DEBUG_MODE			
				for (int i = 0; i < argv_1_2_num; i++) 
				{
					printf("argv_1_2 argument is %s\n", argv_1_2[i]);
				}
				for (int i = 0; i < argv_1_num; i++) 
				{
					printf("argv1 argument is %s\n", argv1[i]);
				}
				for (int i = 0; i < argv_2_num; i++) 
				{
					printf("argv2 argument is %s\n", argv2[i]);
				}
			#endif
		}	
		free(user_input);
		
		/*check for &, are we running this in background?*////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/*background = 0 if process to be run in foreground*/
		int background = 0;
		/*check if & appear in any argument except the last one -> error*/
		for (int i = 0; i < (arguments_num - 1); i++) 
		{
			if (strcmp(argv[i], "&") == 0) {
				fprintf( stderr, "ERROR: \'&\' can only be included at end of command line!\n" );
			}
		}

		if (strcmp(argv[arguments_num-1], "&") == 0) 
		{
			if (piping == 0)
			{
				background = 1;
				//get rid of '&' for future operations
				free(argv[arguments_num]); //this was NULL
				arguments_num--;
				argv = realloc(argv, (arguments_num+1)*sizeof(char*));
				free(argv[arguments_num]);
				argv[arguments_num] = NULL;
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
			else if (piping == 1)
			{
				background = 1;
				//get rid of '&' for future operations
				free(argv2[argv_2_num]); //this was NULL
				argv_2_num--;
				argv2 = realloc(argv2, (argv_2_num+1)*sizeof(char*));
				free(argv2[argv_2_num]);
				argv2[argv_2_num] = NULL;
				#ifdef DEBUG_MODE
				printf("\nRUN IN BACKGROUND\n");
				printf("AFTER GETTING RID OF &\n");
				printf("number of arguments in argv2: %d \n", argv_2_num);
				printf("arguments are: \n");
				for (int i = 0; i < argv_2_num; i++)
				{
					printf("%s\n", argv2[i]);
				}
				printf("\n");
				#endif
			}
		}

		if (piping == 0) 
		{
			int found = 0;
			char* searchingpath_and_file = NULL;
			/* By the end of this loop (in function), will have correct directory of inputted command */////////////////////////////////////////////////////////////////////////////////////
			/* or return not found */
			get_directory(&found, &searchingpath_and_file, argv[0], paths_num, &searchingpaths);
					

			/*IF COMMAND WAS FOUND, FORK AND EXECUTE *//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
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
	  				/*int status;
	  				#ifdef DEBUG_MODE
	  				printf("THIS IS THE PARENT\n");
	  				#endif
	  				pid_t child_pid; */
	  				/*IF COMMAND WAS TO BE RUN IN THE FOREGROUND, BLOCK INDEFINITELY *////////////////////////////////////////////////////////////////////////////////////////////////////
	  				if (background == 0)
	  				{
	  					child_pid = waitpid( pid, &status, 0 );  /* block indefinitely */
	  				}
	  				else /*background = 1, run process in backround*//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	  				{
	  					if (background_pids_size >= 128)
	  					{
	  						fprintf(stderr, "EXCEEDED MAXIMUM AMOUNT OF BACKGROUND PROCESSES\n");
	  					}
	  					printf("[running background process \"%s\"]\n", argv[0]);
	  					background_pids[background_pids_size] = pid;
	  					background_pids_size += 1;
	  					#if 0
	  				   child_pid = waitpid( -1, &status, WNOHANG );
	  				   /* if child_pid = 0, that means that no child process terminated */
	  				   if (child_pid != 0) 
	  				   {
	  				   	printf("[process %d terminated with exit status %d]\n", child_pid, status);	
	  				   }
	  				   #endif
	  				}
	  			}
			}
			/* SPECIAL COMMAND: EXIT *//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			else if (strcmp(argv[0], "exit") == 0) {
				free(current_path);
				free(searchingpath_and_file);
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
			/* SPECIAL COMMAND: CD *///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
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
					if (cd == -1)
					{
						printf("bash: cd: \"%s\": No such file or directory\n", argv[1]);
					}
				}
			}
			/*NO COMMAND FOUND *///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			else {
				fprintf(stderr, "ERROR: command \"%s\" not found\n", argv[0]);
			}
			free(searchingpath_and_file);
		}
		else /*piping = 1 => we must pipe*/
		{
			int found_arg_1 = 0;
			int found_arg_2 = 0;
			char* searchingpath_and_file_1 = NULL;
			char* searchingpath_and_file_2 = NULL;
			get_directory(&found_arg_1, &searchingpath_and_file_1, argv1[0], paths_num, &searchingpaths);
			get_directory(&found_arg_2, &searchingpath_and_file_2, argv2[0], paths_num, &searchingpaths);
			#ifdef DEBUG_MODE
			if (found_arg_1 == 1) {
				printf("Found first command %s\n", argv1[0]);
			}
			else {
				printf("Did not find first command %s\n", argv1[0]);
			}
			if (found_arg_2 == 1) {
				printf("Found second command %s\n", argv2[0]);
			}
			else {
				printf("Did not find second command %s\n", argv2[0]);
			}
			#endif
			if ((found_arg_1 == 1) && (found_arg_2 == 1)) 
			{
				int p[2];   /* array to hold the two pipe (file) descriptors:
                 				p[0] is the read end of the pipe
                 				p[1] is the write end of the pipe */

 				int rc = pipe( p );
                         				
            /* fd 3 is the read end; fd 4 is the write end */
                         				
				if ( rc == -1 )
				{
					perror( "pipe() failed" );
					return EXIT_FAILURE;
				}
				
				pid_t pid1;
				pid1 = fork();
				pid_t pid2;
				
				if ( pid1 == 0 )
				{				
					#ifdef DEBUG_MODE
					printf( "THIS IS FIRST THE CHILD\n");
					#endif
					//sleep(10);
					close( p[0] );
	  				/*we want the output on stdout from first process to go to the write end of the pipe*/ 
	  				/*duplicate fd 4 into fd 1*/
	  				dup2(p[1], 1);	
	  				execv(searchingpath_and_file_1, argv1);
	  			}
	  			else /*PARENT*/
	  			{
	  				waitpid( pid1, &status, 0 );  /* block indefinitely */
	  				pid2 = fork();
	  				if ( pid2 == 0 )
	  				{
		  				//sleep(10);
		  				#ifdef DEBUG_MODE
						printf( "THIS IS SECOND THE CHILD\n");
						#endif
						close( p[1] );
						/* we want the input in stdin to be the read end of the pipe */
						/*duplicate fd 3 into fd 0 */
						dup2(p[0], 0);	
						execv(searchingpath_and_file_2, argv2);  				
	  				}
	  				else /*PARENT*/
	  				{
	  					  				
	  					close( p[0] );
						close( p[1] );
	  		
	  					//pid_t child_pid1;	
	  					//pid_t child_pid2;			
	  					int status2;
	  				
	  					if (background == 0)
	  					{
	  						//child_pid1 = waitpid( pid1, &status, 0 );  /* block indefinitely */
	  						//child_pid2 = waitpid( pid2, &status2, 0 );  /* block indefinitely */
	  						//waitpid( pid1, &status, 0 );  /* block indefinitely */
	  						waitpid( pid2, &status2, 0 );  /* block indefinitely */
	  					}
	  					else if (background == 1)
	  					{
	  						if (background_pids_size >= 128)
	  						{
	  							fprintf(stderr, "EXCEEDED MAXIMUM AMOUNT OF BACKGROUND PROCESSES\n");
	  						}
	  						printf("[running background process \"%s\"]\n", argv1[0]);
	  						background_pids[background_pids_size] = pid1;
	  						background_pids_size += 1;
	  						
	  						if (background_pids_size >= 128)
	  						{
	  							fprintf(stderr, "EXCEEDED MAXIMUM AMOUNT OF BACKGROUND PROCESSES\n");
	  						}
	  						printf("[running background process \"%s\"]\n", argv2[0]);
	  						background_pids[background_pids_size] = pid2;
	  						background_pids_size += 1;
	  					}
	  				}
	  			}
			}
			free(searchingpath_and_file_1);
			free(searchingpath_and_file_2);
		}	
		
		/* FREE ALL ALLOCATED MEMORY *////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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
		
		if (argv1 != NULL) {
			for (int i = 0; i < (argv_1_num+ 1); i++)
			{
				free(argv1[i]);
			}
			free(argv1);
		}
		
		
		if (argv2 != NULL) {
			for (int i = 0; i < (argv_2_num+ 1); i++)
			{
				free(argv2[i]);
			}
			free(argv2);
		}
		
		if (argv_1_2 != NULL) {
			for (int i = 0; i < (argv_1_2_num+ 1); i++)
			{
				free(argv_1_2[i]);
			}
			free(argv_1_2);
		}
	}
}

