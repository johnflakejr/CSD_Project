#ifndef UTIL_H
#define UTIL_H
#include <stdio.h>
#include <time.h> 
#include <errno.h>
#include <signal.h> 
#include <stdlib.h>
#include <dirent.h> 
#include <arpa/inet.h>
#include <string.h> 
#include <unistd.h> 
#include <sys/types.h>
#include <sys/socket.h>
#define DEBUG 0


//Handle dropped connections gracefully (SIGPIPE): 
void broken_pipe_handler(); 

//Get filename from path (e.g: /etc/passwd becomes passwd): 
void get_raw_filename(char*raw_filename,char*ffp); 

//Print usage information
void usage(void);

//Small wrapper for sending error information to client. 
void send_error(int socket, char* message); 

//Given a directory and file, combine to a full filepath: 
char* obtain_full_file_path(char* filename, char* working_dir); 

//Get rid of leading and trailing whitespace: 
void trim_whitespace(char*response, char*input); 

#endif
