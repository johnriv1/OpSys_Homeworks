#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>

void TCPClient(int portNumber);
void UDPClient(int portNumber);

//int STDIN_FILENO = 0;

int main(int argc, char** argv)
{
    if (argc < 3)
    {
        printf("Need port number and protocol arguments\n");
        return EXIT_FAILURE;
    }

    int portNo = atoi(argv[1]);
    if (strcmp(argv[2], "UDP") == 0)
    {
        // UDP to connect
        UDPClient(portNo);
    }
    else 
    {
        // we're using TCP to connect
        TCPClient(portNo);
    }

}



void TCPClient(int portNumber)
{
    struct sockaddr_in servaddr;
    struct hostent * hp = gethostbyname( "127.0.0.1" );
    if (hp == NULL)
    {
        return;
    }
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    memcpy( (void *)&servaddr.sin_addr, (void *)hp->h_addr, hp->h_length );
    servaddr.sin_port = htons(portNumber);

    int servFD = socket(PF_INET, SOCK_STREAM, 0);

    if (connect(servFD, (struct sockaddr*)&servaddr, sizeof(servaddr)) == -1)
    {
        perror("Connect fail");
        return;
    }
    size_t input_size = 1024;
	char* line = calloc(1024, sizeof(char));
    // TCP users must always login first
    do
    {
        printf("Enter Username: ");
        fflush(stdout);
        getline(&line, &input_size, stdin);

    }while(strlen(line) < 4 || strlen(line) > 14);
    char* msg = calloc(6 + strlen(line) + 1, sizeof(char));
    strcpy(msg, "LOGIN ");
    strcpy(&msg[6], line);
    send(servFD, msg, strlen(msg), 0);
    free(msg);
    char * username = calloc(strlen(line) + 1, sizeof(char));
    strcpy(username, line); // store the username locally
    // assume immediate server response
    recv(servFD, line, input_size, 0);
    printf("%s\n", line);
    fd_set rset; //used for select
    int maxfd = servFD + 1;// we only care about stdin and servFD
    // servFD will always be a higher descriptor than stdin
    
    do
    {
        printf("ENTER COMMAND\t {SEND, BROADCAST, WHO, QUIT}\n");
        bzero(line, input_size);
        FD_ZERO(&rset);
        FD_SET(servFD, &rset); // our TCP port
        FD_SET(STDIN_FILENO, &rset);
        select(maxfd, &rset, NULL, NULL, NULL);
        if (FD_ISSET(servFD, &rset))
        {
            // we got a message from the server
            // print it out
            int n = recv(servFD, line, input_size, 0);
            printf("%s\n", line);
            if (n == 0)
            {
                break;

            }
            if (strncmp(line, "SHARE", 5) == 0)
            {
                // someone is sharing a file with us,
                // but no name is specified, so let's make one up and get ready
                FILE* f;
                f = fopen("sharedMessage.txt", "w+");
                if (f == NULL)
                {
                    printf("FAILED TO OPEN FILE\n");
                    continue;
                }
                do
                {
                    bzero(line, input_size);
                    n = recv(servFD, line, input_size, 0);
                    //printf("%d\t%s\n", n, line);
                    for (int i = 0; i < n; i++)
                    {
                        fprintf(f, "%c", line[i]);
                    }

                } while (n == 1024);
                printf("Done receiving file\n");
                fclose(f);
            }
        }
        else if (FD_ISSET(STDIN_FILENO, &rset))
        {
            getline(&line, &input_size, stdin);
	        if (strncmp(line, "WHO", 3) == 0)
            {
                send(servFD, "WHO\0", 3, 0);
            }
            else if (strncmp(line, "SEND", 4) == 0)
            {
                printf("Enter recipient username: ");
                fflush(stdout);
                char* targetUsername = calloc(input_size, sizeof(char));
                getline(&targetUsername, &input_size, stdin);
                // remove the trailing \n
                targetUsername[strlen(targetUsername)-1] = '\0';
                printf("Enter message to send: ");
                fflush(stdout);
                bzero(line, input_size);
                getline(&line, &input_size, stdin);
                char msglen[10];
                bzero(msglen, 10);
                sprintf(msglen, "%ld", strlen(line));
                //itoa(strlen(line), msglen, 10);
                char* msg = calloc(5 + strlen(targetUsername) + strlen(msglen) + strlen(line) + 2, sizeof(char));
                int index = 0;
                strcpy(msg, "SEND ");
                index = 5;
                strcpy(&msg[index], targetUsername);
                index +=  strlen(targetUsername);
                // add a space after the username
                msg[index] = ' ';
                index++;
                strcpy(&msg[index], msglen);
                index += strlen(msglen);
                // now add the \n after msglen
                msg[index] = '\n';
                index++;
                strcpy(&msg[index], line);
                index += strlen(line); //this is now actual message length
                send(servFD, msg, index, 0);
                free(targetUsername);
                free(msg);


            }
            else if (strncmp(line, "BROADCAST", 9) == 0)
            {
                printf("Enter message to Broadcast: ");
                fflush(stdout);
                bzero(line, input_size);
                getline(&line, &input_size, stdin);
                // cut off the newline
                line[strlen(line) - 1] = '\0';
                char msglen[10];
                bzero(msglen, 10);
                sprintf(msglen, "%ld", strlen(line));
                char* msg = calloc(10 + strlen(msglen) + strlen(line) + 1, sizeof(char));
                int index = 0;
                strcpy(msg, "BROADCAST ");
                index = 10;

                strcpy(&msg[index], msglen);
                index += strlen(msglen);

                msg[index] = '\n';
                index++;

                strcpy(&msg[index], line);
                index += strlen(line);
                // index is now the true message length
                send(servFD, msg, index, 0);
                free(msg);
            }
            else if (strncmp(line, "QUIT", 4) == 0)
            {
                break;
            }
            else if (strncmp(line, "SHARE", 5) == 0)
            {
                // we can get the file from the user
                // and send it along
                bzero(line, input_size);
                printf("Enter filename in this directory, to send: ");
                fflush(stdout);
                getline(&line, &input_size, stdin);
                line[strlen(line)-1] = '\0'; //cut off the \nggg
                FILE* f;
                f = fopen(line, "r");
                if (f == NULL)
                {
                    // file does not exist
                    // we should validate this in the client 
                    // before sending the sharemessage
                    printf("File not found\n");
                    continue;
                }
                printf("Enter recipient username: ");
                fflush(stdout);
                char* targetUsername = calloc(input_size, sizeof(char));
                getline(&targetUsername, &input_size, stdin);
                // remove the trailing \n
                targetUsername[strlen(targetUsername)-1] = '\0';
                
                printf("Enter File Length in bytes: ");
                fflush(stdout);
                bzero(line, input_size);
                getline(&line, &input_size, stdin);
                
                char* msg = calloc(strlen(targetUsername) + strlen(line) + 5 + 3, sizeof(char));
                int index = 0;
                strcpy(msg, "SHARE ");
                index = 6;

                strcpy(&msg[index], targetUsername);
                index += strlen(targetUsername);
                
                msg[index] = ' '; // replace null term with space
                index++;

                strcpy(&msg[index], line);
                index += strlen(line);

                msg[index] = '\n';
                index++; // now the full message size
                send(servFD, msg, index, 0);
                // we now wait for an OK!, then get the file from our user,
                // chunk it and start sending
                recv(servFD, line, input_size, 0);
                if (strncmp(line, "OK!\n", 4) == 0)
                {
                    short done = 0;
                    char fileChunk[1025];
                    bzero(fileChunk, 1025);
                    do
                    {
                        bzero(fileChunk, 1024);
                        int i = 0;
                        for(; i < 1024; i++)
                        {
                            fileChunk[i] = getc(f);
                            if (fileChunk[i] == EOF)
                            {
                                fileChunk[i] = '\0';
                                done = 1;
                                break;
                            }
                        }
                        //printf("Sending\n%s\n", fileChunk);
                        // now we need to send filechunk
                        send(servFD, fileChunk, i, 0);
                    } while (done == 0);
                    printf("Done sending file\n");
                    continue; // we won't get an ok from the server

                }
                else
                {
                    printf("ERROR: %s\n", line);
                    continue;
                }
                
            }
            else
            {
                printf("Invalid command\n");
            }
            bzero(line, input_size);
            int n = recv(servFD, line, input_size, 0);
            printf("%d \t %s\n", n, line);
        }
        else
        {
            printf("SELECT triggered on unrecognized source\n");
        }
        
	    
    } while (1);
    
    close(servFD);
    free(line);
}


