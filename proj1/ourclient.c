/* tcp_client.c */ 
/* Programmed by Pat Hough */
/* February 19, 2017 */     

#include <stdio.h>          /* for standard I/O functions */
#include <stdlib.h>         /* for exit */
#include <string.h>         /* for memset, memcpy, and strlen */
#include <netdb.h>          /* for struct hostent and gethostbyname */
#include <sys/socket.h>     /* for socket, connect, send, and recv */
#include <netinet/in.h>     /* for sockaddr_in */
#include <unistd.h>         /* for close */

#define STRING_SIZE 1024
// TO DO MAKE SURE TO CHECK RETURN VALUE OF SEND AND RECEIVE FUNCTIONS.
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
    int end = 0;
    char bufferString[STRING_SIZE];
    int sock_client;  /* Socket used by client */

     struct sockaddr_in server_addr;  /* Internet address structure that
                                        stores server address */
    struct hostent * server_hp;      /* Structure to store server's IP
                                        address */
    char server_hostname[STRING_SIZE]; /* Server's hostname */
    unsigned short server_port;  /* Port number used by server (remote port) */

    unsigned int msg_len;  /* length of message */                      
    int bytes_sent, bytes_recd; /* number of bytes sent or received */
    
    int choice;
    char sentence[STRING_SIZE];  /* send message */
    char serverResponse[STRING_SIZE]; /* receive message */
    
    int amount, balance_before, balance_after, err_code; 
	enum trans_type transtype;
	enum acct_type accttype;
    int messageData[3];
    int i = 0;
	int acctSend;
 
    // Special numbers for our server and client to interpret

    /* Explanation Here for the ints below:   
    Client:
        withdraw:  0
        deposit:   1
        transfer:  2
        check_bal: 3

        checking:  0 
        savings:   1

        amount: [Natural #'s, -1]

    Server response:
        trans_type: [0,1,2,3]
        account_type: [0,1]
        balancebefore_trans:  [-1, Natural #'s]
        balanceafter_trans: Integer
        err_code: [0: no error, 1: insufficient funds, 2: withdrawal not multiple of 20, 3: attempting to withdraw from savings account]
            4: transaction was > 1,000,000

    Client : { account_type, transaction_type, transaction_amount }   
    Server : { account_type, trans_type, balancebefore_trans, balanceafter_trans, err_code }
    */


    /* open a socket */

    if ((sock_client = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        perror("Client: can't open stream socket");
        exit(1);
    }

    /* Note: there is no need to initialize local client address information 
            unless you want to specify a specific local port
            (in which case, do it the same way as in udpclient.c).
            The local address initialization and binding is done automatically
            when the connect function is called later, if the socket has not
            already been bound. */

    /* initialize server address information */
    printf("Enter hostname of server: ");
    scanf("%s", server_hostname);
    if ((server_hp = gethostbyname(server_hostname)) == NULL) {
        perror("Client: invalid server hostname");
        close(sock_client);
        exit(1);
    }

    printf("Enter port number for server: ");
    scanf("%hu", &server_port);

    /* Clear server address structure and initialize with server address */
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    memcpy((char *)&server_addr.sin_addr, server_hp->h_addr,
                                    server_hp->h_length);
    server_addr.sin_port = htons(server_port);

    /* connect to the server */
        
    if (connect(sock_client, (struct sockaddr *) &server_addr, 
                                    sizeof (server_addr)) < 0) {
        perror("Client: can't connect to server");
        close(sock_client);
        exit(1);
    }

    while(!end) {
	    
        // What action are you going to do?
        printf("What action would you like to perform?\nWithdraw (0) | Deposit (1) | Transfer (2) | Check balance (3)\n>> ");
        scanf("%d", &choice);
		printf("here %d\n", choice);
		fflush(stdout);
        switch (choice) {
            case 0:
                transtype = WITHDRAW;
                break;

            case 1:
                transtype = DEPOSIT;
                break;

            case 2:
                transtype = TRANSFER;
                break;

            case 3:
                transtype = CHECK_BAL;
                break;

            default: printf("You did not enter your response correctly. Please try again.\n");
                continue;
        }

        // What account?
        printf("What account are you doing this to?\nSavings (1) | Checking (0)\n>> ");
        scanf("%d", &choice);
        switch (choice) {
            case 0:
                accttype = CHECKING;
				acctSend = 0;
                break;

            case 1: 
                accttype = SAVINGS;
				acctSend = 1;
                break;

            default: printf("You did not enter a valid response. Please try again.\n");
            continue;
        }

        if (transtype != CHECK_BAL) {
            // How much?
            printf("How much?\n>> ");
            scanf("%d", &amount);
            // This is where I would put a type check, but it's supposed to be checked on the server....
        } else { amount = -1; }
        
        // Should make sentence look like so: transaction_type,account_type,amount
        sprintf(sentence,"%d,%d,%d",acctSend,transtype,amount); 
        msg_len = strlen(sentence) + 1;
        memcpy(bufferString,sentence,msg_len);
        //printf("BufferString: %s Sentence: %s\n", bufferString, sentence);
        /* send message */
        //printf("\nSending message: %s, of size: %d\n", sentence, strlen(sentence));
        bytes_sent = send(sock_client, bufferString, msg_len, 0);

        /* get response from server */
        bytes_recd = recv(sock_client, serverResponse, STRING_SIZE, 0); 

        //printf("\nThis is what you got from the server: %s, and this is it's size: %d\n", serverResponse, strlen(serverResponse));

        i=0;
        char *token = strtok(serverResponse,",");
        while (token != NULL) {
            messageData[i] = atoi(token);
            token = strtok(NULL, ",");
            i++;
        }

        accttype = messageData[0];
        transtype = messageData[1];
        balance_before = messageData[2];
        balance_after = messageData[3];
        err_code = messageData[4];

        if(err_code == 0){ // No error
            if(accttype == 0){ // checking account
                if (transtype == WITHDRAW) { // withdraw
                    printf("Your balance before was %d, your balance now is $%d.\n", balance_before, balance_after);
                } else if (transtype == DEPOSIT) { // deposit
                    printf("Your account had %d, it now has $%d left in it.\n", balance_before, balance_after);
                } else if (transtype == TRANSFER) { // transfer
                    printf("You transferred $%d from your savings account into your checking account. Your checking account now has $%d\n", amount, balance_after);
                } else { // check balance
                    printf("Your balance is: $%d\n", balance_after);
                }
            } else { // savings account
                if (transtype == WITHDRAW) { // withdraw
                    printf("Your balance before was %d, your balance now is $%d.\n",balance_before ,balance_after);
                } else if (transtype == DEPOSIT) { // deposit
                    printf("Your account had %d, it now has $%d left in it.\n",balance_before, balance_after);
                } else if (transtype == TRANSFER) { // transfer
                     printf("You transferred $%d from your checking account into your savings account. Your savings account now has $%d\n", amount, balance_after);
                } else { // check balance
                    printf("Your balance is: $%d\n", balance_after);
                }
            }
        } else if (err_code == 1) { // Insufficient funds error
            printf("Sorry, You don't have the necessary funds in your checking account\n");
            continue;
        } else if (err_code == 2) { // #%20 != 0
            printf("You're withdrawal amount must be a multiple of 20!\n");
            continue;
       } else if (err_code == 3) { // Can't withdraw from savings account
            printf("Error, you can't withdraw from a savings account, only a checking account.\n");
            continue;
      } else { // can't make transaction with amount > 1000000
            printf("Error, you can't make a transaction with an amount that's larger than 1,000,000. Please try again, with a smaller amount.\n");
            continue;
     }
        // Ask if user wants to end session. If so, set end boolean to true
        printf("Are you done making transactions? Yes = 1 No = 0\n>> ");
        scanf("%d", &end);
        /* close the socket */
        if (end) {
            // send closing packet to server to tell it to cut connection
            accttype = -1;
            msg_len = strlen(sentence) + 1;
            sprintf(sentence,"%d,%d,%d",accttype,transtype,amount);
			memcpy(bufferString,sentence,msg_len);
			//printf("BufferString: %s Sentence: %s\n", bufferString, sentence);
            send(sock_client, bufferString, msg_len, 0);           
            close (sock_client);
        }
    }
}
