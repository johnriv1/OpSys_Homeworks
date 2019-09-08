This program implements a server that accepts UDP messages and TCP connections/messages. These messages allow for TCP clients to send messages to other TCP clients or UDP clients to send messages to TCP clients. The server acts as a router for multples connections to communicate. To handle multiple TCP connections, multi-threading is used.

Client messages: 
	
	LOGIN <userid>\n
	
	WHO\n
	
	LOGOUT\n
	
	SEND <recipient-userid> <msglen>\n<message>
	
	BROADCAST <msglen> <message>\n
	
	SHARE <recipient-userid> <filelen>\n
			/*Wait for "OK!\n" from server and then send file*/
			<filebytes>

/*
	client.exe is from rpi_opsys_hw4_client_2.0.c
	server.exe is from hw4_3.c (main program)
*/

Run main code in terminal:

	./server 8128

Use client(s) in terminal:

	./client 8128 UDP
	./client 8128 TCP
	
	or
	
	netcat localhost 8128

/* main code is correct as per homework requirements, but test client may have some minor bugs */
