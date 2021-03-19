# FTP-Long_Phan-Long_Nguyen
A FTP application that runs on top of sockets
The following is a C-based implementation of File Transfer Protocol (FTP). It has the following functionalities:
* Authentication:
    * USER: username
    * PASS: password for username to authenticate
* PWD: displays the current server directory
* !PWD: displays the current client directory
* LS: displays all files under the server directory
* !LS: displays all files under the current client directory
* CD: change the current server directory
* !CD: change the current client directory
* PUT filename: upload a file named filename from current client directory to current server directory
* GET filename: download a file named filename from current server directory to current client directory
* QUIT: quit FTP session and closes the TCP connection

### Code Details:
Client's source code is stored in "/client/FTPClient.c" and Server's source code is stored in "/server/FTPServer.c"

The directories 'client' and 'server' act the respective disks of clients and server. Any file transfer take place between these directories. The client and server maintain their files in the respective directories.

At least two different terminals should be opened (one for client and one for server)

For client, we need to enter the IP address and a Port number.

After connection, the client will have to give input a command in legal format so that necessary action can be taken. The server code is executed just once and waits for incoming connections. Server can be stopped by pressing "CTRL + C" on the corresponding terminal on which its code was compiled and run. Further, the client can make a connection with the server and keeps on giving necessary inputs. The client will be closed only when the user gives the "QUIT" input on its own.

The legal format for commands to be entered from client's side is specified as follows:

### Command Format:

* PUT: PUT <<filename>> (For example: PUT abc.txt)
* GET: GET <<filename>> (For example: GET xyz.txt)

### File Transfer specifics:

When we execute GET or PUT, assuming file exits, we create a new TCP connection on a random PORT. Then, we send the file over to the other end, line by line. We don't check for if file exists on the other end already, we instead automatically overwrites the old file.

### Assumption made:

*Filename should only have ASCII characters along with the extension (.txt, .c). When file is not found, it will be handled by printing "File not found".



