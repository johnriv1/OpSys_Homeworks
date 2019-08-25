#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <strings.h>
#include <unistd.h>

#define BUFFER_SIZE 1024



void send_tcp(int sd, char * msg) {

	printf("sending [%s]\n",msg);
	int n = write( sd, msg, strlen( msg ) );    /* or send()/recv() */

	if ( n < strlen( msg ) )
	{
		perror( "write() failed" );
		return; //EXIT_FAILURE;
	}


}

void recv_tcp(int sd, char * recv){
	int recv_len = strlen(recv);
	char buffer[ recv_len + 1];
	int n = read( sd, buffer, recv_len );    /* BLOCKING */

	if ( n == -1 )
	{
		perror( "read() failed" );
		return; //EXIT_FAILURE;
	}
		else if ( n == 0 )
	{
		printf( "Rcvd no data; also, server socket was closed\n" );
	}
	else  /* n > 0 */
	{
		buffer[recv_len] = '\0';    /* assume we rcvd text-based data */
		printf( "TCP rcvd [%s]\n", buffer );
	}
}

int connect_tcp(unsigned short port) {
	/* create TCP client socket (endpoint) */
	int sd = socket( PF_INET, SOCK_STREAM, 0 );

	if ( sd == -1 )
	{
		perror( "socket() failed" );
		exit( EXIT_FAILURE );
	}

#if 1
	struct hostent * hp = gethostbyname( "localhost" );  /* 127.0.0.1 */
	//struct hostent * hp = gethostbyname( "127.0.0.1" );
	//struct hostent * hp = gethostbyname( "128.113.126.29" );
#endif

	//struct hostent * hp = gethostbyname( "linux04.cs.rpi.edu" );

	if ( hp == NULL )
	{
		fprintf( stderr, "ERROR: gethostbyname() failed\n" );
		return EXIT_FAILURE;
	}

	struct sockaddr_in server;
	server.sin_family = PF_INET;
	memcpy( (void *)&server.sin_addr, (void *)hp->h_addr, hp->h_length );
	//unsigned short port = atoi(argv[1]);
	server.sin_port = htons( port );

	printf( "Server address is %s\n", inet_ntoa( server.sin_addr ) );


	//printf( "connecting to server.....\n" );
	if ( connect( sd, (struct sockaddr *)&server, sizeof( server ) ) == -1 )
	{
		perror( "connect() failed" );
		return EXIT_FAILURE;
	}
	printf("Connected to the server via TCP\n");
	return sd;
}



struct udp_connection{
	int sd;
	struct sockaddr_in servaddr;
};

void send_udp(struct udp_connection * udp, char * msg){
	printf("sending [%s]\n",msg);
	sendto(udp->sd,msg,strlen(msg),0,(struct sockaddr *) &udp->servaddr, sizeof(udp->servaddr));
}

void recv_udp(struct udp_connection * udp, char * recv){
	int recv_len = strlen(recv);
	char  buffer[recv_len + 1];
	int len = sizeof(udp->servaddr);
	int n = recvfrom(udp->sd, (char *)buffer, recv_len, 0, (struct sockaddr *) &udp->servaddr, (socklen_t *) &len); 
    buffer[recv_len] = '\0';
    printf("UDP rcvd [%s]\n",buffer);
}

struct udp_connection connect_udp(unsigned short port){ //https://www.geeksforgeeks.org/udp-server-client-implementation-c/
	int sd; 
    //char buffer[MAXLINE]; 
    //char *hello = "Hello from client";
    struct sockaddr_in servaddr; 
  
