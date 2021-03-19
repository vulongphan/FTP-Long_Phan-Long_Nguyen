## About
This is the source code for a FTP (File Transfer Protocol) client/server model hosted locally that runs on top of sockets

## What does the code do
The source code includes server code (`server/FTPServer.c`) and client code (`client/FTPClient.c`) for a FTP application.
The terminal is used for client interface (please note that the interface now only supports Linux based machines).
The application currently supports the following commands
* `USER username`: set user name
* `PASS password`: authenticate using password
* `!CD directory`: change directory on client side
* `CD directory`: change directory on server side
* `!PWD`: get the current path on client side
* `PWD`: get the current path on server side
* `!LS`: list all sub directories at the current directory on client side
* `LS`: list all sub directories at the current directory on server side
* `PUT file`: Upload a file from client to server
* `GET file`: Download a file from server to client
* `QUIT`: Terminate client connection (alternately, press `Ctrl` + `C` to terminate the server)

## Installation
* `git clone` this repo to your local machine
* `cd` into `/server` and run `make && ./FTPServer` to first start the server process 
* `cd` into `/client` and run `make && ./FTPClient 127.0.0.1 9000` to start the client process
* Currently there are two valid users which can be found in `users_list` with their corresponding passwords in `pass_list` in `server/FTPServer.c` (`username: long_phan, password: 1234` and `username: long_nguyen, password: abcd`)
