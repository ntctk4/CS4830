/**
*Nathaniel Callahan
*ntctk4
*lab 3
*due: 11/7/2016
*
*server.c is the "server" for this project. It creates a connection with a client
*and gives the client the options to:logon, logoff, send messages, or create a new user.
*Unless a client logs on successfully the only thing they can do is create a new user.
*Once loged in(the user name is stored so we know who is loged in) server.c will send back
*messages to the client telling them what they have done on the server side. 
*
*This program was made and runs on a linux server.
*/
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<stdio.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>

//this is a linked list struct to store all the currently made users and passwords 
//so I can check to see if they can log or have to create a new account.
struct node {
  
  char* uname;
  char* password;
  struct node *next;
};

//logout clears out the current user string so someone else can log in and message through the server
void logout(char* temp, char* current_user, int newsock);
//send_message takes the message that the user sends and repackages it so that all the connected users will
//see what it is and who sent it (in this project only one user can be connected at a time)
void send_message(char* token, char* temp2, char* buffer, char* current_user, int newsock, int sock);
//login checks to see if the sent user credintials are in the linked list. If the user name and password are in 
//the linked list and nobody is logged on then the current user is set to the users name
char* login(char* token, struct node* link, struct node* root, int newsock, char* current_user, char* temp);
//newuser adds a node in the linked list and stores the user data into a file if user name doesn't match anybody elses and neets
// the credintial requirments.
char* newuser(struct node *link, struct node *root, char* token, int newsock, char* current_user, char* current_pass, char* temp, FILE* file);

