/* ourserver.c */
/* Programmed by Zac Merritt */
/* February 19, 2017 */
#include <stdio.h>          /* for standard I/O functions */
#include <stdlib.h>         /* for exit */
#include <string.h>         /* for memset */
#include <sys/socket.h>     /* for socket, bind, listen, accept */
#include <netinet/in.h>     /* for sockaddr_in */
#include <unistd.h>         /* for close */
#define STRING_SIZE 1024
/* SERV_TCP_PORT is the port number on which the server listens for
incoming requests from clients. You should change this to a different
number to prevent conflicts with others in the class. */
#define SERV_TCP_PORT 3220

enum trans_type {
    WITHDRAW,
    DEPOSIT,
    TRANSFER,
    CHECK_BAL
};

enum acct_type {
    CHECKING,
    SAVINGS
};

int main(void) {
    
	int sock_server;  /* Socket on which server listens to clients */
    int sock_connection;  /* Socket on which server exchanges data with client */
    struct sockaddr_in server_addr;  /* Internet address structure that
                                        stores server address */
    unsigned int server_addr_len;  /* Length of server address structure */
    unsigned short server_port;  /* Port number used by server (local port) */
    struct sockaddr_in client_addr;  /* Internet address structure that
                                        stores client address */
    unsigned int client_addr_len;  /* Length of client address structure */
    char sentence[STRING_SIZE];  /* receive message */
    char strBuf[STRING_SIZE]; /* temp buffer before memcpy, to build the string */
    unsigned int msg_len;  /* length of message */
    int bytes_sent, bytes_recd; /* number of bytes sent or received */
    unsigned int i;  /* temporary loop variable */
    
	int messageData[3]; /* array of ints from parsed message(sentence)
	[0] = acct
	[1] = trans_type
	[2] = trans_amount (-1 if N/A)
	*/
    
	int accountBalances[2];/* Array with the account balances
	[0] = CHECKING
	[1] = SAVINGS
	*/
    
	/* initialize account info */
    accountBalances[0] = 0;
    accountBalances[1] = 0;
	
	int errorCode = 0; /* error code server sends back to client
	0: no error
	1: insufficient funds
	2: withdraw not in 20s
	3: withdraw no checking acct
	4: transaction amount incorrect (over 1000000 or under -1)
	*/
    
	int outArr[5]; /* array of final values
	[0]: acct type
	[1]: trans type
	[2]: before
	[3]: after
	[4]: error code
	*/
    
	enum acct_type clientAcct; /* account given by client message */
    enum trans_type clientTrans; /* transaction type given by client */
    
	/* open a socket */
    if ((sock_server = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        perror("Server: can't open stream socket");
        exit(1);
    }
    
	/* initialize server address information */
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl (INADDR_ANY);  /* This allows choice of
                                        any host interface, if more than one
                                        are present */
    server_port = SERV_TCP_PORT; /* Server will listen on this port */
    server_addr.sin_port = htons(server_port);
    
	/* bind the socket to the local server port */
    if (bind(sock_server, (struct sockaddr *) &server_addr,
             sizeof (server_addr)) < 0) {
        perror("Server: can't bind to local address");
        close(sock_server);
        exit(1);
    }
	
    /* listen for incoming requests from clients */
    if (listen(sock_server, 50) < 0) {    /* 50 is the max number of pending */
        perror("Server: error on listen"); /* requests that will be queued */
        close(sock_server);
        exit(1);
    }
	
    printf("I am here to listen ... on port %hu\n\n", server_port);
    client_addr_len = sizeof (client_addr);
	int j =0;

	
	//wait for connection from client
	sock_connection = accept(sock_server, (struct sockaddr *) &client_addr,
							 &client_addr_len);

	/* The accept function blocks the server until a
	   connection request comes from a client */

	if (sock_connection < 0) {
		perror("Server: accept() error\n");
		close(sock_server);
		exit(1);
	}


    /* wait for message */
    for (;;) {
        /* receive the message */
		
        bytes_recd = recv(sock_connection, sentence, STRING_SIZE, 0);
		if(bytes_recd<=0){
			perror("Server: error on recv, bytes recd 0 or negative"); /* requests that will be queued */
			close(sock_server);
			exit(1);
		}
		
        //message syntax ["int,int,int"] --> "acct_type,trans_type,trans_amount"
		/*
		[0] = acct [0: checking, 1: savings]
		[1] = trans_type [0: withdraw, 1: dep, 2: trans, 3: check bal] 
		[2] = trans_amount (-1 if N/A)		*/
		
		
		
		printf("Received %s it is %d bytes long\n", sentence, bytes_recd );
		i = 0;
        char *token = strtok(sentence, ",");
		
        while (token != NULL)
        {
			messageData[i] = atoi(token);
            token = strtok(NULL, ",");
            i++;
        }
		
		errorCode = 0;
		
        //check account type from message we got
		//if account type is -1 the client is ready to disconnect, so close the socket and wait for connection
		switch (messageData[0]) {
			case 0:
				clientAcct = CHECKING;
				outArr[0] = 0;
				break;
			case 1:
				clientAcct = SAVINGS;
				outArr[0] = 1;
				break;
			default:
				close(sock_connection);
				printf("\nSocket closed because client told us to terminate the connection.\nWaiting for new connection.\n\n");
				sock_connection = accept(sock_server, (struct sockaddr *) &client_addr,
						 &client_addr_len);

				/* The accept function blocks the server until a
				   connection request comes from a client */

				if (sock_connection < 0) {
					perror("Server: accept() error\n");
					close(sock_server);
					exit(1);
				}
				continue;
				break;

		}
		
		

		// check transaction type from message we got
		/*
		0 WITHDRAW
		1 DEPOSIT
		2 TRANSFER
		3 CHECK_BAL
		*/
		switch (messageData[1]) {
			case 0:
				clientTrans = WITHDRAW;
				break;
			case 1:
				clientTrans = DEPOSIT;
				break;
			case 2:
				clientTrans = TRANSFER;
				break;
			case 3:
				clientTrans = CHECK_BAL;
				break;
		}
		
		
		//get balance before
		if((clientAcct == CHECKING && clientTrans != TRANSFER) || (clientAcct == SAVINGS && clientTrans == TRANSFER)){
			outArr[2] = accountBalances[0];
		}
		else{
			outArr[2] = accountBalances[1];
		}
		
		//transaction amount was over limit, send error code
		if (messageData[2] > 1000000 || messageData[2] < -1) {	
			errorCode = 4;
		}
		
		
		//do stuff based on the transaction type
		if (errorCode == 0) {
			switch (clientTrans) {
			
				case WITHDRAW:
					//do withdraw (only on checking)
					outArr[1] = 0;
					if (clientAcct == SAVINGS) {
						errorCode = 3;
					}
					else if ((messageData[2] % 20) != 0 ) {

						errorCode = 2;
					}
					else if ( accountBalances[0]>=messageData[2] ) {
						accountBalances[0] -= messageData[2];
					}
					else {
						errorCode = 1;
					}
					break;	
					
				case DEPOSIT:
					//add input to account specified
					outArr[1] = 1;
					if (clientAcct == CHECKING){
						accountBalances[0] += messageData[2];
					}
					else{
						accountBalances[1] += messageData[2];
					}
					break;
					
				case TRANSFER:
					//MAKE SURE THE MESSAGE CONTAINS INFO FOR THE ACCOUNT INTO WICH THE MONEY WENT
					outArr[1] = 2;					
					if (clientAcct == CHECKING && (accountBalances[0]>=messageData[2]) ) {
						outArr[0] = 1;
						accountBalances[0] -= messageData[2];
						accountBalances[1] += messageData[2];
					}
					else if (clientAcct == SAVINGS && (accountBalances[1]>=messageData[2]) ) {
						outArr[0] = 0;
						accountBalances[1] -= messageData[2];
						accountBalances[0] += messageData[2];
					}
					else {

						errorCode = 1;
					}
					break;
					
				case CHECK_BAL:
					outArr[1] = 3;
					break;
					
				default:
					break;
					//someone messed up, this should never happen
			}
			
		}
		
		//get balance after
		if ((clientAcct == CHECKING && clientTrans != TRANSFER) || (clientAcct == SAVINGS && clientTrans == TRANSFER))
			outArr[3] = accountBalances[0];
		else
			outArr[3] = accountBalances[1];
		
		outArr[4] = errorCode;
		

		//build string "acct,type,before,after,error"
		sprintf(strBuf,"%d,%d,%d,%d,%d",outArr[0],outArr[1],outArr[2],outArr[3],outArr[4]);
		
		msg_len = strlen(strBuf) + 1;
		
		memcpy(sentence, strBuf, msg_len);
		
		
		/* send message */
		bytes_sent = send(sock_connection, sentence, msg_len, 0);
		if(bytes_sent<=0){
			perror("Server: error on send, bytes sent 0 or negative"); /* requests that will be queued */
			close(sock_server);
			exit(1);
		}
        printf("Sent string %s of size %d.\n\n",sentence,bytes_sent);
		
        /* close the socket */
        
    }
	close(sock_connection);
}