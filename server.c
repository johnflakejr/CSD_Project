/*
 * CPT John Lake
 * CSD Board Project
 */
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h> 
#include <unistd.h> 
#include <sys/types.h>
#include <sys/socket.h>
#define DEBUG 1


/*
 * Usage message when command line arguments or commands are improperly used: 
 */
void usage(void){
	printf("Usage: ./NAME PORT DIRECTORY\n");
	exit(1);
}


char ** parse_command(char * buffer, ssize_t message_length){
	char *request_type = strtok(buffer," "); 
	char *filename = strtok(NULL," "); 
	if(DEBUG){
		printf("Command used: %s, %d bytes\n",request_type,(int) strlen(request_type)); 
		printf("File Name: %s\n",filename); 

	}
	//TODO: normalize filenames (i.e: change ./test to test)
	//TODO: evaluate uppercase and lowercase commands
	
	
	return NULL; 
}

/*
	Command line arguments to be used IAW proj doc. 
*/
int main(int argc, char ** args){


	/*
	 * Check Arguments
	*/
	if(argc != 3)
		usage(); 

	int port = atoi(args[1]);
	printf("You are listening on port %d\n",port);

	char * working_dir = args[2]; 
	printf("Current working directory is: %s\n",working_dir);


	struct sockaddr_in client_socket_info; 
	struct sockaddr_in server_socket_info; 
	socklen_t socket_info_size = sizeof(struct sockaddr_in); 
	int server_socket; 

	//Setup socket info for server
	memset(&server_socket_info,0,sizeof(server_socket_info));
	server_socket_info.sin_family=AF_INET;
	server_socket_info.sin_addr.s_addr=htonl(INADDR_ANY);
	server_socket_info.sin_port = htons(port);

	//make, bind, and listen on server socket
	server_socket = socket(AF_INET,SOCK_STREAM,0);
	bind(server_socket,(struct sockaddr*) &server_socket_info,sizeof(struct sockaddr));

	//One connection max: 
	listen(server_socket,1);

	char buffer[4096];

	while(1){
		//Accept new connection from client: 
		int client_socket = accept(server_socket,(struct sockaddr*) &client_socket_info,&socket_info_size);
		memset(buffer,0,4096); 

		if(client_socket < 0){
			printf("Accept socket error:%d\n",errno);
			exit(1); 
		}

		if(DEBUG){
			printf("Socket number: %d\n",client_socket);
			printf("Received connection.\n");
			printf("Client IP/Port:  %s:%d\n",inet_ntoa(client_socket_info.sin_addr),ntohs(client_socket_info.sin_port));
		}


		//Receive command message from the client: 
		ssize_t bytes_received = recv(client_socket,buffer,4095,0);
		buffer[4095] = '\0'; 

		if(bytes_received < 0){
			printf("Recv error: %d\n",errno); 
			exit(1); 
		}
	
		if(DEBUG){
			printf("Received %d bytes.\n",(int) bytes_received);
			printf("Message: %s\n",buffer);
		}	

		//tokenize the message: 
		parse_command(buffer,bytes_received); 

		//Determine if the client is uploading or downloading a file: 



		//Determine what COA to take: 
		send(client_socket,buffer,bytes_received,0);
		close(client_socket);
	}

	close(server_socket);
	return 0; 
}
