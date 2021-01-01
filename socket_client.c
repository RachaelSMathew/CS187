#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <strings.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <ctype.h>       // toupper

#define    MAXLINE     8192  /* Max text line length */

int open_clientfd(char *hostname, int port) {
  // The client's socket file descriptor.
  int clientfd;

  struct sockaddr_in serveraddr;

  // First, we create the socket file descriptor with the given
  // protocol and protocol family.
  if ((clientfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    return -1;



  // The socket API requires that you zero out the bytes!
  bzero((char *) &serveraddr, sizeof(serveraddr));

  // Record the protocol family we are using to connect.
  serveraddr.sin_family = AF_INET;

  // Copy the IP address provided by DNS to our server address
  // structure.
    serveraddr.sin_addr.s_addr = inet_addr(hostname);
    
  // Convert the port from host byte order to network byte order and
  // store this in the server address structure.
  serveraddr.sin_port = htons(port);
    

  // Establish a connection with the server.
    if (connect(clientfd, (struct sockaddr *) &serveraddr, sizeof(serveraddr)) < 0) {
        return -1;
    }

  return clientfd;
}

int main(int argc, char **argv)
{
  // The client socket file descriptor.
  int clientfd;

  // The port number.
  int port;

  // Variable to store the host/server domain name.
  char *host;

  // A buffer to receive data from the server.
  char buf[MAXLINE];

    // First, we check the program arguments:
    if (argc != 4) {
      fprintf(stderr, "usage: %s <host> <port>\n", argv[0]);
      exit(0);
    }

    // fourth argument is host, third is port:
    host = argv[3];

    port = atoi(argv[2]);


    // Open the client socket file descriptor given the host and port:
    clientfd = open_clientfd(host, port);

    //send the first message to the server
    char connectServer[1024];
    sprintf(connectServer, "cs230 HELLO %s\n", argv[1]);

    //first messsage(connectServer) is sent to server
  if(send(clientfd, connectServer, strlen(connectServer), 0) == -1) {return -1;}
    int bytesRecv = 0;
    //the while loop will keep running until the server stop sending messages
    while((bytesRecv = recv(clientfd, buf, MAXLINE, 0)) > 0) {
    // Add a newline back on:
    buf[bytesRecv + 1] = '\0';
    buf[bytesRecv] = '\n';
      
    //prints the server message
    printf("%s", buf);
      
    //count the number of words in the buffer
    int i, count = 0;
    for (i = 0;buf[i] != '\0';i++)
    {
      if (buf[i] == ' ' && buf[i+1] != ' ')
          count++;
    }
    //if there are only 3 words, meaning that it is the final message sent by the server, the flag is captured
    if(count+1 == 3) {
        char *ptr = strtok(buf, " ");
        ptr = strtok(NULL, " ");
        printf("This is the flag %s\n", ptr);
        break;
    }
    //ptr is the first number in the server message
    char *ptr = strtok(&buf[13], " ");
    fflush(stdout);
      
    //the two numbers and math operator in the server message is put into a string array
    char *array[3];
    int k = 0;
    while(ptr != NULL)
    {
        array[k] = ptr;
        ptr = strtok(NULL, " ");
        k++;
    }
      int firstNum = atoi(array[0]);
      int thirdNum = atoi(array[2]);
      int newNum = 0;
      //based on the math operator in the server message, a certain math operation is done
      if(strcmp(array[1], "+") == 0) {
          newNum = firstNum + thirdNum;
      }
      if(strcmp(array[1], "-") == 0) {
          newNum = firstNum - thirdNum;
      }
      if(strcmp(array[1], "*") == 0) {
          newNum = firstNum * thirdNum;
      }
      if(strcmp(array[1], "/") == 0) {
          newNum = firstNum / thirdNum;
      }
      sprintf(buf, "cs230 %d\n", newNum);
      printf("%s", buf);
      fflush(stdout);
      //send a message to server
      send(clientfd, buf, strlen(buf), 0);

  }

  // Close the file descriptor:
  close(clientfd);
 
  exit(0);
}
