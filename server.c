#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8888

int main(){

  // 1 - Initialse the servers variables
  int sock_desc, ret;
  struct sockaddr_in server_addr;
  int client_sock;
  struct sockaddr_in client_addr;
  socklen_t addr_size;
  char buffer[500];
  pid_t childpid;

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
        recv(client_sock, buffer, 500, 0);
        if(strcmp(buffer, "exit") == 0){
          printf("Disconnected from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
          break;
        }else{
          printf("Client: %s\n", buffer);
          send(client_sock, buffer, strlen(buffer), 0);
          bzero(buffer, sizeof(buffer));
        }
      }
    }

  }

  close(client_sock);


  return 0;
}