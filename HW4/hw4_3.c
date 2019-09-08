/*Network Programming and Multi-Threaded Programming using C*/
/*John Rivera*/

/* server-select.c */

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

#include <pthread.h>

typedef struct
{
	int fd;
	char* Userid;
}  TCP_Conn;

int compare_connection(const void *p1, const void *p2)
{
   const TCP_Conn *elem1 = p1;    
   const TCP_Conn *elem2 = p2;
   
   if (strcmp(elem1->Userid, "NA") == 0 && strcmp(elem2->Userid,"NA") != 0)
   {
   	return 1;
   }
   else if (strcmp(elem1->Userid, "NA") != 0 && strcmp(elem2->Userid,"NA") == 0)
   {
   	return -1;
   }
   else if (strcmp(elem1->Userid, "NA") == 0 && strcmp(elem2->Userid,"NA") == 0)
   {
   	return 0;
   }
   else
   {
   	return strcmp(elem1->Userid, elem2->Userid);
   /*
   	if (elem1->Userid < elem2->Userid)
   	{
   		return 1;
   	}
   	else if (elem1->Userid > elem2->Userid)
   	{
   		return -1;
   	}
   	else
   	{
   		return 0;
   	}
   */
   }
   #if 0
   if (elem1->Userid < elem2->arrival_time)
      return -1;
   else if (elem1->arrival_time > elem2->arrival_time)
      return 1;
   /*if times are the same, sort by alphabetical order*/
   else if (elem1->arrival_time == elem2->arrival_time)
   {
  		if (elem1->id < elem2->id)
  			return -1;
  		else
  			return 1;
   }
   else
      return 0;
   #endif
}

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

TCP_Conn client_sockets[ MAX_CLIENTS ]; /* client socket fd list */
int client_socket_index = 0;	/* next free spot */

void print_clients()
{
	printf("TCP connections are: ");
	for (int i = 0; i < client_socket_index; i++)
	{
		printf("| fd %d , user %s |", client_sockets[i].fd, client_sockets[i].Userid);
	}
	printf("\n");
}

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
	char* arg = calloc(1024, sizeof( char ));
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
				memset(arg, 0, 1024);
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


