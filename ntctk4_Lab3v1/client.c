/**
*Nathaniel Callahan
*ntctk4
*lab 3
*due: 11/7/2016
*
*client.c is the client side of this chatroom. The main purpose of this program is to create a connection with the server and to send and revice messages from the server
*this program was made and ran on a linux server.
*/
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<netinet/in.h>
#include<netdb.h>

//#define SERVER_PORT 9999
#define MAX_LINE 256


void main(int argc, char **argv)
{
  //create variables to help with creating a socket and connecting to the server
  int sock;
  int portno = 15588;
  int n;
  
  struct sockaddr_in server_address;
  struct hostent *server;
  
  //buffers to hold sent and recived messages
  char message[1000];
  char server_reply[2000];
  
  //create a socket
  sock = socket(AF_INET, SOCK_STREAM, 0);
  if(socket < 0)
  {
    printf("\nFailed to open socket");
    return;
  }
  
  //not sure if server name should be an argument to take in
  //not specified in lab doc
  server = gethostbyname("ip-172-31-28-94"); 
  if(server == NULL)
  {
    printf("Failed to connect to server\n");
    return;
  }
  
  //get variables ready to connect to the server
  bzero((char *)&server_address, sizeof(server_address));
  server_address.sin_family = AF_INET;
  bcopy((char *)server->h_addr, (char *)&server_address.sin_addr.s_addr, server->h_length);
  server_address.sin_port = htons(portno);
  
  //on success we have connected to server.c and can now send messages back and forth.
  if(connect(sock, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
  {
    printf("failed to connect");
    return;
  }
  printf("My chat room client. Version One.\n");
  while(1)
  {
    //this makes sure that the message sent won't be larger than the buffer on the server side
    fgets(message, 1000, stdin);
  
    //user writes a message to be sent to the server to proccess
    n = write(sock, message, strlen(message));
    if(n < 0)
    {
      printf("error writing to the socket\n");
      return;
    }
  
    //clear message buffer to it can be used to get the servers message
    bzero(message, strlen(message));
    
    //get the servers message
    recv(sock, message, 1000, 0);
  
    //print out the command line what the server sends
    printf("%s", message);
    
    //clear out message buffer so the user can write a new message
    //this all goes on throgh an infinfate loop until the client is doen (hit ctrl c)
    bzero(message, strlen(message));
  
  }
  return;
  
}