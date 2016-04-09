#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>


int main(int argc, char *argv[])
{
    int client_sockfd;
    int len;
    struct sockaddr_in remote_addr; // struct of Server internet address
    char user_input[BUFSIZ];  // user's input
    memset(&remote_addr, 0, sizeof(remote_addr)); // initialize the server address
    remote_addr.sin_family = AF_INET; // setup IP
    remote_addr.sin_addr.s_addr = inet_addr("127.0.0.1");// server address, assume it is the localhost here
    remote_addr.sin_port = htons(8000); // port number of Server

    /* Create Client's Socket. Set as IPv4, TCP */
    if((client_sockfd=socket(PF_INET,SOCK_STREAM,0))<0)
    {
        perror("Client: Fail to create Socket");
        return 1;
    }

    /* bind the socket to Client address */
    if(connect(client_sockfd,(struct sockaddr *)&remote_addr,sizeof(struct sockaddr))<0)
    {
        perror("Client: Fail to bind socket with IP");
        return 1;
    }
    printf("Client: Connecting to Server...\n");
    printf("Client: Connected to Server successfully\n\n");

    /* receive user's input and send it to server if it contains "C00L" */
    while(1) // infinite loop
    {
        printf("%s\n", "Please enter an alpha numeric string: (enter \"quit\" to quit)");
        scanf("%s",user_input);

        /* see if the input is quit */
        if (strncmp(user_input, "quit", 4) == 0) {
            send(client_sockfd,user_input,strlen(user_input),0);
            break; // jump the loop
		}

        /* if there is no "C00L" in user's input, just ignore the input */
        if (strstr(user_input, "C00L") == NULL) {
			continue;
		}

        /* if it contains "C00L" then send it to Server */
        send(client_sockfd,user_input,strlen(user_input),0);
    }

    close(client_sockfd); // close Socket
    return 0;
}
