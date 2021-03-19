#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <unistd.h>
#include <netinet/in.h>

int main(int argc, char *argv[])
{
	/**
	 * How a FTP client application works
	 * First a socket is created
	 * Then the client establishes a connection to the server
	 * In a connection-oriented protocol, the first connection that the client makes to server is a control connection which must be persistent
	 * In this case, in the control connection, the client needs to authenticate itself with the server for the control connection to be established
	 * 
	*/
	if (argc != 3)
	{
		printf("Error: Please only provide address and port as 2 arguments\n");
		return -1;
	}
	u_int32_t ip;
	if (strcmp(argv[1], "127.0.0.1") == 0)
		ip = INADDR_LOOPBACK;
	else
	{
		printf("Error: Only support localhost connection at 127.0.0.1\n");
		return -1;
	}
	int port = atoi(argv[2]);

	// Create a socket and set address family, port number and address
	int server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd < 0)
	{
		perror("Socket: ");
		return (-1);
	}

	struct sockaddr_in server_address;
	memset(&server_address, 0, sizeof(server_address));

	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(port);
	server_address.sin_addr.s_addr = htonl(ip);

	// The client makes connection the server
	if (connect(server_fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
	{
		perror("Connect :");
		return -1;
	}

	char message[100];

	while (1)
	{
		printf("ftp> ");

		//gets(message);  //not safe
		fgets(message, 100, stdin);			 //more safe but has no \n at the end,
		message[strcspn(message, "\n")] = 0; //lets add it

		if (strncmp(message, "!CD ", 4) == 0) // '!CD' command
		{
			char dir[100];
			strncpy(dir, &message[4], sizeof(message) - 4);
			if (chdir(dir) == -1)
			{
				printf("Directory does not exist\n");
			}
		}
		else if (strncmp(message, "!PWD", 4) == 0) // '!PWD' command
		{
			system("pwd");
		}
		else if (strncmp(message, "!LS", 3) == 0) // '!LS' command
		{
			system("ls");
		}
		else if (strncmp(message, "USER ", 5) == 0 || strncmp(message, "PASS ", 5) == 0 || strncmp(message, "PWD", 3) == 0 || strncmp(message, "LS", 2) == 0 || strncmp(message, "CD ", 3) == 0)
		{ // valid server commands
			send(server_fd, message, strlen(message), 0);
			memset(message, 0, sizeof(message));
			recv(server_fd, message, sizeof(message) - 1, 0);
			printf("%s\n", message);
		}
		else if (strncmp(message, "PUT ", 4) == 0)
		{
			char file_name[100];
			char file_content[500];
			FILE *file;
			char line[100];
			strncpy(file_name, &message[4], sizeof(message) - 4); // get file_name from PUT command

			if (!(file = fopen(file_name, "r")))
			{
				printf("File not found\n");
			}
			else
			{
				send(server_fd, message, strlen(message), 0); // send PUT command to server
				memset(message, 0, sizeof(message));
				recv(server_fd, message, sizeof(message) - 1, 0); // wait for confirmation message from server
				// printf("%s\n", message);
				if (strcmp(message, "Authenticate first") == 0)
				{
					printf("%s\n", message);
				}
				else
				{
					memset(file_content, 0, sizeof(file_content));
					while (fgets(line, sizeof(line), file) != NULL) // read each line of file
					{
						strcat(file_content, line);
						memset(line, 0, sizeof(line));
					}

					// open new TCP connection to send file
					int port = atoi(message);
					int server_sd = socket(AF_INET, SOCK_STREAM, 0);
					if (server_sd < 0)
					{
						perror("Socket: ");
						return (-1);
					}
					struct sockaddr_in server_address;
					memset(&server_address, 0, sizeof(server_address));

					server_address.sin_family = AF_INET;
					server_address.sin_port = htons(port); // new port for the new TCP connection
					server_address.sin_addr.s_addr = htonl(ip);
					if (connect(server_sd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
					{
						perror("Connect :");
						return -1;
					}

					send(server_sd, file_content, strlen(file_content), 0); // send file_content to server through the new TCP connection
					fclose(file);
					close(server_sd); // close the TCP connection for file transfer

					memset(message, 0, sizeof(message));
					recv(server_fd, message, sizeof(message) - 1, 0); // wait for confirmation message from server
					printf("%s\n", message);
				}
			}
		}
		else if (strncmp(message, "GET ", 4) == 0)
		{
			char file_name[100];
			char file_content[500];
			FILE *file;
			strncpy(file_name, &message[4], sizeof(message) - 4); // get file_name from GET command
			
			send(server_fd, message, strlen(message), 0);
			memset(message, 0, sizeof(message));
			recv(server_fd, message, sizeof(message) - 1, 0); // wait for confirmation message from server
			if (strcmp(message, "Authenticate first") == 0 || strcmp(message, "File not found") == 0)
			{
				printf("%s\n", message);
			}
			else
			{
				// open new TCP connection to send file
				int port = atoi(message);
				int server_sd = socket(AF_INET, SOCK_STREAM, 0);
				if (server_sd < 0)
				{
					perror("Socket: ");
					return (-1);
				}
				struct sockaddr_in server_address;
				memset(&server_address, 0, sizeof(server_address));

				server_address.sin_family = AF_INET;
				server_address.sin_port = htons(port); // new port for the new TCP connection
				server_address.sin_addr.s_addr = htonl(ip);
				if (connect(server_sd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
				{
					perror("Connect :");
					return -1;
				}
				recv(server_sd, file_content, sizeof(file_content) - 1, 0); // wait to receive file content from server

				file = fopen(file_name, "w"); // open a new file to write to
				fputs(file_content, file);

				memset(file_name, 0, sizeof(file_name));
				memset(file_content, 0, sizeof(file_content));
				fclose(file);
				close(server_sd);

				printf("GET file successful\n");
			}
		}
		else if (strncmp(message, "QUIT", 4) == 0)
		{ // terminate connection and close socket
			send(server_fd, message, strlen(message), 0);
			close(server_fd);
			printf("Connection terminated\nSocket closed\n");
			break;
		}
		else
		{
			printf("Invalid FTP command\n");
		}
	}

	return 0;
}