void UDPClient(int portNumber)
{
    struct sockaddr_in servaddr;
    struct hostent * hp = gethostbyname( "127.0.0.1" );
    if (hp == NULL)
    {
        return;
    }
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    memcpy( (void *)&servaddr.sin_addr, (void *)hp->h_addr, hp->h_length );
    servaddr.sin_port = htons(portNumber);

    int servFD = socket(PF_INET, SOCK_DGRAM, 0);

    size_t input_size = 1024;
	char* line = calloc(1024, sizeof(char));
    
    fd_set rset; //used for select
    int maxfd = servFD + 1;// we only care about stdin and servFD
    // servFD will always be a higher descriptor than stdin
    
    do
    {
        printf("ENTER COMMAND\t {SEND, BROADCAST, WHO, QUIT}\n");
        bzero(line, input_size);
        FD_ZERO(&rset);
        FD_SET(servFD, &rset); // our TCP port
        FD_SET(STDIN_FILENO, &rset);
        select(maxfd, &rset, NULL, NULL, NULL);
        if (FD_ISSET(servFD, &rset))
        {
            // we got a message from the server
            // print it out
            recv(servFD, line, input_size, 0);
            printf("%s\n", line);
        }
        else if (FD_ISSET(STDIN_FILENO, &rset))
        {
            getline(&line, &input_size, stdin);
	        if (strncmp(line, "WHO", 3) == 0)
            {
                sendto(servFD, "WHO\0", 3, 0, (struct sockaddr*) &servaddr, sizeof(servaddr));
            }
            else if (strncmp(line, "SEND", 4) == 0)
            {
                printf("Enter recipient username: ");
                fflush(stdout);
                char* targetUsername = calloc(input_size, sizeof(char));
                getline(&targetUsername, &input_size, stdin);
                // remove the trailing \n
                targetUsername[strlen(targetUsername)-1] = '\0';
                printf("Enter message to send: ");
                fflush(stdout);
                bzero(line, input_size);
                getline(&line, &input_size, stdin);
                char msglen[10];
                bzero(msglen, 10);
                sprintf(msglen, "%ld", strlen(line));
                //itoa(strlen(line), msglen, 10);
                char* msg = calloc(5 + strlen(targetUsername) + strlen(msglen) + strlen(line) + 2, sizeof(char));
                int index = 0;
                strcpy(msg, "SEND ");
                index = 5;
                strcpy(&msg[index], targetUsername);
                index +=  strlen(targetUsername);
                // add a space after the username
                msg[index] = ' ';
                index++;
                strcpy(&msg[index], msglen);
                index += strlen(msglen);
                // now add the \n after msglen
                msg[index] = '\n';
                index++;
                strcpy(&msg[index], line);
                index += strlen(line); //this is now actual message length
                sendto(servFD, msg, index, 0, (struct sockaddr*) &servaddr, sizeof(servaddr));
                free(targetUsername);
                free(msg);


            }
            else if (strncmp(line, "BROADCAST", 9) == 0)
            {
                printf("Enter message to Broadcast: ");
                fflush(stdout);
                bzero(line, input_size);
                getline(&line, &input_size, stdin);
                // cut off the newline
                line[strlen(line) - 1] = '\0';
                char msglen[10];
                bzero(msglen, 10);
                sprintf(msglen, "%ld", strlen(line));
                char* msg = calloc(10 + strlen(msglen) + strlen(line) + 1, sizeof(char));
                int index = 0;
                strcpy(msg, "BROADCAST ");
                index = 10;

                strcpy(&msg[index], msglen);
                index += strlen(msglen);

                msg[index] = '\n';
                index++;

                strcpy(&msg[index], line);
                index += strlen(line);
                // index is now the true message length
                sendto(servFD, msg, index, 0, (struct sockaddr*) &servaddr, sizeof(servaddr));
                free(msg);
            }
            else if (strncmp(line, "QUIT", 4) == 0)
            {
                break;
            }
            else
            {
                printf("Invalid command\n");
            }
            bzero(line, input_size);
            int n = recv(servFD, line, input_size, 0);
            printf("%d \t %s\n", n, line);
        }
        else
        {
            printf("SELECT triggered on unrecognized source\n");
        }
        
	    
    } while (1);
    
    close(servFD);
    free(line);
}
