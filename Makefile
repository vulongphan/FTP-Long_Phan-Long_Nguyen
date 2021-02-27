FTPServer: FTPServer.o
	gcc -O -c FTPServer.c 
FTPClient: FTPClient.o
	gcc -O -c FTPClient.c
clean: 
	rm -f FTPServer.o FTPClient.o