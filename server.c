/*
 * CPT John Lake
 * CSD Board Project
 */
#include "util.h"
#include <stdio.h>
#include <time.h> 
#include <errno.h>
#include <stdlib.h>
#include <dirent.h> 
#include <arpa/inet.h>
#include <string.h> 
#include <unistd.h> 
#include <sys/types.h>
#include <sys/socket.h>
#define DEBUG 1

#define UPLOAD 1
#define DOWNLOAD 2
#define ERROR 3



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
	if(port < 0 || port > 65535)
		usage(); 


	char * working_dir = args[2]; 
	//Check if directory is valid: 
	DIR* dir = opendir(working_dir);
	if (dir) {
		    closedir(dir);
	} else {
		printf("Error: directory not found or is invalid.\n"); 
		usage(); 
	}

	printf("You are listening on port %d\n",port);
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

	char command_buffer[4096];
	char response_buffer[4096];
	char file_buffer[4096];
	char * full_file_path; 

	while(1){
		//Accept new connection from client: 
		int client_socket = accept(server_socket,(struct sockaddr*) &client_socket_info,&socket_info_size);
		memset(command_buffer,0,4096); 
		memset(response_buffer,0,4096); 
		memset(file_buffer,0,4096); 

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
		ssize_t bytes_received = recv(client_socket,command_buffer,4095,0);
		command_buffer[4095] = '\0'; 

		if(bytes_received < 0){
			printf("Recv error: %d\n",errno); 
			exit(1); 
		}

		//tokenize the message: 
		int command; 
		char ** parsed_command = parse_command(command_buffer,bytes_received); 
		if(parsed_command == NULL){
			send_error(client_socket,"Unrecognized command.  Please use UPLOAD [filename] or DOWNLOAD [filename].\n\n"); 
			continue; 
		}


		//Determine if the client is uploading or downloading a file: 
		if(strcmp(parsed_command[0],"UPLOAD") == 0){
			command = UPLOAD; 
		} else if (strcmp(parsed_command[0],"DOWNLOAD") == 0){
			command = DOWNLOAD; 
		} else {
			send_error(client_socket,"Unrecognized command.  Please use UPLOAD [filename] or DOWNLOAD [filename].\n\n"); 
			continue;
		}
	
		if(DEBUG){
			if(command == UPLOAD)
				printf("Uploading file\n"); 
			else if (command == DOWNLOAD)
				printf("Downloading file\n"); 
			printf("After parsing:\n");
			printf("Command used: %s, %d bytes\n",parsed_command[0],(int) strlen(parsed_command[0])); 
			printf("Filename used: %s, %d bytes\n",parsed_command[1],(int) strlen(parsed_command[1])); 
		}

		full_file_path = obtain_full_file_path(parsed_command[1],working_dir); 
		printf("FFP: %s\n",full_file_path); 

		//Upload file: 
		if(command == UPLOAD){
				
			//Write to file: 
			FILE * file_pointer = fopen(full_file_path,"wb"); 
			if(!file_pointer){
				send_error(client_socket,"Error writing file. Closing.\n\n"); 	
				continue; 
			}
			sprintf(response_buffer,"READY %s\n",parsed_command[1]); 
			send(client_socket,response_buffer,strlen(response_buffer),0); 
				
			ssize_t total_b; 
			bytes_received = recv(client_socket,file_buffer,sizeof(file_buffer),0); 
			while(bytes_received){
				total_b += bytes_received; 
				fwrite(file_buffer,1,bytes_received,file_pointer);
				bytes_received = recv(client_socket,file_buffer,sizeof(file_buffer),0); 
			}
			if(DEBUG)
				printf("Wrote %d bytes to %s\n",(int) total_b,parsed_command[1]); 
			fclose(file_pointer); 
		} else if (command == DOWNLOAD){

			//Check if file exists: 
			if(access(full_file_path,F_OK) == -1){
				send_error(client_socket,"FILE_NOT_FOUND\n"); 
				continue; 
			}

			//File exists.  Check size: 
			FILE * file_pointer = fopen(full_file_path,"rb"); 
			if(!file_pointer){
				send_error(client_socket,"Error reading file. Closing.\n\n"); 	
				continue; 
			}

			//Determine file size: 
			fseek(file_pointer,0,SEEK_END); 
			int size = ftell(file_pointer); 
			sprintf(response_buffer,"SENDING %d\n",size); 
			send(client_socket,response_buffer,strlen(response_buffer),0); 
			fseek(file_pointer,0,SEEK_SET); 
			sleep(1); 
			ssize_t total_b;
			ssize_t bytes_read = fread(file_buffer,1,4096,file_pointer); 
			while(bytes_read > 0){
				total_b += bytes_read; 
				send(client_socket,file_buffer,bytes_read,0); 
				bytes_read = fread(file_buffer,1,4096,file_pointer); 
			}
			fclose(file_pointer); 
		}
		close(client_socket);
	}
	close(server_socket);
	return 0; 
}
