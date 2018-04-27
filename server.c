#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#define BUFFSIZE 1024
#define PORT 8000

//declare global variables
int sock_desc, ret;
struct sockaddr_in server_addr;
int client_sock;
struct sockaddr_in client_addr;
socklen_t addr_size;
char buffer[BUFFSIZE];
pid_t childpid;
char *login;

int authenticateUser(char *login);
void recieveClientFile(int client_sock);

int main(){

  int authResult = 0;
  // 2 - Create the socket
  sock_desc = socket(AF_INET, SOCK_STREAM, 0);
  if(sock_desc < 0){
    printf("Error creating socket.\n");
    exit(1);
  }
  printf("Server socket has been created.\n");

  // 3 - Initialise the socket
  //memset(&server_addr, '\0', sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(PORT);
  server_addr.sin_addr.s_addr = INADDR_ANY;

  // 4 - Bind the socket
  if(bind(sock_desc, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
    perror("Error binding server socket");
    exit(1);
  }
  printf("Socket Binding to port %d successful\n", 8888);

  // 5 - Listen for client connections
  if(listen(sock_desc, 10) == 0){
    printf("Waiting for client connections...\n");
  }else{
    printf("[-]Error in binding.\n");
  }

  //infinite loop to continuously listen for connections
  while(1){
    // 6 - Accept a clients connection
    client_sock = accept(sock_desc, (struct sockaddr*)&client_addr, &addr_size);
    if(client_sock < 0){
      perror("Closing client connection...");
      exit(1);
    }
    // print client details using inet_ntoa() and inet_ntohs() - converts from network byte order to string
    printf("Connection accepted from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

    //create a new process for the client using fork()
    if((childpid = fork()) == 0){
      close(sock_desc);

      //infinite loop to deal with client
      //for testing simply prints the clients message
      //exits when "exit" specified 
      while(1){

          printf("authenticaing user\n");
          //read in the username
          memset(buffer, 0, BUFFSIZE);
          recv(client_sock, buffer, BUFFSIZE, 0);
          login = &buffer[0];
          printf("login: %s\n", login);

          //AUTH USER
          authResult = authenticateUser(login);
          
          //authenticate the user
          if( authResult == 0){
            printf("Unsuccessful login\n");
            send(client_sock, "unsuccessful", strlen("unsuccessful"), 0);
            exit(1);
          }else{
            send(client_sock, "successful", strlen("successful"), 0);
            //proceed with the file transfer
            recieveClientFile(client_sock);
            printf("File transfered successfully\n");
            exit(1);
          }

      }//end while

    }//end fork()

  }//end listening while

  close(client_sock);


  return 0;
}//end server

//function to handle recieving files from client
void recieveClientFile(int client_sock){
  
  char file_buffer[512];
  char* file_path = "";

  //read in the destination
  memset(buffer, 0, BUFFSIZE);
  if(recv(client_sock, buffer, BUFFSIZE, 0) < 0){
    //destination read error
    printf("destination of file cannot be read\n");
  }
  printf("file destination: %s\n", buffer);

  file_path = buffer;

  FILE *file_open = fopen(file_path, "w");
  if(file_open == NULL){
    printf("File %s cannot be opened on server\n", file_path);
  }
  else{
    
    bzero(file_buffer, 512);
    int block_size = 0, i=0;

    while((block_size = recv(client_sock, file_buffer, 512, 0)) > 0){

      printf("Data Received %d = %d\n", i, block_size);
      int write_sz = fwrite(file_buffer, sizeof(char), block_size, file_open);
      bzero(file_buffer, 512);
      i++;
    }
  }
  printf("Transfer completed successfully\n");
  fclose(file_open);
}

//function that authenitcates a username and password
int authenticateUser(char *login){

  char del[2] = ",";
  char *userDetails;

  char line[128];
  FILE *file = fopen("users.txt", "r");

  if(file){
    while(fgets(line, sizeof(line), file)){
      /* get the first token */
      userDetails = strtok(line, del);
      if(strcmp(userDetails, login) == 0){
        return 1;
      }
    }
    fclose(file);
    return 0;
  }
}