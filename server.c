#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>

//used to deal with threading
void *connection_handler(void *);

int main(int argc, char *argv[]){
	
	//initialise all the required variables
	int socket_desc, client_socket, client, read_size, *new_socket;
	//structure to hold the server and client address
	struct sockaddr_in server_addr, client_addr;
	//store the client message --> testing purposes
	char client_msg[2000];

	//STEP 1 - create the socket
	socket_desc = socket(AF_INET, SOCK_STREAM, 0);
	//error checking
	if(socket_desc == -1){
		printf("Error creating socket\n");
	}
	puts("Socket Successfully created!\n");
	
	//STEP 2 - initialize the servers address structure
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(8888);

	//STEP - 3 Bind the socket
	if(bind(socket_desc, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0){
		perror("Socket binding failed!\n");
		return 1;
	}
	puts("Socket Binding successful!\n");

	//STEP - 4 Init listing state
	//backlog of up to five clients that can connect
	listen(socket_desc, 5);

	//STEP - 5 Accept incoming connections from clients
	puts("Awaiting client connections...\n");
	client = sizeof(struct sockaddr_in);

	//infinite while loop to accept connections from client
	while( (client_socket = accept(socket_desc, (struct sockaddr *)&client_socket, (socklen_t*)&client))){
		//A: create the thread for the new client
		puts("Client Connection Accepted\n");
		pthread_t c_thread;
		new_socket = malloc(1);
		*new_socket = client_socket;

		if(pthread_create(&c_thread, NULL, connection_handler, (void*)new_socket) < 0){
			perror("Thread creation error\n");
			return 1;
		}
		puts("Connection Handler Assigned to Thread\n");
	}

	//error checking for new client socket
	if(client_socket < 0){
		perror("Could not accept client\n");
		return 1;
	}

	return 0;
}//end main

//Connection handler for each of the clients
void *connection_handler(void *socket_desc){
	//get the descriptor of the socket param
	int socket = *(int*)socket_desc;
	int read_size;
	char *msg, client_msg[2000];

	//send a message to the connected client
	msg = "This is a test\n";
	write(socket, msg, strlen(msg));

	//read in a message from the client
	while((read_size = recv(socket, client_msg, 2000, 0)) > 0){
		write(socket, client_msg, strlen(client_msg));
	}

	if(read_size == 0){
		puts("Client has disconnected\n");
		fflush(stdout);
	}else if(read_size == -1){
		perror("Could not read client message\n");
	}

	//free the pointer for the socket
	free(socket_desc);
	return 0;
}