void main(int argc, char **argv)
{
  //need the text file as an argument
  if(argc < 2)
  {
    printf("not enough arguments\n");
    return;
  }
  //create the root node for the linked list
  struct node *root = malloc(sizeof(struct node));
  root->next = 0;
  
  //create variables to traverse linked list and to store data
  struct node *link;
  char* current_user;
  char* current_pass;
  
  //set link equal to root so we can fill in root's data first
  link = root;
  
  //variables for creating a socket connection
  int sock;
  int newsock;
  int port;
  int clilen;
  
  //variables used for grabing and manipulating data off the buffer.
  char buffer [1000];
  char temp [1000];
  char temp2 [1000];
  char *token;
  
  //more variables used to create a socket connection
  struct sockaddr_in serv_addr;
  struct sockaddr_in cli_addr;
  int n;
  
  //temp varibles to move data around
  char users[50];
  char *uname;
  char *password;
  int i;
  
  //open up the text file
  FILE *file = fopen(argv[1], "r");
  if(!file)
  {
    printf("file didn't open\n");
  }
  
  //while there is still data in the file create and fill nodes in the linked list
   while(fgets(users, sizeof(users), file) != NULL)
   {
   //tokenize the data to extract the usernames and passwords for the linked list
    link->uname = strdup(strtok(users, " ,()\t\r\n\v\f"));
    link->password = strdup(strtok(NULL, " ,()\t\r\n\f"));
    
    //creating new nodes for the linked list to grow
    struct node *newNode = malloc(sizeof(struct node));
    link->next = newNode;
    link = link->next;
    link->uname = malloc(sizeof(users));
    link->password = malloc(sizeof(users));
    link->next = NULL;
  }
 
   //done with file, can close it now
  fclose(file);
  
  //create a socket
  sock = socket(AF_INET, SOCK_STREAM, 0);
  if(sock < 0)
  {
    printf("socket failed to open\n");
    return;
  }
  
  bzero((char *)&serv_addr, sizeof(serv_addr));
  //port number involving student id
  int portno = 15588;
 
  //binding the created socket 
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(portno);
  if(bind(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
  {
    printf("failed to bind\n");
    return;
  }
 
  printf("My chat room server. Version One.\n"); 
  listen(sock, 5);
  clilen = sizeof(cli_addr);
  //create a new socket connetion between the server and client
  //this is what will be used to transfer data back and forth
  newsock = accept(sock, (struct sockaddr *)&cli_addr, &clilen);
  if(newsock < 0)
  {
    printf("acception failure\n");
    return;
  }
  
  //infinitly loop to get client data from newsock    
  while(1)
  {
    listen(sock, 5);
  
    //clear temp variables out
    bzero(buffer, 1000);
    bzero(temp, 1000);
    bzero(temp2, 1000);
    //get data from the client using read
    n = read(newsock, buffer, 1000);
    if(n <= 0)
    {
      printf("nothing in buffer\n");
      return;
    }
   //it the client sent anything grab the first token from the data to see which command the client chose.
    if(n > 1)
    {
      strncpy(temp, buffer, 1000);
      token = strtok(temp, " \n");
    }
    //if token isn't login, newuser, send, or logout then it is an invlid command
    if( n <= 1 || (strcmp(token, "login") != 0  && strcmp(token, "newuser") != 0  && strcmp(token, "send") != 0 && strcmp(token, "logout") != 0))
    {
      n = write(newsock, "invalid command\n", 17);
    }
    
    
    /*---------------------------------------------------*/
    else
    {
    //if the user chose login and someone else isn't already logged in then go to login function
      if(!current_user && strcmp(token, "login") != 0 && strcmp(token, "newuser") != 0)
      {
        n = write(newsock, "Server: Denied. Please login first.\n", 37); 
      }
      else if(current_user && strcmp(token, "newuser") == 0)
      {
        n = write(newsock, "Server: already loged in. Cant't create a new user.\n", 53);
      }
      else if(!current_user && strcmp(token, "login") == 0)
      {
      
        current_user = login(token, link, root, newsock, current_user, temp);
       
      }
      
      
      /*------------------------------------------------*/
      //if the user chose the newuser command, take them to the newuser function
      else if(!current_user && strcmp(token, "newuser") == 0)
      {
        file = fopen(argv[1], "a");
        current_user = newuser(link, root, token, newsock, current_user, current_pass, temp, file);
        fclose(file);
      }
      //if the user chose the logout command and they are already logged in
      //then send them to the logout function
      else if(current_user && strcmp(token, "logout") == 0)
      {
      
        logout(temp, current_user, newsock);

        free(current_user);
        current_user = NULL; 
      }
      //otherwise  it is a message and send them to the send message function
      else
      {

  
        send_message(token, temp2, buffer, current_user, newsock, sock);
      
      }
    }
      
    sleep(2);
  }
  //shutdown(sock, SHUT_RDWR);
  close(sock);
  
  return;
  
 
}

//logout clears the current user string and sends a message to the user
//letting them know of a successful logout.
void logout(char* temp, char* current_user, int newsock)
{
  int n = 0;
  bzero(temp, 1000);
  strcat(temp, "Server: ");
  strcat(temp, current_user);
  strcat(temp, " left.\n");
  n = write(newsock, temp, strlen(temp));
  printf("%s logout.\n", current_user);
  return;

}

//send message takes the message sent by the user and sends it back to all other users(just one)
//and formats it so that everyone knows who sent it.
void send_message(char* token, char* temp2, char* buffer, char* current_user, int newsock, int sock)
{
  int n = 0;
  
  size_t len = strlen(token);
  strncpy(temp2, buffer + len, 1000);

  bzero(buffer, 1000);
    
  strncpy(buffer, current_user, 33);
  strcat(buffer, ":");
    
  strcat(buffer, temp2);

  n = write(newsock, buffer, strlen(buffer));
  printf("%s", buffer);
  if(n < 0)
  {
    printf("error writing to the socket");
    close(sock);
    return;
  }
  return;
}

//checks to see if anybody is loged in and if the user credintials are correct. If all of this is good
// the user's user name will be put into the current user string and a message will be sent to the client
//so they know that login was a success or failure.
char* login(char* token, struct node *link, struct node *root, int newsock, char* current_user, char* temp)
{
  int n = 0;
  token = (strtok(NULL, " \n"));

  //check through entire linked list until user is found or their not in it
  link = root;
  while(link->next != NULL)
  {
        
        if(token == NULL)
        {
          n = write(newsock, "Server: login failed.\n", 23);
          break;
        }
        //check to see if user name matched
        if(strcmp(token, link->uname) == 0)
        {
          //need to free this. Done in logout
          //sets current user
          current_user = strdup(token);
          token = (strtok(NULL, " \n"));
          //check to see if password matches
          if(strcmp(token, link->password) == 0)
          {
            //send message to user for a successful login.
            bzero(temp, 1000);
            strcat(temp, "Server: ");
            strcat(temp, current_user);
            strcat(temp, " joins\n");
            n = write(newsock, temp, strlen(temp));
            break;
          }
          else
          {
            //send message for failed login
            free(current_user);
            current_user = NULL;
            n = write(newsock, "Server: login failed.\n", 23);
          }
        }
        //iterate through the linked list
        link = link->next;
        if(link->next == NULL)
        {
          //send message for no username found in linked list that matches user given name.
          n = write(newsock, "Server: no such user account.\n", 31);
          break;
        }
  }
  //print out server side message and return the current user.
  printf("%s login.\n", current_user);
  return current_user;
      
}

//newuser creates a new user based on the credintials the client sends to the server. If the user name matches anything in the linked list
//then fail, put new user in the text file
char* newuser(struct node *link, struct node *root, char* token, int newsock, char* current_user, char* current_pass, char* temp, FILE* file)
{
  //start at root
  link = root;
  token = (strtok(NULL, " \n"));
  int n = 0;
  //iterating through the linked list
  while(link->next != NULL)
  {
    if(token == NULL)
    {
      //user didn't provide enough info for a new account
      n = write(newsock, "Server: not enough arguments.\n", 31);
      break;
    }
    if(strcmp(token, link->uname) == 0)
    {
      //the given user name matched a name in the linked list. send error
      n = write(newsock, "Server: can't have duplicate user names.\n", 42);
      break;
    }
    link = link->next;
    }
    if(n == 0)
    {
      current_user = strdup(token);
      token = (strtok(NULL, " \n"));
      if(token == NULL)
      {
        //didn't give enough arguments for an account. send back an error to the user.
        free(current_user);
        current_user = NULL;
        n = write(newsock, "Server: not enough arguments.\n", 31);
      }
      else
      {
        current_pass = strdup(token);
        //make sure user name is less than 32 bytes and that the password is between 4 and 8 bytes
        if(strlen(current_user) > 32 || 4 > strlen(current_pass) || strlen(current_pass) > 8)
        {
          free(current_user);
          current_user = NULL;
          n = write(newsock, "Server: invalid name or password.\n", 35);
        }
        else
        {
          //create a new node for the new user in the linked list
          struct node *newNode = malloc(sizeof(struct node));
          link->next = newNode;
          link = link->next;
          link->uname = malloc(50);
          link->password = malloc(50);
          link->next = NULL;
        
          //add the node to the linked list
          //This looks weird but it was the only way to get the uname and password into the linked list
          link = root;
          while(link->next != NULL)
          {
            if(strlen(link->uname) < 2)
            {
              link->uname = strdup(current_user);
              link->password = strdup(current_pass);
            }
            link = link->next;
          }
          //put the new user info into the text file, formatted
          bzero(temp, 1000);
          strcat(temp, "(");
          strcat(temp, current_user);
          strcat(temp, ", ");
          strcat(temp, current_pass);
          free(current_pass);
          current_pass = NULL;
          strcat(temp, ")");
          n = write(newsock, "Server: new user was create\n", 29);
          printf("new user %s", temp);
          //file = fopen("users.txt", "a");    
          fprintf(file, "\n%s", temp);
          //fclose(file); 
          }
        }   
      }  
    //send a server message and return the current user since the new user starts out logged in.
   printf("\n%s account created.\n", current_user);
   return current_user; 
}