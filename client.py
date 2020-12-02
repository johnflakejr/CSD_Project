'''
CPT John Lake
CSD Board Project - Client
Last updated 2DEC2020
'''

import argparse;
import ipaddress;
import socket;
import os; 
import sys;
import time; 

def upload(sock,filename):

    #Server doesn't care about the directory we get the file from on the client. Strip it if it's provided. 
    net_filename = os.path.basename(filename); 
    msg = "UPLOAD " + net_filename; 

    #Send the UPLOAD message: 
    sock.send(msg.encode())

    #If the server doesn't reply with READY, then abort. 
    data = sock.recv(1024).decode();
    expected = "READY " + net_filename; 
    if(expected not in data): 
        print("Error uploading.  The server did not reply with the expected message or filename.\n",file=sys.stderr); 
        return; 

    print("Uploading \"" + filename + "\" to the server.\n");

    #Open file, read bytes, and send them. 
    f = None; 
    try: 
        f = open(filename,"rb"); 
    except: 
        print("Error opening file.  Check permissions.",file=sys.stderr); 
        return; 

    read_bytes = f.read(4096); 
    total_b = 0; 
    while(read_bytes):
        total_b = total_b + len(list(read_bytes)); 
        sock.send(read_bytes); 
        read_bytes = f.read(4096); 
    f.close(); 

    print("Uploaded " + str(total_b) + " bytes to the server.\n"); 

def download(sock,filename): 

    #Send the DOWNLOAD message: 
    msg = "DOWNLOAD " + filename
    sock.send(msg.encode())

    #If the server doesn't reply with READY, then abort. 
    data = sock.recv(1024).decode();
    if("FILE_NOT_FOUND" in data): 
        print("Error: File not found on server.\n",file=sys.stderr); 
        return; 
    if("SENDING " not in data):  
        print("Error: could not download - the server sent a message that was not understood.\n",file=sys.stderr); 
        return; 

    size = data.split(" ")[1]; 
    if(size == "" or size == '\n'):
        print("Error: size not received.\n",file=sys.stderr); 
        return; 
    print("Downloading \"" + filename + "\" with size " + str(size));


    #Open file, read bytes from server,and save them
    f = open(filename,"wb"); 
    data = sock.recv(4096); 
    total_b = 0
    while(data): 
        total_b = total_b + len(list(data))
        if(total_b > int(size)):
            f.close(); 
            os.remove(filename); 
            print("Error: server sent more data than expected.  Cancelling. \n"); 
            return; 
        f.write(data); 
        data = sock.recv(4096); 

    #Error out if the server doesn't send the expected size: 
    if(total_b < int(size)): 
        f.close(); 
        os.remove(filename); 
        print("Error: server sent less data than expected.  Cancelling. \n"); 
        return; 

    print("Wrote " + str(total_b) + " bytes to file.\n"); 
    f.close(); 



def validate_input(ip,port,command,filename): 
    #valid port? 
    if(port <0 or port > 65535):
        print("Error: Invalid port.\n",file=sys.stderr);
        quit();

    #valid IP?
    try: 
        ipaddress.ip_address(ip);
    except:  
        print("Error: Invalid IP.\n",file=sys.stderr);
        quit(); 

    #valid command?
    if(command != "U" and command != "D"):
        print("Error: Invalid command.  U/D are the only valid commands.",file=sys.stderr); 
        quit(); 

    #if uploading, check if file exists. 
    if(command == "U"): 
        if(not os.path.isfile(filename)):
            print("Error: File doesn't exist.",file=sys.stderr); 
            quit(); 

def main():
        #Setup arg parsing: 
    parser = argparse.ArgumentParser(description="Client for File Uploads.  Only compatible with Python 3");
    parser.add_argument("IP",type=str,default="127.0.0.1",help="IP address for file server");
    parser.add_argument("PORT",type=int,default=5000,help="Port the server is listening on");
    parser.add_argument("COMMAND",type=str,default="D",help="Command to use (Upload: U / Download: D)");
    parser.add_argument("FILE",type=str,help="Filename to upload/download");


    #Parse arguments: 
    args = parser.parse_args(); 
    ip = args.IP; 
    port = args.PORT; 
    command = args.COMMAND;
    filename = args.FILE;
    validate_input(ip,port,command,filename);

    #Connect to server: 
    sock = socket.socket(socket.AF_INET,socket.SOCK_STREAM); 
    try:
        sock.connect((ip,port));
        if(command == "U"):
            upload(sock,filename); 
        elif(command == "D"): 
            download(sock,filename); 
    except socket.error:  
        print("Error: something went wrong with the socket.\n",file=sys.stderr); 

    sock.close();    

main(); 
