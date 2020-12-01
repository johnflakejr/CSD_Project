import argparse;
import ipaddress;
import socket;
import os; 


def upload(sock,filename):
    #Server doesn't care about the directory we get the file from on the client. 
    net_filename = os.path.basename(filename); 
    msg = "UPLOAD " + net_filename; 

    #Send the UPLOAD message: 
    sock.send(msg.encode())
    data = sock.recv(1024).decode();

    #If the server doesn't reply with READY, then abort. 
    if("READY" not in data): 
        print("Error uploading.\n"); 
        return; 

    #Open file, read bytes, and send them. 
    f = open(filename,"rb"); 
    read_bytes = f.read(4096); 
    while(read_bytes):
        sock.send(read_bytes); 
        read_bytes = f.read(4096); 
    f.close(); 
    
    

    
    

def download(sock,filename): 

    #Send the DOWNLOAD message: 
    msg = "DOWNLOAD " + filename
    sock.send(msg.encode())
    data = sock.recv(1024).decode();

    #If the server doesn't reply with READY, then abort. 
    if("FILE_NOT_FOUND" in data): 
        print("Error downloading - File not found.\n"); 
        return; 
    if("SENDING" not in data):  
        print("Error downloading - Unspecified error.\n"); 
        return; 


    #Open file, read bytes from server,and save them
    f = open(filename,"wb"); 
    data = sock.recv(4096); 
    while(data): 
        f.write(data); 
        data = sock.recv(4096); 
    f.close(); 



def validate_input(ip,port,command,filename): 
    #valid port? 
    if(port <0 or port > 65535):
        print("Invalid port.\n");
        quit();

    #valid IP?
    try: 
        ipaddress.ip_address(ip);
    except:  
        print("Invalid IP.\n");
        quit(); 

    #valid command?
    if(command != "U" and command != "D"):
        print("Invalid command.  U/D are the only valid commands."); 
        quit(); 

    #if uploading, check if file exists. 
    if(command == "U"): 
        if(not os.path.isfile(filename)):
            print("File doesn't exist."); 
            quit(); 

def main():
    #Setup arg parsing: 
    parser = argparse.ArgumentParser(description="Client for File Uploads");
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
    sock.connect((ip,port));
    if(command == "U"):
        upload(sock,filename); 
    elif(command == "D"): 
        download(sock,filename); 

    sock.close();    
    

main(); 
