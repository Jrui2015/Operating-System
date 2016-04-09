#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>

static volatile int catchCtrlC = 0; // store the status of Ctrl-C

void intHandler(int dummy) { // handle the interupt caused by Ctrl-C
    catchCtrlC = 1;
}

int main(int argc, char *argv[])
{
    /* Variables declaration */
    int server_sockfd;// server's Socket
    int client_sockfd;// client's Socket
    struct sockaddr_in server_addr; // struct of Server internet address
    struct sockaddr_in remote_addr; // struct of Client internet address
    int sin_size;
    char user_input[BUFSIZ];  // data that is also the user's input received by Server
    memset(&server_addr, 0, sizeof(server_addr)); // initialize the server_addr
    server_addr.sin_family = AF_INET; // setup IP
    server_addr.sin_addr.s_addr = INADDR_ANY;// set IP of server, allow every connection
    server_addr.sin_port = htons(8000); // port number of Server
    int len;   // length of data received by server
    int count; // count the number of digits
    int countTotalInput; // count the Total Number so far
    int numOfLine; // count the number of line so far
    FILE * file = NULL; // for manipulate the secrets.out file
    file = fopen("./secrets.out", "a+"); // use append mode to append data

    /* handle the interupt caused by Ctrl-C */
    struct sigaction act;
    act.sa_handler = intHandler;
    sigaction(SIGINT, &act, NULL);

    /* create Server's Socket. Set as IPv4, TCP */
    if((server_sockfd = socket(PF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Server: Fail to create Socket");
        return 1;
    }

    /* bind the socket to server address */
    if (bind(server_sockfd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) < 0)
    {
        perror("Server: Fail to bind Socket with server address");
        return 1;
    }

    /* listen to the connection request */
    listen(server_sockfd, 5);

    sin_size = sizeof(struct sockaddr_in);

    /* Waiting for the connection request coming */
    printf("Server: Waiting for connections...\n");
    if((client_sockfd = accept(server_sockfd, (struct sockaddr *)&remote_addr, &sin_size)) < 0)
    {
        perror("Server: Fail to connect with client");
        return 1;
    }
    printf("Server: get connection from client: %s\n", inet_ntoa(remote_addr.sin_addr));
    printf("(Press Ctrl-C to show Summary Information and quit)\n\n");


    /*接收客户端的数据并将其发送给客户端--recv返回接收到的字节数，send返回发送的字节数*/
    countTotalInput = 0; // every time start up the server, set this to 0 to count the total number of all input
    numOfLine = 0; // every time start up the server, set this to 0 to count the line number of all input
    while(1) // infinite loop
    {
        if (catchCtrlC == 1) { // if we got Ctrl-C in server side, print the Summary information and quit;
            printf("\n");
            printf("%s\n", "Summary:");
            printf("Number of lines received so far: %d\n", numOfLine);
            printf("Total digit count across all input: %d\n", countTotalInput);
            break;
        }

        if ((len = recv(client_sockfd,user_input,BUFSIZ,0)) > 0) { // when receiving data from client
            user_input[len]='\0';
            printf("Data received by Server: "); // print out the data that server receives for debuging
            printf("%s\n", user_input);

            /* if "quit" is received, also print out the Summary information and then quit */
            if (strncmp(user_input, "quit", 4) == 0)
            {
                catchCtrlC = 1;
                continue;
            }

            numOfLine++; // line amount increase
            count = 0; // set count to 0 every time

            /* count the amount of digits */
            char *ptr = user_input; // set a pointer to the begin of the shared memory
            char *end = ptr + strlen(user_input); // find the end of the data in shared memory
            for (ptr = user_input; ptr != end; ptr++) { // go through the data
                if (isdigit(*ptr)) { // if the char is digit, then count++
                    count++;
                }
            }

            /* output the Original Line and the Digits amount in it to "secrets.out" */
            fprintf(file, "Count Digits: %d  Original: ", count);
            fwrite(user_input, 1, strlen(user_input), file);
            fwrite("\n", 1, 1, file); // ???

            /* flush the memory to refresh the data change to the file immediately */
            fflush(file);
            countTotalInput += count; // refresh the total number of digits so far
        }
    }

    fclose(file); // close the file "secrets.out"
    close(client_sockfd);
    close(server_sockfd);
    return 0;
}
