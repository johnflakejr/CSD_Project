#include "util.h"

/*
 *	If the client drops the connection, we want to handle it gracefully: 
 */
void broken_pipe_handler(){
	fprintf(stderr,"Error: broken pipe.\n"); 
	return; 
}

/*
 *	Given a path and filename, return just the filename: 
 *	Ex: /etc/passwd returns passwd
 */
void get_raw_filename(char* raw_filename,char * ffp){
	strncpy(raw_filename,ffp,strlen(ffp)+1); 
	raw_filename[strlen(ffp)] = '\0'; 
	if(DEBUG){
		printf("Full filepath: %s\n",ffp); 
		printf("String length of filepath: %d\n",(int)strlen(ffp)); 
	}

	//Find out what the last "/" character's location is: 
	int last_position = -1; 
	int i; 
	for(i = 0; i < strlen(raw_filename); i++){
		if(raw_filename[i] == '/')
			last_position = i; 
	}

	//Next, shift bytes over:
	for(i = 0; i < strlen(raw_filename) - last_position; i++)
		raw_filename[i] = raw_filename[i+last_position+1]; 
	if(DEBUG)
		printf("Returned filepath: %s\n",raw_filename); 
}

/*
 * Usage message when command line arguments or commands are improperly used: 
 */
void usage(void){
	printf("Usage: ./server PORT DIRECTORY\n");
	exit(1);
}

/*
 * Microscopic helper function to send a custom error message to the client
 */
void send_error(int socket, char* message){ 
	if(send(socket,message,strlen(message),0) < 0){
		fprintf(stderr,"Error sending message to client.\n"); 
	}
}

/*
 * Combine working dir and filename to obtain full file path (absolute or relative): 
 */
char * obtain_full_file_path(char * filename,char * working_dir){

	if(DEBUG){
		printf("Combining working directory: %s with filename %s\n",working_dir,filename); 
	}

	//Allocate enough memory for the filename plus the working directory
	//+2 is to account for the extra '/' and null terminator, because strlen() doesn't include '\0'.  
	char * ffp = malloc(strlen(filename) + strlen(working_dir) + 2);  

	//Add "/" to the end of the directory if it isn't there. 
	int dir_len = strlen(working_dir); 
	if(working_dir[dir_len-1] != '/'){
		sprintf(ffp,"%s/%s",working_dir,filename); 
	}else{
		sprintf(ffp,"%s%s",working_dir,filename); 
	}
	if(DEBUG)
		printf("Full filepath: %s\n",ffp); 

	return ffp; 
}	


/*
 * Get rid of trailing and leading whitespace: 
 * Modify the string directly
 */
void trim_whitespace(char * response,char * input){

	//Allocate enough space for the entire input string
	strncpy(response,input,strlen(input)+1); 

	
	if(DEBUG)
		printf("Trimming leading whitespace from [%s].\n",response); 

	//Trim leading whitespace and forward slashes (paths): 
	int offset=0, i = 0; 
	while(response[offset] == ' ' || response[offset] == '\t' || response[offset] == '\n'){
		offset++; 
	}

	if(DEBUG)
		printf("Moving bytes to left.  Found offset: %d\n",offset); 

	//Start at the offset found above (first spot with no whitespace), and move the characters over (incl trailing whitespace):  
	i = 0; 
	while(response[offset+i] != '\0'){
		response[i] = response[offset + i]; 	
		i++; 
	}
	response[i] = '\0'; 


	if(DEBUG)
		printf("Finding offset of last non-whitespace char.\n"); 

	//Determine last non-whitespace character: 
	int index = -1; 
	i = 0; 
	while(response[i] != '\0'){
		if(response[i] != ' ' && response[i] != '\t' && response[i] != '\n')
			index = i; 
		i++; 
	}
	//Null terminate right after the last non-whitespace character, effectively removing trailing whitespace: 
	response[index+1] = '\0';  
}

