FTPServer: FTPServer.o
	gcc -O -c server/FTPServer.c 
FTPClient: FTPClient.o
	gcc -O -c client/FTPClient.c
clean: 
	rm -f FTPServer.o FTPClient.o