    // Creating socket file descriptor 
    if ( (sd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { 
        perror("socket creation failed"); 
        exit(EXIT_FAILURE); 
    } 
  
    memset(&servaddr, 0, sizeof(servaddr)); 
      
    // Filling server information 
    servaddr.sin_family = PF_INET; 
    servaddr.sin_port = htons(port); 
    servaddr.sin_addr.s_addr = INADDR_ANY;
    
    struct udp_connection udp;
    udp.sd = sd;
    udp.servaddr = servaddr;

    return udp;
}

void test4(unsigned short port){

	int tcp1 = connect_tcp(port);
	send_tcp(tcp1,"LOGIN Morty\n");
	recv_tcp(tcp1,"OK!\n");
	send_tcp(tcp1,"WHO\n");
	recv_tcp(tcp1,"OK!\nMorty\n");

	int tcp2 = connect_tcp(port);
	send_tcp(tcp2,"LOGIN Rick\n");
	recv_tcp(tcp2,"OK!\n");
	send_tcp(tcp2,"WHO\n");
	recv_tcp(tcp2,"OK!\nMorty\nRick\n");

	send_tcp(tcp1,"LOGOUT\n");
	recv_tcp(tcp1,"OK!\n");
	send_tcp(tcp1,"LOGIN Beth\n");
	recv_tcp(tcp1,"OK!\n");

	struct udp_connection udp_c = connect_udp(port);
	struct udp_connection * udp = &udp_c;
	send_udp(udp,"WHO\n");
	recv_udp(udp,"OK!\nBeth\nRick\n");

	send_tcp(tcp1,"LOGOUT\n");
	recv_tcp(tcp1,"OK!\n");
	close(tcp1);

	send_tcp(tcp2,"LOGOUT\n");
	recv_tcp(tcp2,"OK!\n");
	close(tcp2);

	send_udp(udp,"WHO\n");
	recv_udp(udp,"OK!\n");

	close(udp->sd);
	return;
}

void test5(unsigned short port){

	int tcp1 = connect_tcp(port);
	send_tcp(tcp1,"LOGIN Rick\n");
	recv_tcp(tcp1,"OK!\n");

	int tcp2 = connect_tcp(port);
	send_tcp(tcp2,"LOGIN Rick\n");
	recv_tcp(tcp2,"ERROR Already connected\n");
	send_tcp(tcp2,"LOGIN Summer\n");
	recv_tcp(tcp2,"OK!\n");

	send_tcp(tcp1,"WHO\n");
	recv_tcp(tcp1,"OK!\nRick\nSummer\n");

	close(tcp1);
	close(tcp2);
}

void test6(unsigned short port){

	int tcp1 = connect_tcp(port);
	send_tcp(tcp1,"LOGIN Rick\n");
	recv_tcp(tcp1,"OK!\n");
	send_tcp(tcp1,"SEND Rick 22\nI'm talking to myself!\n");
	recv_tcp(tcp1,"OK!\n");
	recv_tcp(tcp1,"FROM Rick 22 I'm talking to myself!\n");
	
	int tcp2 = connect_tcp(port);
	send_tcp(tcp2,"LOGIN G\n");
	recv_tcp(tcp2,"ERROR Invalid userid\n");
	close(tcp2);

	int tcp3 = connect_tcp(port);
	send_tcp(tcp3,"LOGIN Summer\n");
	recv_tcp(tcp3,"OK!\n");
	send_tcp(tcp3,"SEND Rick 25\nStop talking to yourself!\n");
	recv_tcp(tcp3,"OK!\n");

	recv_tcp(tcp1,"FROM Summer 25 Stop talking to yourself!\n");
	send_tcp(tcp1,"LOGOUT\n");
	recv_tcp(tcp1,"OK!\n");
	close(tcp1);

	send_tcp(tcp3,"SEND Rick 20\nAre you still there?\n");
	recv_tcp(tcp3,"ERROR Unknown userid\n");
	send_tcp(tcp3,"SEND Summer 26\nNow I'm talking to myself!\n");
	recv_tcp(tcp3,"OK!\n");
	recv_tcp(tcp3,"FROM Summer 26 Now I'm talking to myself!\n");

	struct udp_connection udp_c = connect_udp(port);
	struct udp_connection * udp = &udp_c;
	send_udp(udp,"WHO\n");
	recv_udp(udp,"OK!\nSummer\n");

	close(tcp3);

}

void test7(unsigned short port){

	int tcp1 = connect_tcp(port);
	send_tcp(tcp1,"LOGIN Rick\n");
	recv_tcp(tcp1,"OK!\n");
	send_tcp(tcp1,"SEND Morty 21\nAre you there, Morty?\n");
	recv_tcp(tcp1,"OK!\n");
	
	int tcp2 = connect_tcp(port);
	send_tcp(tcp2,"LOGIN Morty\n");
	recv_tcp(tcp2,"OK!\n");
	send_tcp(tcp2,"WHO\n");
	recv_tcp(tcp2,"OK!\nMorty\nRick\n");
	
	send_tcp(tcp1,"SEND Morty 24\nAha, there u are, Morty!\n");
	recv_tcp(tcp1,"OK!\n");
	
	send_tcp(tcp2,"SEND Rick 27\nYes, idiot, I'm right here!\n");
	recv_tcp(tcp2,"OK!\n");
	send_tcp(tcp2,"BROADCAST 21\nAaaaaaaaaaaaaaaaaagh!\n");
	recv_tcp(tcp2,"OK!\n");
	
	send_tcp(tcp1, "LOGOUT\n");
	recv_tcp(tcp1,"OK!\n");
	close(tcp1);
	
	send_tcp(tcp2,"WHO\n");
	recv_tcp(tcp2,"OK!\nMorty\n");
	send_tcp(tcp2,"LOGOUT\n");
	recv_tcp(tcp2,"OK!\n");
	close(tcp2);
	
}

void test8(unsigned short port){

}

void test9(unsigned short port){

}


int main(int argc, char ** argv)
{
	unsigned short port = atoi(argv[1]);
	test7(port);
	//test5(port);
	//test6(port);

	return EXIT_SUCCESS;
}
