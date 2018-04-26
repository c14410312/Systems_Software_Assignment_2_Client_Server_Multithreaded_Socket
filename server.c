#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#define BUFFSIZE 1024
#define PORT 8888

//declare global variables
int sock_desc, ret;
struct sockaddr_in server_addr;
int client_sock;
struct sockaddr_in client_addr;
socklen_t addr_size;
char buffer[BUFFSIZE];
pid_t childpid;

void recieveClientFile(int client_sock);

int main(){

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
        memset(buffer, 0, BUFFSIZE);
        recv(client_sock, buffer, BUFFSIZE, 0);
        
        if(strcmp(buffer, "exit") == 0){
          printf("%s\n", buffer);
          printf("Disconnected from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
          break;
        } 
        else if(strcmp(buffer, "password") == 0){
          //password auth was successful, recieve data from client
          printf("Client: %s\n", buffer);
          write(client_sock, "Password Accepted", strlen("Password Accepted"));
          memset(buffer, 0, BUFFSIZE);
          recieveClientFile(client_sock);
        }else{
          printf("%s\n", buffer);
          write(client_sock, "incorrect password", strlen("incorrect password"));
          memset(buffer, 0, BUFFSIZE);
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
  char* file_name = "serverFiles/index.html";

  FILE *file_open = fopen(file_name, "w");
  if(file_open == NULL){
    printf("File %s cannot be opened on server\n", file_name);
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