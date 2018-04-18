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

	//initialise client/server variables
	int client_sock;
	struct sockaddr_in server_addr;
	char buffer[500];

	//create the client socket
	client_sock = socket(AF_INET, SOCK_STREAM, 0);
	if(client_sock < 0){
		printf("Error creating client socket\n");
		exit(1);
	}
	printf("Client socket successfully created\n");

	//initialise server address variables
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT);
	server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

	// connect to the server
	if(connect(client_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
		perror("Error connecting to server");
		exit(1);
	}
	printf("Successfully Connected to Server\n");

	//infinite while loop to continuously communicate with server
	while(1){

		//send data to server
		printf("Client: ");
		scanf("%s", &buffer[0]);
		send(client_sock, buffer, strlen(buffer), 0);

		if(strcmp(buffer, "exit") == 0){
			close(client_sock);
			printf("Disconnected from server\n");
			exit(1);
		}

		//read data from server
		if(recv(client_sock, buffer, 1024, 0) < 0){
			perror("Error reading server data");
		}else{
			printf("Server: %s\n", buffer);
		}
	}

	return 0;
}