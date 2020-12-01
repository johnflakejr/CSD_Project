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
/*
 * Usage message when command line arguments or commands are improperly used: 
 */
void usage(void){
	printf("Usage: ./NAME PORT DIRECTORY\n");
	exit(1);
}


/*
 * Helper function to send a custom error message to the client and close the connection. 
 *
 */
void send_error(int socket, char* message){ 
	send(socket,message,strlen(message),0); 
	close(socket); 
	printf("Sent error message.\n"); 
}

/*
 * Combine working dir and filename to obtain full file path (absolute or relative): 
 */
char * obtain_full_file_path(char * filename,char * working_dir){
	char * ffp = malloc(strlen(filename) + strlen(working_dir));  
	int dir_len = strlen(working_dir); 
	if(working_dir[dir_len-1] != '/'){
		sprintf(ffp,"%s/%s",working_dir,filename); 
	}else{
		sprintf(ffp,"%s%s",working_dir,filename); 
	}

	printf("Full filepath: %s\n",ffp); 
	return ffp; 
}	

char * trim_whitespace(char * input){
	int offset=0, i = 0; 

	//Trim leading whitespace and forward slashes (paths): 
	while(input[offset] == ' ' || input[offset] == '\t' || input[offset] == '\n'){
		offset++; 
	}
	i = 0; 

	//Start at the offset found above, and move the characters over (incl trailing whitespace):  
	while(input[offset+i] != '\0'){
		input[i] = input[offset + i]; 	
		i++; 
	}
	input[i] = '\0'; 

	//Determine last non-whitespace character: 
	int index = -1; 
	i = 0; 
	while(input[i] != '\0'){
		if(input[i] != ' ' && input[i] != '\t' && input[i] != '\n')
			index = i; 
		i++; 
	}
	input[index+1] = '\0';  
	return input; 
}



/**
 * Given a buffer, extract the command and filename and return them as an array of two strings. 
 */
char ** parse_command(char * buffer, ssize_t message_length){
	char *request_type = strtok(buffer," "); 
	char *filename = strtok(NULL,"\n"); 

	if(request_type == NULL || filename == NULL)
		return NULL; 
	if(DEBUG){
		printf("Command used: %s, %d bytes\n",request_type,(int) strlen(request_type)); 
		printf("Filename used: %s, %d bytes\n",filename,(int) strlen(filename)); 

	}
	
	char ** parsed_command = (char**) malloc(2 * sizeof(char*)); 
	parsed_command[0] = request_type; 
	parsed_command[1] = trim_whitespace(filename);
	return parsed_command; 
}
