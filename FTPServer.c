#include<stdio.h>
#include<string.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<sys/types.h>
 #include <unistd.h>
#include<netinet/in.h>

int userExist(char* user_name, char** users_list, int len);

int main()
{
	/**
	 * How FTP server application works:
	 * First a socket is created
	 * Then the socket is binded to a port
	 * Then it listens for connections from clients
	 * The accepted connection is on a new socket, and the old socket is used to listen for other connections
	 * In a connection-oriented protocol, the client needs to establish a persistent connection with the server as the control channel and the subsequent communications have to be done on a different port
	 * In this case, this persistent connection is instantiated when the client authenticates with the server
	 * Each data stream between client and server needs to be done on separate TCP connection (first opened and then closed when data transfer finishes)
	*/

	// array of users
	char* users_list[] = {"long_phan", "long_nguyen"};
	// array of passwords corresponding to users
	char* pass_list[] = {"1234", "abcd"};
 
	//1. Create a socket and server address structure (address family, port number as 8888 and address are set)
	int server_fd = socket(AF_INET,SOCK_STREAM,0);
	if(server_fd<0)
	{
		perror("Socket: ");
		return (-1);
	}

	struct sockaddr_in server_address;
	memset(&server_address,0,sizeof(server_address));

	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(8889);
	server_address.sin_addr.s_addr = htonl(INADDR_ANY);
	
	//2. Bind the socket with the server address
	if(bind(server_fd,(struct sockaddr*) &server_address,sizeof(server_address))<0)
	{
		perror("Bind: ");
		return -1;
	}

	//3. Socket starts to listen for connections
	if(listen(server_fd,2)<0)
	{
		perror("Listen: ");
		return -1;
	}

	//4. accept
	struct sockaddr_in client_address;				//we to pass this to accept method to get client info
	int client_address_len = sizeof(client_address); // accept also needs client_address length
	
	char client_name[50];

	while(1)
	{
		int client_fd = accept(server_fd, (struct sockaddr*)&client_address,(socklen_t *)&client_address_len); // waiting for connections from a client
		char message[100];
		
		inet_ntop(AF_INET,&client_address.sin_addr,client_name,sizeof(client_name));

		if(client_fd<0)
		{
			perror("Accept: ");
			return -1;
		}

		int pid = fork();
		if(pid==0)
		{
			char command[100];
			char user_name[100];
			char response[100];
			
			int set_user = 0; // 0 means user is not set and 1 means otherwise
			
			while(1)
			{
				memset(message,0,sizeof(message));
				
				
				if(recv(client_fd,message,sizeof(message)-1,0)>0)
				{
					if(strcmp(message,"bye")==0)
					{
						break;
					}
					// printf("%s: %s \n",client_name, message);

					strncpy(command, &message[0], 5); 
					command[5] = 0;	
					printf("Command: %s\n", command);
					if (strncmp(command, "USER ", 5) == 0) { // if command is USER user_name
						strncpy(user_name, &message[5], sizeof(message)-5);
						if (userExist(user_name, users_list, 2) >= 0) { // if user exists
							set_user = 1;
							strcpy(response, "Username OK, password required");
							send(client_fd,response,strlen(response),0);

						}
						else {
							strcpy(response,"Username not recognized");
							send(client_fd,response,strlen(response),0);
						}	
					}
					
					else if (strncmp(command, "PASS ", 5) == 0) { 
						if (set_user == 1){
							char input[100] = "User authorized";
							strcpy(response,input);
							printf("response for correct pass: %s\n", response);
							send(client_fd,response,sizeof(response),0);
						}
						else {
							char input[100] = "User not recognize, please use USER to pass your user name first";
							strcpy(response,input);
							send(client_fd,response,sizeof(response),0);
						}
						
					}
					else {
						strcpy(response, "Command not recognized");
						send(client_fd,response,sizeof(response),0);
					}
					
				}
			}

			printf("Client disconnected...! \n");
			close(client_fd);
		}
	}

	close(server_fd);
	//5. send/receive 
	//6. close the socket
	return 0;
}

int userExist(char* user_name, char** users_list, int len) { // return index of user_name
	int i;
  	for(i = 0; i < len; i++) {
		if(strncmp(users_list[i], user_name, strlen(user_name)) == 0) { // if the string is in the array
			return i;
		}
  	}
  	return -1; // if user name not found then return -1
}