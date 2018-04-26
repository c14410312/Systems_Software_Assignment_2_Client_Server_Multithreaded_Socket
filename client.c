#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8888
#define BUFFSIZE 1024

//global variables - client/server
int client_sock;
struct sockaddr_in server_addr;
char buffer[BUFFSIZE];

void fileTransfer(char* fpath);

int main(int argc, char *argv[]){
	int isAuth = 0;
	char *filePath = argv[1];//read in the file name

	//read in file from command line
	if(argc == 1){
		printf("Error, file name required as an argument...\n");
		exit(1);
	}
	else if(argc > 2){
		printf("Error, 1 argument required, detected 2...\n");
		exit(1);
	}
	else{
		printf("%s\n", filePath);
	}
	

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
		//request the username from the user
		//send data to server
		
		//reset the buffer
		memset(buffer, 0, BUFFSIZE);
		
		//1-create an authentication function
		printf("Enter Password: ");
		scanf("%s", &buffer[0]);
		send(client_sock, buffer, strlen(buffer), 0);

		if(strcmp(buffer, "exit") == 0){
			close(client_sock);
			printf("Disconnected from server\n");
			exit(1);
		}
		//read data from server
		if(recv(client_sock, buffer, BUFFSIZE, 0) < 0){
			perror("Error reading server data");
		}else{
			//if the password is correct, authenticate
			if(strcmp(buffer, "Password Accepted") == 0){
				//run the file transer
				fileTransfer(filePath);
				printf("file %s transfered to server\n", filePath);
				exit(1);
			}
			else{
				printf("Incorrect Password. Try Again.\n");
			}
		}
	}

	return 0;
}

//function to deal with transfering the file
void fileTransfer(char* fpath){

	char* filePath = fpath;
	char fileBuffer[512];
	
	printf("Sending %s to the server", filePath);
	FILE *file_open = fopen(filePath, "r");
	bzero(fileBuffer, 512);
	int blockSize,  i=0;

	while((blockSize = fread(fileBuffer, sizeof(char), 512, file_open)) > 0){
		printf("data sent %d = %d\n", i, blockSize);
		if(send(client_sock, fileBuffer, blockSize, 0) < 0){
			perror("Error sending file to server");
			exit(1);
		}
		bzero(fileBuffer, 512);
		i++;
	}
}