void * TCP_connection(void * arg)
{
	int fd = *(int*) arg;
	char buffer[ BUFFER_SIZE ];
	int n;
	//int s;
	char* this_userid = calloc(17, sizeof(char));
	strcpy(this_userid, "NA");
	do
	{
		n = recv( fd, buffer, BUFFER_SIZE - 1, 0 );
		
		if ( n < 0 )
		{
			perror( "recv()" );
		}
		else if ( n == 0 )
		{
			int k;
			//printf( "Client on fd %d closed connection\n", fd );
			printf("CHILD %ld: Client disconnected\n", pthread_self());
			close( fd );

			/* remove fd from client_sockets[] array: */
			for ( k = 0 ; k < client_socket_index ; k++ )
			{
				if ( fd == client_sockets[ k ].fd )
				{
					/* found it -- copy remaining elements over fd */
					int m;
					for ( m = k ; m < client_socket_index - 1 ; m++ )
					{
						pthread_mutex_lock( &mutex );
						client_sockets[ m ].fd = client_sockets[ m + 1 ].fd;
						strcpy(client_sockets[ m ].Userid, client_sockets[ m + 1].Userid);
						pthread_mutex_unlock( &mutex );
					}
					pthread_mutex_lock( &mutex );
					client_socket_index--;
					pthread_mutex_unlock( &mutex );
				}
			}
		}
		else
		{
			buffer[n] = '\0';
			//printf( "Received message: %s\n", buffer );
			char** command = NULL;
			int command_num = string_parse(buffer, &command, ' ', 1);
			
			if (strcmp(command[0], "LOGIN") == 0)
			{
				/* Format is: LOGIN <userid>\n */
				char** args = NULL;
				int arguments_num = string_parse(buffer, &args, ' ', 3);
				int repeat_userid = 0;
				if (arguments_num == 2)
				{
					/* gets rid of the newline character */
					args[1][strlen(args[1])-1] = '\0';
					printf("CHILD %ld: Rcvd LOGIN request for userid %s\n", pthread_self(), args[1]);
					/* username must have length in range of [4,16] */
					if (strlen(args[1]) >= 4 && strlen(args[1]) <= 16)
					{
						for (int i=0; i < client_socket_index; i++)
						{
							if (strcmp(client_sockets[i].Userid, args[1]) == 0)
							{
								repeat_userid = 1;
								break;
							}
						}
						if (repeat_userid == 0)
						{
						/* find correct fd in client_sockets array and set userid */
							for (int i=0; i < client_socket_index; i++)
							{
								if (client_sockets[i].fd == fd)
								{
									pthread_mutex_lock( &mutex );
									if (strcmp(client_sockets[i].Userid, "NA") == 0)
									{
										client_sockets[i].Userid = args[1];
										//s = send( fd, "OK!\n", 4, 0 );
										send( fd, "OK!\n", 4, 0 );
										strcpy(this_userid, args[1]);
										//printf("USERid is now %s", this_userid);
									}
									else
									{
										/* if same client already connected */
										send( fd, "ERROR Already connected\n", 24, 0 );
										printf("CHILD %ld: Sent ERROR (Already connected)\n", pthread_self());
									}
									pthread_mutex_unlock( &mutex );
									break;
								}
							}
						}
						else
						{
							/* if same user already connected on a client */
							pthread_mutex_lock( &mutex );
							send( fd, "ERROR Already connected\n", 24, 0 );
							pthread_mutex_unlock( &mutex );
							printf("CHILD %ld: Sent ERROR (Already connected)\n", pthread_self());
						}
					}
					else
					{
						pthread_mutex_lock( &mutex );
						send( fd, "ERROR Invalid userid\n", 21, 0 );
						pthread_mutex_unlock( &mutex );
						printf("CHILD %ld: Sent ERROR (Invalid userid)\n", pthread_self());
					}
				}
				else
				{
					pthread_mutex_lock( &mutex );
					send(fd, "ERROR Invalid userid\n", 21, 0 );
					pthread_mutex_unlock( &mutex );
					printf("CHILD %ld: Sent ERROR (Invalid userid)\n", pthread_self());
				}
				
				//print_clients();
			}
			else if (strcmp(command[0], "WHO\n") == 0)
			{
				printf("CHILD %ld: Rcvd WHO request\n", pthread_self());
				/* max 32 users with user ids of 16 length at max. 
					'\n' after every user id. -> 32*16 + 32 = 544 -> we can use buffer[1024] */ 
				sprintf(buffer, "OK!\n"); 
				pthread_mutex_lock( &mutex );
				qsort(client_sockets, client_socket_index, sizeof(TCP_Conn), compare_connection);
				for (int i = 0; i < client_socket_index; i++)
				{
					if (strcmp(client_sockets[i].Userid, "NA") != 0)
					{
						sprintf(buffer+strlen(buffer), "%s\n", client_sockets[i].Userid) ;
					}
					else
					{
						break;
					}
				}
				send(fd, buffer, strlen(buffer), 0);
				pthread_mutex_unlock( &mutex );
			}
			else if (strcmp(command[0], "LOGOUT\n") == 0)
			{
				printf("CHILD %ld: Rcvd LOGOUT request\n", pthread_self());
				/* must be logged in to do this command */
				if (strcmp(this_userid, "NA") != 0)
				{
					/* remove fd from userid: */
					for (int k = 0 ; k < client_socket_index ; k++ )
					{
						if ( fd == client_sockets[ k ].fd )
						{
							pthread_mutex_lock( &mutex );
							send( fd, "OK!\n", 4, 0 );
							/* found it -- copy remaining elements over fd */
							strcpy(client_sockets[k].Userid, "NA");
							strcpy(this_userid, "NA");
							pthread_mutex_unlock( &mutex );
							//printf("CHILD %ld: Client disconnected\n", pthread_self());
							break;
						}
					}
				}
				else
				{
					pthread_mutex_lock( &mutex );
					send(fd, "ERROR must LOGIN to use this command\n", 37, 0 );
					pthread_mutex_unlock( &mutex );
					printf("CHILD %ld: Sent ERROR (Not logged in)\n" , pthread_self());
				}
			}
			else if (strcmp(command[0], "SEND") == 0)
			{
				if (strcmp(this_userid, "NA") != 0)
				{
					/* Format is: SEND <recipient-userid> <msglen>\n<message> */
					//printf("THIS MANY ARGUMENTS %d", arguments_num);
					char** send_args = NULL;
					int valid_format = 0;
					int send_arguments_num = string_parse(buffer, &send_args, ' ', 2);
					if (send_arguments_num == 3)	
					{
						char** message_args = NULL;
						int message_args_num = string_parse(send_args[2], &message_args, '\n', 1);
						//int message_args_num = string_parse(send_args[2], &message_args, 'x', 1);
						if (message_args_num == 2)
						{
							valid_format = 1;
						}
						else if (message_args_num == 1)
						{
							if (send_args[2][strlen(send_args[2])-1] == '\n')
							{
								valid_format = 1;
							}
						}
						if (valid_format == 1)
						{
							printf("CHILD %ld: Rcvd SEND request to userid %s\n", pthread_self(), send_args[1]);
							/* does recipient client exist?, if so get its fd */
							int found = 0;
							int fd_found = 0;
							for (int i = 0; i < client_socket_index; i++)
							{
								if (strcmp(client_sockets[i].Userid, send_args[1]) == 0)
								{
									//printf("FOUND HIM\n");
									found = 1;
									fd_found = client_sockets[i].fd;
									break;
								}
							}
							if (found == 1)
							{
								//printf("WE WILL SEND %s\n", message_args[1]);
								if (atoi(message_args[0]) >= 1 && atoi(message_args[0]) <= 990)
								{
									pthread_mutex_lock( &mutex );
									send(fd, "OK!\n", 4, 0);
									pthread_mutex_unlock( &mutex );
									int bytes_left = atoi(message_args[0]);
									char* send_buffer = calloc( (strlen(message_args[0])+9+strlen(this_userid) + atoi(message_args[0])), sizeof(char));
									sprintf(send_buffer, "FROM %s %s ", this_userid, message_args[0]);
									if (message_args_num == 2)
									{
										sprintf(send_buffer+strlen(send_buffer), "%s", message_args[1]);
										bytes_left -= strlen(message_args[1]);
									} 
									while ( bytes_left > 0)
									{
										n = recv(fd, buffer, BUFFER_SIZE, 0);
										bytes_left -= n;
										sprintf(send_buffer+strlen(send_buffer), "%s", buffer);
									}
									//sprintf(send_buffer + strlen(send_buffer), "\n");
									//s = send(fd_found, message_args[1], strlen(message_args[1]), 0);
									pthread_mutex_lock( &mutex );
									send(fd_found, send_buffer, strlen(send_buffer), 0);
									pthread_mutex_unlock( &mutex );
									free(send_buffer);
								}
								else
								{
									pthread_mutex_lock( &mutex );
									send(fd, "ERROR Invalid msglen\n", 21,0);
									pthread_mutex_unlock( &mutex );
									printf("CHILD %ld: Sent ERROR (Invalid msglen)\n" , pthread_self());
								}
							}
							else
							{
								pthread_mutex_lock( &mutex );
								send(fd, "ERROR Unknown userid\n", 21, 0 );
								pthread_mutex_unlock( &mutex );
								printf("CHILD %ld: Sent ERROR (Unknown userid)\n" , pthread_self());
							}
						}
						else
						{
							pthread_mutex_lock( &mutex );
							send(fd, "ERROR Invalid SEND format\n", 26, 0 );
							pthread_mutex_unlock( &mutex );
							printf("CHILD %ld: Sent ERROR (Invalid SEND format)\n" , pthread_self());
						}		
					}
					else
					{
						pthread_mutex_lock( &mutex );
						send(fd, "ERROR Invalid SEND format\n", 26, 0 );
						pthread_mutex_unlock( &mutex );
						printf("CHILD %ld: Sent ERROR (Invalid SEND format)\n" , pthread_self());
					}
				}
				else
				{
					pthread_mutex_lock( &mutex );
					send(fd, "ERROR must LOGIN to use this command\n", 37, 0 );
					pthread_mutex_unlock( &mutex );
					printf("CHILD %ld: Sent ERROR (Not logged in)\n" , pthread_self());
				}
			}
			else if (strcmp(command[0], "BROADCAST") == 0)
			{
				printf("CHILD %ld: Rcvd BROADCAST request\n", pthread_self());
				if (strcmp(this_userid, "NA") != 0)
				{
					if (command_num == 2)
					{
						char** message_args = NULL;
						int message_args_num = string_parse(command[1], &message_args, '\n', 1);
						int valid_format = 0;
						if (message_args_num == 2)
						{
							valid_format = 1;
						}
						if (message_args_num == 1)
						{
							if (command[1][strlen(command[1])-1] == '\n')
							{
								valid_format = 1;
							}
						}
						if (valid_format == 1)
						{
							if (atoi(message_args[0]) >= 1 && atoi(message_args[0]) <= 990)
							{
								pthread_mutex_lock( &mutex );
								send(fd, "OK!\n", 4, 0);
								pthread_mutex_unlock( &mutex );
								char* send_buffer = calloc( (strlen(message_args[0])+ 9 +strlen(this_userid) + atoi(message_args[0])), sizeof(char));
								int bytes_left = atoi(message_args[0]);
								sprintf(send_buffer, "FROM %s %s ", this_userid, message_args[0]);
								if (message_args_num == 2)
								{
									sprintf(send_buffer+strlen(send_buffer), "%s", message_args[1]);
									bytes_left -= strlen(message_args[1]);
								} 
								while ( bytes_left > 0)
								{
									n = recv(fd, buffer, BUFFER_SIZE, 0);
									bytes_left -= n;
									sprintf(send_buffer+strlen(send_buffer), "%s", buffer);
								}
								//sprintf(send_buffer + strlen(send_buffer), "\n");
								
								for (int k = 0 ; k < client_socket_index ; k++ )
								{
									if ( strcmp(client_sockets[k].Userid, "NA") != 0 )
									{
										//sprintf(buffer, "FROM %s %s %s\n", this_userid, message_args[0], message_args[1]);
										pthread_mutex_lock( &mutex );
										send(client_sockets[k].fd, send_buffer, strlen(send_buffer), 0);
										pthread_mutex_unlock( &mutex );
										//s = send(client_sockets[k].fd, message_args[1], strlen(message_args[1]), 0 );
									}
								}
								free(send_buffer);
							}
							else
							{
								pthread_mutex_lock( &mutex );
								send(fd, "ERROR Invalid msglen\n", 21,0);
								pthread_mutex_unlock( &mutex );
								printf("CHILD %ld: Sent ERROR (Invalid msglen)\n" , pthread_self());
							}
						}
						else
						{
							pthread_mutex_lock( &mutex );
							send(fd, "ERROR Invalid BROADCAST format\n", 31, 0 );
							pthread_mutex_unlock( &mutex );
							printf("CHILD %ld: Sent ERROR (Invalid BROADCAST format)\n" , pthread_self());
						}
					}
					else
					{
						pthread_mutex_lock( &mutex );
						send(fd, "ERROR Invalid BROADCAST format\n", 31, 0 );
						pthread_mutex_unlock( &mutex );
						printf("CHILD %ld: Sent ERROR (Invalid BROADCAST format)\n" , pthread_self());	
					}
				}
				else
				{
					pthread_mutex_lock( &mutex );
					send(fd, "ERROR must LOGIN to use this command\n", 37, 0 );
					pthread_mutex_unlock( &mutex );
					printf("CHILD %ld: Sent ERROR (Invalid BROADCAST format)\n" , pthread_self());
				}
			}
			else if (strcmp(command[0], "SHARE") == 0)
			{
				printf("CHILD %ld: Rcvd SHARE request\n", pthread_self());
				if (strcmp(this_userid, "NA") != 0)
				{
					char** share_args = NULL;
					int share_arg_num = string_parse(buffer, &share_args, ' ', 4);
					int found_recip = 0;
					int recip_fd = 0;
					if (share_arg_num == 3)
					{
						char* filelen = calloc(strlen(share_args[2]), sizeof(char));
						strcpy(filelen, share_args[2]);
						filelen[sizeof(filelen) - 1] = '\0';
						if (atoi(filelen) > 0)
						{
							for (int i = 0; i < client_socket_index; i++)
							{	
								if (strcmp(client_sockets[i].Userid, share_args[1]) == 0)
								{
									found_recip = 1;
									recip_fd = client_sockets[i].fd;
									break;
								}	
							}
							if (found_recip == 1)
							{
								int bytes_left = atoi(share_args[2]);
								/* acknowledge share command */
								pthread_mutex_lock( &mutex );
								send(fd, "OK!\n", 4, 0);
								pthread_mutex_unlock( &mutex );
								/* tell recipient that it will receive shared data */
								sprintf(buffer, "SHARE %s %s", this_userid, share_args[2]);
								pthread_mutex_lock( &mutex );
								//send(atoi(share_args[1]), buffer, strlen(buffer), 0);
								send(recip_fd, buffer, strlen(buffer), 0);
								pthread_mutex_unlock( &mutex );
								/*receive bytes and immedietely direct package to recipient*/
								while (bytes_left != 0)
								{
									n = recv( fd, buffer, BUFFER_SIZE, 0 );
									//printf("Received %d bytes\n", n);
									pthread_mutex_lock( &mutex );
									send(fd, "OK!\n", 4, 0);
									//send(atoi(share_args[1]), buffer, n, 0);
									send(recip_fd, buffer, n, 0);
									pthread_mutex_unlock( &mutex );
									bytes_left -= n;
								}
							}
							else
							{
								pthread_mutex_lock( &mutex );
								send(fd, "ERROR SHARE not supported over UDP\n", 35, 0);
								pthread_mutex_unlock( &mutex );
								printf("CHILD %ld: Sent ERROR (SHARE not supported over UDP)\n" , pthread_self());
							}
						}
						else
						{
							pthread_mutex_lock( &mutex );
							send(fd, "ERROR Invalid filelen\n", 22, 0 );
							pthread_mutex_unlock( &mutex );
							printf("CHILD %ld: Sent ERROR (Invalid filelen)\n" , pthread_self());
						}
					}
					else
					{
						pthread_mutex_lock( &mutex );
						send(fd, "ERROR Invalid SHARE format\n", 27, 0 );
						pthread_mutex_unlock( &mutex );
						printf("CHILD %ld: Sent ERROR (Invalid SHARE format)\n" , pthread_self());
					}
				}
				else
				{
					pthread_mutex_lock( &mutex );
					send(fd, "ERROR Unknown userid\n", 21, 0 );
					pthread_mutex_unlock( &mutex );
					printf("CHILD %ld: Sent ERROR (Unknown userid)\n" , pthread_self());
				}
			}
			else
			{
				send(fd, "ERROR Command not recognized\n", 29, 0);
				printf("CHILD %ld: Sent ERROR (Command not recognized)\n" , pthread_self());
			}
		}
	}
	while (n > 0);
	free(this_userid);
	return NULL;
}

