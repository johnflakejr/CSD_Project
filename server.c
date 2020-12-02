/*
 * CPT John Lake
 * CSD Board Project
 * Last Updated: 30NOV2020
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
#include <signal.h> 
#include <sys/types.h>
#include <sys/socket.h>


/*
 *	If the client drops the connection, we want to handle it gracefully: 
 */
void broken_pipe_handler(){
	fprintf(stderr,"Error: broken pipe.\n"); 
	return; 
}

/*
 * Download files from this server to the client.
 */
void download(int client_socket, char *full_file_path, char* filename){

	//Setup buffers
	char file_buffer[4096];
	char response_buffer[4096];
	memset(file_buffer,0,4096); 
	memset(response_buffer,0,4096); 


	//Check if file exists: 
	if(access(full_file_path,F_OK) == -1){
		fprintf(stderr,"Error: client requested a file that was not found.  File: %s\n",full_file_path);
		send_error(client_socket,"FILE_NOT_FOUND\n"); 
		return;
	}

	//File exists. Open and check size: 
	FILE * file_pointer = fopen(full_file_path,"rb"); 
	if(!file_pointer){
		fprintf(stderr,"Error: client requested a file that could not be opened. Check permissions.\n"); 
		send_error(client_socket,"Error reading file. Closing.\n\n"); 	
		return; 
	}
	fseek(file_pointer,0,SEEK_END); 
	int size = ftell(file_pointer); 
	fseek(file_pointer,0,SEEK_SET); 

	printf("Client requested %s, which is %d bytes.\n",full_file_path,size); 

	//Send SENDING message to client with number of bytes: 
	sprintf(response_buffer,"SENDING %d\n",size); 
	if(send(client_socket,response_buffer,strlen(response_buffer),0) < 0){
		fprintf(stderr,"Error: there was an issue sending \"SENDING\" to the client.\n"); 
		fclose(file_pointer); 
		return; 
	}
		

	//Let client parse for a second: 
	sleep(1); 

	//Read from file and send to client
	ssize_t bytes_read = fread(file_buffer,1,4096,file_pointer); 
	if(bytes_read < 0){
		fprintf(stderr,"Error: there was an issue reading the file.\n"); 
		fclose(file_pointer); 
		return;
	}
	while(bytes_read > 0){
		if(send(client_socket,file_buffer,bytes_read,0) < 0){
			fprintf(stderr,"Error: there was an issue sending the file to the client.\n"); 
			fclose(file_pointer); 
			return; 
		}
		bytes_read = fread(file_buffer,1,4096,file_pointer); 
	}
	fclose(file_pointer); 
}

/*
 * Upload files from the client to this server.
 */
void upload(int client_socket,char * full_file_path, char* filename){
	
	//Setup buffers
	char file_buffer[4096];
	char response_buffer[4096];
	memset(file_buffer,0,4096); 
	memset(response_buffer,0,4096); 

	//Write to file.  Use binary mode.  
	FILE * file_pointer = fopen(full_file_path,"wb"); 
	if(!file_pointer){
		fprintf(stderr, "Error: could not open file for writing.\n"); 
		send_error(client_socket,"Error writing file. Closing.\n\n"); 	
		return; 
	}
	
	//Generate READY message with filename and send to client: 
	sprintf(response_buffer,"READY %s\n",filename); 
	if(send(client_socket,response_buffer,strlen(response_buffer),0) < 0){
		fprintf(stderr,"Error: there was an issue sending \"READY\" to the client.\n"); 
		fclose(file_pointer); 
		return;
	}


	//Get file from client
	ssize_t total_b = 0;
	ssize_t bytes_received = recv(client_socket,file_buffer,sizeof(file_buffer),0); 
	if(bytes_received < 0){
		fprintf(stderr,"Error: there was an issue reading data from the client.\n"); 
		fclose(file_pointer); 
		return; 
	}
	while(bytes_received){
		total_b += bytes_received; 
		fwrite(file_buffer,1,bytes_received,file_pointer);
		bytes_received = recv(client_socket,file_buffer,sizeof(file_buffer),0); 
	}

	printf("Client uploaded the file %s, which is %d bytes.\n",full_file_path, (int) total_b); 

	fclose(file_pointer);
}


