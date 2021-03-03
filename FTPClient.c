#include<stdio.h>
#include<string.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<sys/types.h>
 #include <unistd.h>
#include<netinet/in.h>

int main()
{
	/**
	 * How a FTP client application works
	 * First a socket is created
	 * Then the client establishes a connection to the server
	 * In a connection-oriented protocol, the first connection that the client makes to server is a control connection which must be persistent
	 * In this case, in the control connection, the client needs to authenticate itself with the server for the control connection to be established
	 * 
	*/

	//1. Create a socket and set address family, port number and address
	int server_fd = socket(AF_INET,SOCK_STREAM,0);
	if(server_fd<0)
	{
		perror("Socket: ");
		return (-1);
	}

	struct sockaddr_in server_address;
	memset(&server_address,0,sizeof(server_address));

	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(9000);
	server_address.sin_addr.s_addr = htonl(INADDR_ANY);
	
	//2. The client makes connection the server which is listening on port 8888
	if(connect(server_fd,(struct sockaddr*)&server_address,sizeof(server_address))<0)
	{
		perror("Connect :");
		return -1;
	}

	char message[100];

	while(1)
	{
		printf("ftp> ");
		//gets(message);  //not safe
		fgets(message,100,stdin); //more safe but has no \n at the end,
		message[strcspn(message,"\n")]=0; //lets add it
	//3. send/rec
		send(server_fd,message,strlen(message),0);
		memset(message, 0, sizeof(message));
		recv(server_fd,message,sizeof(message)-1,0);
		printf("%s\n", message);
		if(strcmp(message,"bye")==0)  break;
	}

	//4. close
	close(server_fd);
	return 0;
}




