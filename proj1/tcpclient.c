/* tcp_ client.c */ 
/* Programmed by Adarsh Sethi */
/* February 19, 2017 */     

#include <stdio.h>          /* for standard I/O functions */
#include <stdbool.h>        /* Support for boolean values */
#include <stdlib.h>         /* for exit */
#include <string.h>         /* for memset, memcpy, and strlen */
#include <netdb.h>          /* for struct hostent and gethostbyname */
#include <sys/socket.h>     /* for socket, connect, send, and recv */
#include <netinet/in.h>     /* for sockaddr_in */
#include <unistd.h>         /* for close */

#define STRING_SIZE 1024

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
    bool end = false;
    char ending_choice;
    int sock_client;  /* Socket used by client */

    struct sockaddr_in server_addr;  /* Internet address structure that
                                        stores server address */
    struct hostent * server_hp;      /* Structure to store server's IP
                                        address */
    char server_hostname[STRING_SIZE]; /* Server's hostname */
    unsigned short server_port;  /* Port number used by server (remote port) */

    unsigned int msg_len;  /* length of message */                      
    int bytes_sent, bytes_recd; /* number of bytes sent or received */
    
    char choice[STRING_SIZE];
    char sentence[STRING_SIZE];  /* send message */
    char serverResponse[STRING_SIZE]; /* receive message */
    
    int transtype, accttype, amount, balance_before, balance_after, err_code; 
    int messageData[];
    int i = 0;

    int checking_account = 0;
    int savings_account = 0;
 
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

        /* User interface */
        // What action are you going to do?
        printf("What action would you like to perform?\nWithdraw (w) | Deposit (d) | Transfer (t) | Check balance (b)\n>> ");
        scanf("%s", choice);
        switch (choice) {
            case 'w':
                transtype = WITHDRAW;
                break;

            case 'd':
                transtype = DEPOSIT;
                break;

            case 't':
                transtype = TRANSFER;
                break;

            case 'b':
                transtype = CHECK_BAL;
                break;

            default: printf("You did not enter your response correctly. Please try again.");
        }

        // What account?
        printf("What account are you doing this to?\nSavings (s) | Checking (c)\n>> ");
        scanf("%s", choice);
        switch (choice) {
            case 's':
                accttype = SAVINGS;
                break;

            case 'c': 
                accttype = CHECKING;
                break;

            default: printf("You did not enter a valid response. Please try again.");
        }

        if (choice != CHECK_BAL) {
            // How much?
            printf("How much?\n>> ");
            scanf("%d", amount);
            // This is where I would put a type check, but it's supposed to be checked on the server....
        } else { amount = -1; }
        
        // Should make sentence look like so: transaction_type,account_type,amount
        sprintf(sentence,"%d,%d,%d",transtype,accttype,amount); 
        msg_len = strlen(sentence) + 1;

        /* send message */
        printf("\nSending message: %s, of size: %d\n", sentence, strlen(sentence));
        bytes_sent = send(sock_client, sentence, msg_len, 0);

        /* get response from server */
        bytes_recd = recv(sock_client, serverResponse, STRING_SIZE, 0); 

        printf("\nThis is what you got from the server: %s, and this is it's size: %d", serverResponse, strlen(serverResponse));

        char *token = strtok(serverResponse,",");
        messageData[i] = atoi(token);

        while (token != NULL) {
            i++;
            token = strtok(NULL, ",");
            messageData[i] = atoi(token);
        }

        accttype = messageData[0];
        transtype = messageData[1];
        balance_before = messageData[2];
        balance_after = messageData[3];
        err_code = messageData[4];

        if(){

        }

        // Ask if user wants to end session. If so, set end boolean to true
        printf("Are you done making transactions? Yes = 1 No = 0");
        scanf("%d", end);
        /* close the socket */
        if (end) {

            // send(sock_client, sentence, msg_len, 0);           
            close (sock_client);
        }
    }
}