/*
 * This is where the magic happens
 */
int main(int argc, char ** args){

	//Check arg count
	if(argc != 3)
		usage(); 

	//Ensure port is valid
	int port = atoi(args[1]);
	if(port < 0 || port > 65535)
		usage(); 

	//Check if directory is valid: 
	char * working_dir = args[2]; 
	DIR* dir = opendir(working_dir);
	if (dir) {
		closedir(dir);
	} else {
		fprintf(stderr,"Error: directory not found or is invalid.\n"); 
		usage(); 
	}

	//Setup socket info for server
	int server_socket; 
	struct sockaddr_in server_socket_info; 
	socklen_t socket_info_size = sizeof(struct sockaddr_in); 
	memset(&server_socket_info,0,sizeof(server_socket_info));
	server_socket_info.sin_family=AF_INET;
	server_socket_info.sin_addr.s_addr=htonl(INADDR_ANY);
	server_socket_info.sin_port = htons(port);

	//Setup handler for SIGPIPEs
	signal(SIGPIPE,broken_pipe_handler); 

	//make, bind, and listen on server socket
	server_socket = socket(AF_INET,SOCK_STREAM,0);
	if(server_socket == -1){
		fprintf(stderr,"Error creating socket: %d\n",errno);
		exit(errno); 
	}
	if(bind(server_socket,(struct sockaddr*) &server_socket_info,sizeof(struct sockaddr)) == -1){
		fprintf(stderr,"Error binding socket: %d\n",errno);
		exit(errno); 
	}
	if(listen(server_socket,1) ==-1){
		fprintf(stderr,"Error listening on socket: %d\n",errno);
		exit(errno); 
	}

	//Declare buffers / pointers for dynamically allocated memory
	char command_buffer[4096];
	char * full_file_path; 
	char ** parsed_command;

	while(1){

		//Accept new connection from client: 
		struct sockaddr_in client_socket_info; 
		int client_socket = accept(server_socket,(struct sockaddr*) &client_socket_info,&socket_info_size);
		memset(command_buffer,0,4096); 

		if(client_socket < 0){
			fprintf(stderr,"Error accepting socket: %d\n",errno);
			exit(errno); 
		}

		if(DEBUG){
			printf("Client IP/Port:  %s:%d\n",inet_ntoa(client_socket_info.sin_addr),ntohs(client_socket_info.sin_port));
		}


		//Receive command message from the client: 
		ssize_t bytes_received = recv(client_socket,command_buffer,4095,0);
		command_buffer[4095] = '\0'; 

		if(bytes_received < 0){
			fprintf(stderr,"Error receiving data:%d\n",errno); 
			exit(errno); 
		}

		//tokenize the message: 
		parsed_command = parse_command(command_buffer,bytes_received); 
	
		//Command message error: 
		if(parsed_command == NULL){
			fprintf(stderr,"Error: There was an issue with the command sent by the client.  Terminating connection.\n"); 
			send_error(client_socket,"Unrecognized command.  Please use UPLOAD [filename] or DOWNLOAD [filename].\n\n"); 
		}else{
			//Add working directory to the filepath 
			full_file_path = obtain_full_file_path(parsed_command[1],working_dir); 

			//Upload file: 
			if(strcmp(parsed_command[0],"UPLOAD") == 0){
				printf("Upload request from %s:%d\n",inet_ntoa(client_socket_info.sin_addr),ntohs(client_socket_info.sin_port));
				upload(client_socket,full_file_path,parsed_command[1]); 
			//Download file: 
			}else if (strcmp(parsed_command[0],"DOWNLOAD") == 0){
				printf("Download request from %s:%d\n",inet_ntoa(client_socket_info.sin_addr),ntohs(client_socket_info.sin_port));
				download(client_socket,full_file_path,parsed_command[1]);
			//Neither UPLOAD nor DOWNLOAD:
			}else { 
				fprintf(stderr,"Error: Unrecognized command from the client.\n"); 
				send_error(client_socket,"Unrecognized command.  Please use UPLOAD [filename] or DOWNLOAD [filename].\n\n"); 
			}
			free(full_file_path); 
		}

		free(parsed_command); 
		close(client_socket);
	}
	close(server_socket);
	return 0; 
}
