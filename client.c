#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#define PORT 8000
#define BUFFSIZE 1024

//global variables - client/server
int client_sock;
struct sockaddr_in server_addr;
char buffer[BUFFSIZE];
pthread_mutex_t file_lock;

void fileTransfer(char* fpath, char* dpath);

int main(int argc, char *argv[]){
	char *authDetails = argv[1];//read in the user auth info
	char *filePath = argv[2];//read in the file name
	char *destPath = argv[3]; //read in the destination on the server
	pthread_mutex_init(&file_lock, NULL); //init the lock 

	//read in file from command line
	if(argc == 1){
		printf("Error, file name required as an argument...\n");
		exit(1);
	}
	else if(argc > 4){
		printf("Error, 1 argument required, detected 2...\n");
		exit(1);
	}
	else{
		printf("%s\n", authDetails);
		printf("%s\n", filePath);
		printf("%s\n", destPath);
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
		printf("Authenticating User ...\n");
		send(client_sock, authDetails, strlen(authDetails), 0);

		memset(buffer, 0, BUFFSIZE);
		if(recv(client_sock, buffer, BUFFSIZE, 0) < 0){
			perror("Error reading server data");
		}else{
			//if the password is correct, authenticate
			if(strcmp(buffer, "successful") == 0){
				//run the file transer
				fileTransfer(filePath, destPath);
				printf("file %s transfered to server\n", filePath);
				exit(1);
			}
			else{
				printf("Incorrect login. Try Again.\n");
				exit(1);
			}
		}
		//printf("Authentication successful\n");
		//run the file transer
		//fileTransfer(filePath, destPath);
		//printf("file %s transfered to server\n", filePath);
		//exit(1);
	}

	return 0;
}

//function to deal with transfering the file
void fileTransfer(char* fpath, char* dpath){

	char* filePath = fpath;
	char* destPath = dpath;
	char fileBuffer[512];

	printf("Sending %s to the server", filePath);
	//first send the file path
	if(send(client_sock, destPath, strlen(destPath), 0) < 0){
		printf("Could not send destination path to server\n");
	}

	//lock the process of transfering the file
	pthread_mutex_lock(&file_lock);
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
	//unlock the write file process
	pthread_mutex_unlock(&file_lock);
}