#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <unistd.h>
#include <netinet/in.h>

void serve_client(int server_fd, int client_fd, char *client_name, char **users_list, char **pass_list);
int userExist(char *user_name, char **users_list, int len);
int validPassword(char *pass, char **pass_list, int index);

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
	char *users_list[] = {"long_phan", "long_nguyen"};
	// array of passwords corresponding to users
	char *pass_list[] = {"1234", "abcd"};

	//1. Create a socket and server address structure
	int server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd < 0)
	{
		perror("Socket: ");
		return (-1);
	}

	struct sockaddr_in server_address;
	memset(&server_address, 0, sizeof(server_address));

	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(9000);
	server_address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

	//2. Bind the socket with the server address
	if (bind(server_fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
	{
		perror("Bind: ");
		return -1;
	}

	//3. Socket starts to listen for connections
	if (listen(server_fd, 2) < 0)
	{
		perror("Listen: ");
		return -1;
	}

	//4. accept
	struct sockaddr_in client_address;				 //we to pass this to accept method to get client info
	int client_address_len = sizeof(client_address); // accept also needs client_address length
	char client_name[50];

	printf("Server listening...\n");

	while (1)
	{
		int client_fd = accept(server_fd, (struct sockaddr *)&client_address, (socklen_t *)&client_address_len); // waiting for connections from a client

		inet_ntop(AF_INET, &client_address.sin_addr, client_name, sizeof(client_name));

		if (client_fd < 0)
		{
			perror("Accept: ");
			return -1;
		}

		printf("Connected to %s\n", client_name);

		int pid = fork();
		if (pid == 0)
		{
			serve_client(server_fd, client_fd, client_name, users_list, pass_list);
		}
	}

	//5. close the socket
	close(server_fd);

	return 0;
}

/**
 * 
*/
void serve_client(int server_fd, int client_fd, char *client_name, char **users_list, char **pass_list)
{
	char message[100];
	int index;				   // index of the user and their password
	int set_user = 0;		   // 0 means user is not set and 1 means otherwise
	int authenticate_user = 0; // 0 means user has not authenticated and 1 means otherwise

	while (1)
	{
		memset(message, 0, sizeof(message));
		if (recv(client_fd, message, sizeof(message) - 1, 0) > 0)
		{
			printf("Message from %s: %s\n", client_name, message);

			if (strncmp(message, "USER ", 5) == 0) // USER command
			{
				if (set_user == 1)
				{
					strcpy(message, "User already set");
					send(client_fd, message, strlen(message), 0);
				}
				else
				{
					char user_name[100];
					strncpy(user_name, &message[5], sizeof(message) - 5); // get username
					index = userExist(user_name, users_list, 2);		  // update index of user name and password
					if (index >= 0)
					{ // if user exists
						set_user = 1;
						strcpy(message, "Username OK, password required");
						send(client_fd, message, strlen(message), 0);
					}
					else
					{
						strcpy(message, "Username doesn't exist");
						send(client_fd, message, strlen(message), 0);
					}
				}
			}
			else if (strncmp(message, "PASS ", 5) == 0) // PASS command
			{
				if (set_user == 1) // if user is set
				{
					char pass[100];
					strncpy(pass, &message[5], sizeof(message) - 5); // get password
					if (validPassword(pass, pass_list, index) == 1)	 // if correct password or user has authenticated
					{
						authenticate_user = 1;
						strcpy(message, "Authentication complete");
						send(client_fd, message, strlen(message), 0);
					}
					else if (authenticate_user == 1)
					{
						strcpy(message, "Already authenticated with user name and password");
						send(client_fd, message, strlen(message), 0);
					}
					else
					{
						strcpy(message, "Wrong Password");
						send(client_fd, message, strlen(message), 0);
					}
				}
				else
				{
					strcpy(message, "Set USER first");
					send(client_fd, message, strlen(message), 0);
				}
			}
			else if (strncmp(message, "PWD", 3) == 0 || strncmp(message, "LS", 2) == 0)
			{
				if (authenticate_user == 0)
				{
					strcpy(message, "Authenticate first");
					send(client_fd, message, strlen(message), 0);
				}
				else
				{
					FILE *fp;
					char result[200];
					char line[100];
					char command[10];
					strncpy(command, &message[0], strlen(message)); // store away system command
					fp = popen(command, "r");
					while (fgets(line, sizeof(line), fp) != NULL) //read each line of the file
					{
						strcat(result, line);
						memset(line, 0, sizeof(line));
					}
					result[strlen(result) - 1] = '\0'; // remove \n at the end
					send(client_fd, result, strlen(result), 0);
					memset(result, 0, sizeof(result));
					memset(command, 0, sizeof(command));
					pclose(fp); // close the output stream
				}
			}
			else if (strncmp(message, "CD ", 3) == 0)
			{
				if (authenticate_user == 0)
				{
					strcpy(message, "Authenticate first");
					send(client_fd, message, strlen(message), 0);
				}
				else
				{
					char dir[50];
					strncpy(dir, &message[3], strlen(message) - 3);
					if (chdir(dir) == -1)
					{
						strcpy(message, "Directory does not exist");
						send(client_fd, message, strlen(message), 0);
					}
					else
					{
						strcpy(message, "CD successfully executed");
						send(client_fd, message, strlen(message), 0);
					}
				}
			}
			else if (strncmp(message, "PUT ", 4) == 0) // the idea is to accept a new TCP connection on a new port (21) for file transfer
			{
				if (authenticate_user == 0)
				{
					strcpy(message, "Authenticate first");
					send(client_fd, message, strlen(message), 0);
				}
				else
				{
					// we need the server to be ready for a new TCP connection for file transfer from client
					int port = rand() % 10000 + 1000; // randomize port in range [1000,9999]

					//1. Create new socket
					int server_sd = socket(AF_INET, SOCK_STREAM, 0);
					if (server_sd < 0)
					{
						perror("Socket: ");
						return;
					}
					struct sockaddr_in server_address;
					memset(&server_address, 0, sizeof(server_address));
					server_address.sin_family = AF_INET;
					server_address.sin_port = htons(port);
					server_address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

					//2. Bind the socket with the server address
					if (bind(server_sd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
					{
						perror("Bind: ");
						return;
					}

					//3. Socket starts to listen for connections
					if (listen(server_sd, 2) < 0)
					{
						perror("Listen: ");
						return;
					}

					char file_name[100];
					char file_content[500];
					FILE *file;
					strncpy(file_name, &message[4], sizeof(message) - 4); // get the file name

					// send the port for the new TCP connection to client
					memset(message, 0, sizeof(message));
					sprintf(message, "%i", port);
					send(client_fd, message, strlen(message), 0);

					int client_sd = accept(server_sd, NULL, NULL); // server accepts connection

					memset(file_content, 0, sizeof(file_content));
					recv(client_sd, file_content, sizeof(file_content) - 1, 0); // wait for file_content to be received

					file = fopen(file_name, "w"); // open a new file to write to
					fputs(file_content, file);	  // move file content to FILE stream

					memset(file_name, 0, sizeof(file_name));
					memset(file_content, 0, sizeof(file_content));
					fclose(file); // close file stream

					// close TCP connection that receives file
					close(client_sd);
					close(server_sd);

					// send confirmation message to client
					strcpy(message, "PUT file successful");
					send(client_fd, message, strlen(message), 0);
				}
			}
			else if (strncmp(message, "GET ", 4) == 0)
			{
				if (authenticate_user == 0)
				{
					strcpy(message, "Authenticate first");
					send(client_fd, message, strlen(message), 0);
				}
				else
				{
					char file_name[100];
					FILE *file;
					char file_content[500];
					char line[100];
					strncpy(file_name, &message[4], sizeof(message) - 4);
					file = fopen(file_name, "r");
					if (file == NULL)
					{
						strcpy(message, "File not found");
						send(client_fd, message, strlen(message), 0);
					}
					else
					{
						// we need the server to be ready for a new TCP connection for file transfer from client
						int port = rand() % 10000 + 1000; // randomize port in range [1000,9999]

						//1. Create new socket
						int server_sd = socket(AF_INET, SOCK_STREAM, 0);
						if (server_sd < 0)
						{
							perror("Socket: ");
							return;
						}
						struct sockaddr_in server_address;
						memset(&server_address, 0, sizeof(server_address));
						server_address.sin_family = AF_INET;
						server_address.sin_port = htons(port);
						server_address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

						//2. Bind the socket with the server address
						if (bind(server_sd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
						{
							perror("Bind: ");
							return;
						}

						//3. Socket starts to listen for connections
						if (listen(server_sd, 2) < 0)
						{
							perror("Listen: ");
							return;
						}

						// send the port for the new TCP connection to client
						memset(message, 0, sizeof(message));
						sprintf(message, "%i", port);
						send(client_fd, message, strlen(message), 0);

						int client_sd = accept(server_sd, NULL, NULL); // server accepts connection

						memset(file_content, 0, sizeof(file_content));
						while (fgets(line, sizeof(line), file) != NULL) // read each line of file
						{
							strcat(file_content, line);
							memset(line, 0, sizeof(line));
						}
						send(client_sd, file_content, strlen(file_content), 0); // send file_content to server through the new TCP connection

						memset(file_name, 0, sizeof(file_name));
						memset(file_content, 0, sizeof(file_content));
						fclose(file); // close file stream

						// close TCP connection that receives file
						close(client_sd);
						close(server_sd);
					}
				}
			}
			else if (strncmp(message, "QUIT", 4) == 0)
			{
				close(client_fd);
				printf("Connection to %s terminated\n", client_name);
				break;
			}
		}
	}
};

/**
 * 
*/
int userExist(char *user_name, char **users_list, int len)
{ // return index of user_name
	int i;
	for (i = 0; i < len; i++)
	{
		if (strncmp(users_list[i], user_name, strlen(users_list[i])) == 0)
		{ // if the string is in the array
			return i;
		}
	}
	return -1; // if user name not found then return -1
}

/**
 * 
*/
int validPassword(char *pass, char **pass_list, int index)
{ // check the pass of user name at an index
	if (strcmp(pass_list[index], pass) == 0)
	{ // if correct password
		return 1;
	}
	return 0;
}