int main( int argc, char** argv )
{

	setvbuf( stdout, NULL, _IONBF, 0 );
	
	unsigned short port;
	if (argc ==	2)
	{
		port = (unsigned short) atoi(argv[1]);
	}
	else
	{
		fprintf(stderr, "MAIN: ERROR must include port number\n");
		return EXIT_FAILURE;
	}
	
	printf("MAIN: Started server\n");
	
	/* ====== */
	fd_set readfds;
	/* ====== */
	
	/*create TCP and UDP socket and bind (SAME AS USUAL CODE */////////////////////////////////////
	
	/* Create the listener socket as UDP socket */
	int udp_sock = socket( PF_INET, SOCK_DGRAM, 0 );
	if ( udp_sock < 0 ) 
	{
		perror( "udp socket() failed" );
		exit( EXIT_FAILURE );
	}
	/* Create the listener socket as TCP socket */
	int tcp_sock = socket( PF_INET, SOCK_STREAM, 0 );
	if ( tcp_sock < 0 ) 
	{
		perror( "tcp socket() failed" );
		exit( EXIT_FAILURE );
	}
	/* socket structures from /usr/include/sys/socket.h */
	struct sockaddr_in server;///BOTH
	struct sockaddr_in client;

	server.sin_family = PF_INET; ///BOTH
	server.sin_addr.s_addr = htonl(INADDR_ANY); ///BOTH

	//unsigned short port = 8128; //BOTH

	/* htons() is host-to-network-short for marshalling */
	/* Internet is "big endian"; Intel is "little endian" */
	server.sin_port = htons( port ); ///BOTH
	int len = sizeof( server );

	if ( bind( tcp_sock, (struct sockaddr *)&server, len ) < 0 )
	{
		perror( "tcp bind() failed" );
		exit( EXIT_FAILURE );
	}
	if ( bind( udp_sock, (struct sockaddr *) &server, len ) < 0 )
	{
		perror( "udp bind() failed" );
		return EXIT_FAILURE;
	}
	/* 32 is number of waiting clients */
	if ( listen( tcp_sock, 32 ) == -1 )
	{
		perror( "listen() failed" );
		return EXIT_FAILURE;
	}
	//printf( "SERVER: TCP listener socket bound to port %d\n", port );

	int fromlen = sizeof( client );

	///////////////////////////////////////////////////////////////////////////////////////////////////
	/*
	int udp_sock = socket( PF_INET, SOCK_DGRAM, 0 );
	if ( udp_sock < 0 ) 
	{
		perror( "udp socket() failed" );
		exit( EXIT_FAILURE );
	}
	if ( bind( udp_sock, (struct sockaddr *) &server, len ) < 0 )
	{
		perror( "udp bind() failed" );
		return EXIT_FAILURE;
	}
	*/
	/////////////////////////////////////////////////////////////////////////////////////////////////
	printf("MAIN: Listening for TCP connections on port: %hu\n", port);
	printf("MAIN: Listening for UDP datagrams on port: %hu\n", port);
	
	char buffer[ BUFFER_SIZE ];
	int n, rc;

	while ( 1 )
	{

		FD_ZERO( &readfds );
	/*following line will add fd as one of the descripters that we're interested in listening on*/
		FD_SET( udp_sock, &readfds );	 /* listener socket, fd 3 */
		FD_SET( tcp_sock, &readfds );	 /* listener socket, fd 4 */
		//printf( "Set FD_SET to include listener fd %d\n", tcp_sock );

		/* initially, this for loop does nothing; but once we have */
		/*	client connections, we will add each client connection's fd */
		/*	 to the readfds (the FD set) */
		/*
		for ( i = 0 ; i < client_socket_index ; i++ )
		{
			FD_SET( client_sockets[ i ], &readfds );
			printf( "Set FD_SET to include client socket fd %d\n",
							client_sockets[ i ] );
		}
		*/
		/* This is a BLOCKING call, but will block on all readfds */
		int ready = select( FD_SETSIZE, &readfds, NULL, NULL, NULL );
		if (ready == -1)
		{
			perror( "select() failed" );
			return EXIT_FAILURE;
		}
		/* ready is the number of ready file descriptors */
		//printf( "select() identified %d descriptor(s) with activity\n", ready );

		if ( FD_ISSET( udp_sock, &readfds ) )
		{
			n = recvfrom( udp_sock, buffer, BUFFER_SIZE, 0, 
				(struct sockaddr *) &client, (socklen_t *) &len );
			
			printf("MAIN: Rcvd incoming UDP datagram from: %s\n", inet_ntoa(client.sin_addr));

			//printf( "RCVD %d bytes\n", n );
			buffer[n] = '\0';   /* assume that its printable char[] data */
      	//printf( "RCVD: [%s]\n", buffer );
      	
      	char** command = NULL;
			int command_num = string_parse(buffer, &command, ' ', 1);
			
			if (strcmp(command[0], "LOGIN") == 0)
			{
				pthread_mutex_lock( &mutex );
				sendto(udp_sock, "ERROR LOGIN not supported over UDP\n", 35, 0, (struct sockaddr *)&client, fromlen);
				pthread_mutex_unlock( &mutex );
				printf("MAIN: Sent ERROR (LOGIN not supported over UDP)\n");
			}
			else if (strcmp(command[0], "WHO\n") == 0)
			{
				printf("MAIN: Rcvd WHO request\n");
				/* max 32 users with user ids of 16 length at max. 
					'\n' after every user id. -> 32*16 + 32 = 544 -> we can use buffer[1024] */ 
				sprintf(buffer, "OK!\n"); 
				pthread_mutex_lock( &mutex );
				qsort(client_sockets, client_socket_index, sizeof(TCP_Conn), compare_connection);
				for (int i = 0; i < client_socket_index; i++)
				{
					if (strcmp(client_sockets[i].Userid, "NA") != 0)
					{
						sprintf(buffer+strlen(buffer), "%s\n", client_sockets[i].Userid) ;
					}
					else
					{
						break;
					}
				}
				sendto(udp_sock, buffer, strlen(buffer), 0, (struct sockaddr *)&client, fromlen);
				pthread_mutex_unlock( &mutex );
			}
			else if (strcmp(command[0], "LOGOUT\n") == 0)
			{
				pthread_mutex_lock( &mutex );
				sendto(udp_sock, "ERROR LOGOUT not supported over UDP\n", 36, 0 , (struct sockaddr *)&client, fromlen);
				pthread_mutex_unlock( &mutex );
				printf("MAIN: Sent ERROR (LOGOUT not supported over UDP)\n");
			}
			else if (strcmp(command[0], "SEND") == 0)
			{
				pthread_mutex_lock( &mutex );
				sendto(udp_sock, "ERROR SEND not supported over UDP\n", 34, 0, (struct sockaddr *)&client, fromlen);
				pthread_mutex_unlock( &mutex );
				printf("MAIN: Sent ERROR (SEND not supported over UDP)\n");
			}
			else if (strcmp(command[0], "BROADCAST") == 0)
			{
				printf("MAIN: Rcvd BROADCAST request\n");
				if (command_num == 2)
				{
					char** message_args = NULL;
					int message_args_num = string_parse(command[1], &message_args, '\n', 1);
					if (message_args_num == 2)
					{
						if (atoi(message_args[0]) >= 1 && atoi(message_args[0]) <= 990)
						{
							sendto(udp_sock, "OK!\n", 4, 0, (struct sockaddr *)&client, fromlen);
							for (int k = 0 ; k < client_socket_index ; k++ )
							{
								if ( strcmp(client_sockets[k].Userid, "NA") != 0 )
								{
									sprintf(buffer, "FROM UDP-client %s %s", message_args[0], message_args[1]);
									pthread_mutex_lock( &mutex );
									send(client_sockets[k].fd, buffer, strlen(buffer), 0);
									pthread_mutex_unlock( &mutex );
									//send(client_sockets[k].fd, message_args[1], strlen(message_args[1]), 0 );
									//send(client_sockets[k].fd, message_args[1], strlen(message_args[1]), 0 );
								}
							}
						}
						else
						{
							pthread_mutex_lock( &mutex );
							sendto(udp_sock, "ERROR Invalid msglen\n", 21,0, (struct sockaddr *)&client, fromlen);
							pthread_mutex_unlock( &mutex );
							printf("MAIN: Sent ERROR (Invalid msglen)\n");
						}
					}
					else
					{
						pthread_mutex_lock( &mutex );
						sendto(udp_sock, "ERROR Invalid BROADCAST format\n", 31, 0, (struct sockaddr *)&client, fromlen );
						pthread_mutex_unlock( &mutex );
						printf("MAIN: Sent ERROR (Invalid BROADCAST format)\n");
					}
				}
				else
				{
					pthread_mutex_lock( &mutex );
					sendto(udp_sock, "ERROR Invalid BROADCAST format\n", 31, 0, (struct sockaddr *)&client, fromlen);
					pthread_mutex_unlock( &mutex );
					printf("MAIN: Sent ERROR (Invalid BROADCAST format)\n");
				}
			}
			else if (strcmp(command[0], "SHARE") == 0)
			{
				pthread_mutex_lock( &mutex );
				sendto(udp_sock, "ERROR SHARE not supported over UDP\n", 35, 0, (struct sockaddr *)&client, fromlen );
				pthread_mutex_unlock( &mutex );
				printf("MAIN: Sent ERROR (SHARE not supported over UDP)\n");
			}
      	continue;
		}
		/* is there activity on the listener TCP descriptor? */
		if ( FD_ISSET( tcp_sock, &readfds ) )
		{
			pthread_t tid;
			printf("MAIN: Rcvd incoming TCP connection from: %s\n", inet_ntoa(client.sin_addr));
			int newsock = accept( tcp_sock, (struct sockaddr *)&client, (socklen_t *)&fromlen );
						 /* this accept() call we know will not block */
			//printf( "Accepted client connection\n" );
			client_sockets[ client_socket_index ].fd = newsock;
			client_sockets[ client_socket_index ].Userid = calloc(17, sizeof(char));
			/*"NA" signifies that no user is logged in to TCP connection*/
			client_sockets[ client_socket_index].Userid = "NA";
			client_socket_index++;
			rc = pthread_create(&tid, NULL, TCP_connection, &newsock);
			if ( rc != 0 )
			{
				fprintf( stderr, "MAIN: Could not create thread (%d)\n", rc );
				exit(EXIT_FAILURE);
			}
		}
	}

	return EXIT_SUCCESS; /* we never get here */
}

