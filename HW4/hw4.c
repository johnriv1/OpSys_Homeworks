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
#define MAX_CLIENTS 100			/* <===== */


int main()
{
	/* ====== */
	fd_set readfds;
	int client_sockets[ MAX_CLIENTS ]; /* client socket fd list */
	int client_socket_index = 0;	/* next free spot */
	/* ====== */
	
	/*create TCP socket and bind (SAME AS USUAL CODE */////////////////////////////////////////////
	
	/* Create the listener socket as TCP socket */
	int tcp_sock = socket( PF_INET, SOCK_STREAM, 0 );
	if ( tcp_sock < 0 ) 
	{
		perror( "tcp socket() failed" );
		exit( EXIT_FAILURE );
	}
	/* Create the listener socket as UDP socket */
	int udp_sock = socket( PF_INET, SOCK_DGRAM, 0 );
	if ( udp_sock < 0 ) 
	{
		perror( "udp socket() failed" );
		exit( EXIT_FAILURE );
	}
	/* socket structures from /usr/include/sys/socket.h */
	struct sockaddr_in server;///BOTH
	struct sockaddr_in client;

	server.sin_family = PF_INET; ///BOTH
	server.sin_addr.s_addr = htonl(INADDR_ANY); ///BOTH

	unsigned short port = 8128; //BOTH

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
	/* 5 is number of waiting clients */
	if ( listen( tcp_sock, 5 ) == -1 )
	{
		perror( "listen() failed" );
		return EXIT_FAILURE;
	}
	printf( "SERVER: TCP listener socket bound to port %d\n", port );

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
	char buffer[ BUFFER_SIZE ];
	int i, n;

	while ( 1 )
	{

		FD_ZERO( &readfds );
	/*following line will add fd as one of the descripters that we're interested in listening on*/
		FD_SET( tcp_sock, &readfds );	 /* listener socket, fd 3 */
		printf( "Set FD_SET to include listener fd %d\n", tcp_sock );

		/* initially, this for loop does nothing; but once we have */
		/*	client connections, we will add each client connection's fd */
		/*	 to the readfds (the FD set) */
		for ( i = 0 ; i < client_socket_index ; i++ )
		{
			FD_SET( client_sockets[ i ], &readfds );
			printf( "Set FD_SET to include client socket fd %d\n",
							client_sockets[ i ] );
		}

		/* This is a BLOCKING call, but will block on all readfds */
		int ready = select( FD_SETSIZE, &readfds, NULL, NULL, NULL );

		/* ready is the number of ready file descriptors */
		printf( "select() identified %d descriptor(s) with activity\n", ready );


		/* is there activity on the listener descriptor? */
		if ( FD_ISSET( tcp_sock, &readfds ) )
		{
			int newsock = accept( tcp_sock, (struct sockaddr *)&client, (socklen_t *)&fromlen );
						 /* this accept() call we know will not block */
			printf( "Accepted client connection\n" );
			client_sockets[ client_socket_index++ ] = newsock;
		}


		/* is there activity on any of the established tcp connections? */
		for ( i = 0 ; i < client_socket_index ; i++ )
		{
			int fd = client_sockets[ i ];

			if ( FD_ISSET( fd, &readfds ) )
			{
				/* can also use read() and write() */
				n = recv( fd, buffer, BUFFER_SIZE - 1, 0 );
						/* we know this recv() call will not block */

				if ( n < 0 )
				{
					perror( "recv()" );
				}
				else if ( n == 0 )
				{
					int k;
					printf( "Client on fd %d closed connection\n", fd );
					close( fd );

					/* remove fd from client_sockets[] array: */
					for ( k = 0 ; k < client_socket_index ; k++ )
					{
						if ( fd == client_sockets[ k ] )
						{
							/* found it -- copy remaining elements over fd */
							int m;
							for ( m = k ; m < client_socket_index - 1 ; m++ )
							{
								client_sockets[ m ] = client_sockets[ m + 1 ];
							}
							client_socket_index--;
							break;	/* all done */
						}
					}
				}
				else
				{
					buffer[n] = '\0';
					printf( "Received message from %s: %s\n",
									inet_ntoa( (struct in_addr)client.sin_addr ),
									buffer );

					/* send ack message back to client */
					n = send( fd, "ACK\n", 4, 0 );
					if ( n != 4 )
					{
						perror( "send() failed" );
					}
				}
			}
		}
	}

	return EXIT_SUCCESS; /* we never get here */
}

