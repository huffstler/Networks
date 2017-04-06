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
    char modifiedSentence[STRING_SIZE]; /* send message */
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
    
	/* initialize account info */
    accountBalances[0] = 0;
    accountBalances[1] = 0;
    
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
	
    /* wait for incoming connection requests in an indefinite loop */
    for (;;) {
	/*
        sock_connection = accept(sock_server, (struct sockaddr *) &client_addr,
                                 &client_addr_len);
								 
        /* The accept function blocks the server until a
           connection request comes from a client */
        /*
		if (sock_connection < 0) {
            perror("Server: accept() error\n");
            close(sock_server);
            exit(1);
        }
		
        /* receive the message */
		/*
        bytes_recd = recv(sock_connection, sentence, STRING_SIZE, 0);
        /*DO STUFF IN HERE WITH THE MESSAGE YOU RECIEVED*/
        //message syntax ["int,int,int"] --> "acct_type,trans_type,trans_amount"
        
		//sentence structure
		/*
		[0] = acct [0: checking, 1: savings]
		[1] = trans_type [0: withdraw, 1: dep, 2: trans, 3: check bal] 
		[2] = trans_amount (-1 if N/A)
		
		
		
		
		
		
		
		*/
		char s1[] = "0,1,1000";
		strcpy(sentence,s1);
		
		printf("we received %s\n",sentence);
		
		
		i = 0;
        char *token = strtok(sentence, ",");
		
        while (token != NULL)
        {
			printf("adding %d to slot %d\n",atoi(token), i);
			messageData[i] = atoi(token);
            token = strtok(NULL, ",");
            i++;
        }

        //check account
		printf("checking account from message got %d\n", messageData[0]);
		switch (messageData[0]) {

			case 0:
			printf("we are checking\n");
				clientAcct = CHECKING;
				outArr[0] = 0;
				break;
			case 1:
				clientAcct = SAVINGS;
				outArr[0] = 1;
				break;
			default:
				//someone messed up
				printf("client sent us garbage account information.\n");
				break;

		}
		
		//get balance before
		switch (clientAcct) {
			case CHECKING:
				outArr[2] = accountBalances[0];
				break;
			case SAVINGS:
				outArr[2] = accountBalances[1];
				break;
		}
		
		//check transaction type
		/*
		0 WITHDRAW
		1 DEPOSIT
		2 TRANSFER
		3 CHECK_BAL
		*/
		if (messageData[2] > 1000000 || messageData[2] < -1) {
			//transaction amount was over limit, send error code
			printf("we got a transaction amount in the negatives, or over 1,000,000");
			errorCode = 4;
		}
		
		if (errorCode == 0) {
			switch (messageData[1]) {
				case 0:
					//do withdraw (only on checking)
					clientTrans = WITHDRAW;
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
				case 1:
					printf("we are depositing\n");
					clientTrans = DEPOSIT;
					outArr[1] = 1;
					if (clientAcct == CHECKING){
						printf("were checking and epositing\n");
						accountBalances[0] += messageData[2];
					}
					else{
						printf("were savings and epositing\n");
						accountBalances[1] += messageData[2];
					}
					break;
				case 2:
				//MAKE SURE THE MESSAGE CONTAINS INFO FOR THE ACCOUNT INTO WICH THE MONEY WENT
					clientTrans = TRANSFER;
					outArr[1] = 2;					
					if (clientAcct == CHECKING && (accountBalances[0]>=messageData[2]) ) {
						accountBalances[0] -= messageData[2];
						accountBalances[1] += messageData[2];
					}
					else if (clientAcct == SAVINGS && (accountBalances[1]>=messageData[2]) ) {
						accountBalances[1] -= messageData[2];
						accountBalances[0] += messageData[2];
					}
					else {
						errorCode = 1;
					}
					break;
				case 3:
					clientTrans = CHECK_BAL;
					outArr[1] = 3;
					break;
				default:
					//someone messed up, this should never happen
					printf("Client sent us garbage in the trans type slot.\n");
			}
			
		}
		
		//get balance after
		if (clientAcct == CHECKING)
			outArr[3] = accountBalances[0];
		else
			outArr[3] = accountBalances[1];
		
		outArr[4] = errorCode;
		
		//build string "acct,type,before,after,error"
		sprintf(sentence,"%d,%d,%d,%d,%d",outArr[0],outArr[1],outArr[2],outArr[3],outArr[4]);
		msg_len = strlen(sentence) + 1;
		
		printf("about to send string %s of size %d.\n",sentence,msg_len);
		
		/* send message */
		//bytes_sent = send(sock_connection, sentence, msg_len, 0);
        
		
        /* close the socket */
        //close(sock_connection);
    }
}