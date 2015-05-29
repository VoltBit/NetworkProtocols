#include "networking.h"
using namespace std;
int client_count = 0;
char server_p[21];
client_info clients[21];
std::map<std::vector<char>, size_t> files[MAX_CLIENTS];

int check_client(char* client_n){
	int i;
	for(i = 0; i < client_count; i++){
		if(strcmp(clients[i].cl_name, client_n) == 0) return i;
	}
	return -1;
}

void error(const char* msg){
    perror(msg);
    exit(0);
}

void remove_client(int cl_fd){
	int i, j;
	map<vector<char>, size_t>::iterator it;
	if(client_count == 1){
		memset(clients, 0, sizeof(clients));
		client_count = 0;
		return;
	}
	for(j = 0; j < client_count; j++){
		if(clients[j].fd == cl_fd){
			for(i = j; i < client_count - 1; i++)
				clients[i] = clients[i + 1];
			files[i].clear();
			// memmove(clients + j, clients + j + 1, (client_count - j - 1) * sizeof(*clients));
			break;
		}
	}
	client_count--;
}

int register_client(char* data, struct in_addr ip, int fd){
	int index = client_count;
	printf("To register: %s, size: %i, len: %i\n", data, (int)sizeof(data), (int)strlen(data));
	strtok(data, " ");
	if(strlen(data) == 0){
		printf("Wrong register information. Client not registered, connection dropped.");
		return -1;
	}
	int n = check_client(data);
	if(n != -1){
		printf("Server: Client already registered. Updating.\n");
		index = n;
		client_count--;
		// return 0;
	}
	clients[index].fd = fd;
	strcpy(clients[index].cl_name, data);
	data = strtok(NULL, " ");
	if(strlen(data) == 0){
		printf("Wrong register information. Client not registered, connection dropped.");
		memset(clients[index].cl_name, 0, strlen(clients[index].cl_name));
		return -1;
	}
	strcpy(clients[index].cl_port, data);
	
	char* tmp = inet_ntoa(ip);
	strcpy(clients[index].ip_addr, tmp);
	printf("IP Address test %s\n", tmp);
	client_count++;
	printf("\nRegistered client number %i: %s %s %s\n", index, clients[index].cl_name,
		clients[index].cl_port, clients[index].ip_addr);
	data = strtok(NULL, " ");
	return 0;
}

int parse_command(char* buffer, char input[4][BUFFLEN]){
	int counter = 0, check = 0;
	strtok(buffer, " ");
	while(buffer){
		strcpy(input[counter], buffer);
		// printf("%s\n", input[counter]);
		buffer = strtok(NULL, " ");
		counter++;
		if(counter == 4){
			printf("Server: received unknown command form client\n");
		}
	}
	if(strcmp(input[0], "infoclients") == 0) return 1;

	if(strcmp(input[0], "getshare") == 0 && strlen(input[1]) != 0){
		return 2;
	}
	else if(strcmp(input[0], "getshare") && strlen(input[1]) == 0){
		printf("Server: no client received. Usage: getshare <client>\n");
		return -1;
	}

	if(strcmp(input[0], "share") == 0 && strlen(input[1]) != 0 &&
		strlen(input[2]) != 0){
		return 3;
	}
	else if(strlen(input[1]) == 0){
		printf("Server: no filename received. Usage: share <filename>\n");
		return -1;
	} else if(strlen(input[2]) == 0){
		printf("Serve: file size not received for share command");
		return -1;
	}

	if(strcmp(input[0], "unshare") == 0 && strlen(input[1]) != 0){
		return 4;
	}
	else if(strlen(input[1]) == 0){
		printf("Server: no filename received. Usage: unshare <filename>\n");
		return -1;
	}

	if(strcmp(input[0], "getfile") == 0 && strlen(input[1]) != 0){
		return 5;
	}
	else if(strlen(input[1]) == 0 || strlen(input[2]) == 0){
		printf("Server: wrong getfile command. Usage: getfile <client> <filename>\n");
		return -1;
	}

	if(strcmp(input[0], "history") == 0){
		return 6;
	}

	if(strcmp(input[0], "quit") == 0){
		return 7;
	}
	return check;
}

int convert_size(size_t size, size_format* sf){
	int i = 1;
	while( size / (1 << 10) >= 1 ){
		size = size / (1 << 10);
		i++;
		if(i == 5){
			printf("File is too large.\n");
			return -1;
		}
	}
	switch(i){
		case 1:
			strcpy(sf->unit, "B");
			sf->size = (int)size;
			break;
		case 2:
			strcpy(sf->unit, "KiB");
			sf->size = (int)size;// / (1 << 10);
			break;
		case 3:
			strcpy(sf->unit, "MiB");
			sf->size = (int)size;// / (1 << 20);
			break;
		case 4:
			strcpy(sf->unit, "GiB");
			sf->size = (int)size;// / (1 << 30);
			break;
	}
	return 0;
}

void infoclients(int fd){
	char buffer[BUFFLEN], tmp[BUFFLEN];
	int n, i;
	uint32_t t = htonl(client_count);
	send(fd, &t, sizeof(uint32_t), 0);
	
	n = recv(fd, buffer, sizeof(buffer) - 1, 0);
	printf("%s\n", buffer);

	for(i = 0; i < client_count; i++){
		memset(tmp, 0, BUFFLEN);
		strcpy(tmp, clients[i].cl_name);
		strcat(tmp, " ");
		strcat(tmp, clients[i].ip_addr);
		strcat(tmp, " ");
		strcat(tmp, clients[i].cl_port);
		printf("Sending: %s [%i]\n", tmp, (int)strlen(tmp));
		send(fd, tmp, strlen(tmp), 0);
		n = recv(fd, buffer, sizeof(buffer) - 1, 0);
		buffer[n + 1] = '\0';
		printf("%s\n", buffer);
	}
	printf("Trimitere infoclient terminata.\n");
}

void serialize_map(map<vector<char>, size_t> M, char* buffer){
	char tmp[BUFFLEN], tmp2[BUFFLEN];
	map<vector<char>, size_t>::iterator it;
	memset(buffer, 0, BUFFLEN);
	buffer[0] = '\0';
	if(M.empty()) return;

	sprintf(buffer, "%zu ", M.size());
	for(it = M.begin(); it != M.end(); it++){
		memset(tmp, 0, BUFFLEN);
		tmp[0] = '\0';
		std::copy(it->first.begin(), it->first.end(), tmp);
		sprintf(tmp2, "%s %zu ", tmp, it->second);
		strcat(buffer, tmp2);
	}
	// printf("Serialize test: %s\n", buffer);
}

int get_share(int fd, char input[4][BUFFLEN]){
	int i, n;
	char buffer[BUFFLEN], tmp[BUFFLEN];
	for(i = 0; i < client_count; i++){
		if(strcmp(input[1], clients[i].cl_name) == 0){
			serialize_map(files[i], buffer);
			strcpy(tmp, buffer);
			sprintf(buffer, "%s %s", "OK", tmp);
			n = send(fd, buffer, strlen(buffer), 0);
			if(n < 0){
				printf("Sending shared files failed.\n");
			}
			printf("Sent size: %i\n", n);
			break;
		}
	}
	if(i == client_count){
		printf("Client not found\n");
		memset(buffer, 0, BUFFLEN);
		buffer[0] = '-';
		buffer[1] = '3';
		buffer[2] = '\0';
		send(fd, buffer, strlen(buffer) + 1, 0);
		return -3;
	}
	printf("Finished getshare.\n");
	return 0;
}


void display_all_files(){
	int i, count = 0;
	char  buffer[BUFFLEN];
	memset(buffer, 0, BUFFLEN);
	buffer[0] = '\0';
	map<vector<char>, size_t>::iterator it;
	for(i = 0; i < client_count; i++){
		count = 0;
		cout << clients[i].cl_name << "\n";
		for(it = files[i].begin(); it != files[i].end(); it++){
			cout << "\t";
			std::copy(it->first.begin(), it->first.end(), std::ostream_iterator<char>(std::cout));
			cout << " " << it->second << endl;
			count++;
		}
		if(count > 0) serialize_map(files[i], buffer);
		cout << endl << "Serialized: " << endl << buffer << endl;
	}
}

int unshare(int fd, char input[4][BUFFLEN]){
	int i;
	char buffer[BUFFLEN];
	map<vector<char>, size_t>::iterator it;
	for(i = 0; i < client_count; i++){
		if(clients[i].fd == fd){
			vector<char> vect;
			vect.assign(input[1], input[1] + strlen(input[1]));
			if(files[i].count(vect) == 0){
				printf("File not found\n");
				memset(buffer, 0, BUFFLEN);
				buffer[0] = '-';
				buffer[1] = '5';
				buffer[2] = '\0';
				send(fd, buffer, strlen(buffer) + 1, 0);
				return -5;
			}
			else{
				files[i].erase(vect);
				send(fd, "OK", strlen("OK"), 0);
				return 0;
			}
		}
	}
	printf("Finished unshare.\n");
	return -3;
}

void share(int fd, char input[4][BUFFLEN]){
	printf("Start sharing\n");
	char tmp[BUFFLEN];
	size_t fsize;
	strcpy(tmp, input[1]);
	fsize = atol(input[2]);
	int i;
	for(i = 0; i < client_count; i++){
		if(clients[i].fd == fd){
			printf("Socket: %i - %s\n", fd, clients[i].cl_name);
			vector<char> vect;
			vect.assign(input[1], input[1] + strlen(input[1]));
			std::copy(vect.begin(), vect.end(), std::ostream_iterator<char>(std::cout));
			files[i].insert(pair<vector<char>, size_t>(vect, fsize));
			break;
		}
	}
	send(fd, "Succes", 7, 0);
	printf("Completed sharing\n");
	// display_all_files();
}

int main(int argc, char* argv[]){
	int i, ret, cmdtype;
	char buffer[BUFFLEN], recv_input[4][BUFFLEN];
	struct sockaddr_in serv_addr, cl_addr;
	int sockfd, fdmax = 6, new_sockfd;
	fd_set read_fds;
	fd_set tmp_fds;
	printf("Server opened\n");
	if(argc != 2){
		printf("Wrong server input. Exiting.\n");
		return -1;
	}
	strcpy(server_p, argv[1]);

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0){
		printf("Socket error. Exiting.\n");
		return -1;
	}

	memset((char *) &serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(atoi(argv[1]));
	serv_addr.sin_addr.s_addr = INADDR_ANY;

	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(struct sockaddr)) < 0) 
    	error("Error on binding");
	
	if(listen(sockfd, MAX_CLIENTS) == -1){
		error("Error on listen");
		exit(-1);
	}

	FD_ZERO(&read_fds);
	FD_ZERO(&tmp_fds);
	FD_SET(sockfd, &read_fds);
	FD_SET(0, &read_fds);

	fdmax = sockfd;
	while(1){
		tmp_fds = read_fds;
		if (select(fdmax + 1, &tmp_fds, NULL, NULL, NULL) == -1) 
			error("ERROR in select");
		for(i = 0; i <= fdmax; i++){
			if(FD_ISSET(i, &tmp_fds)){
				// printf("Found: %i\n", i);
				/* Check the socket the server is listening to for new connections. */
				if(i == sockfd){
					socklen_t cl_len = sizeof(cl_addr);
					if((new_sockfd = accept(sockfd, (struct sockaddr *)&cl_addr, &cl_len)) == -1){
						error("Server accept");
					}
					else{
						FD_SET(new_sockfd, &read_fds);
						if(fdmax < new_sockfd) fdmax = new_sockfd;
						if((ret = recv(new_sockfd, buffer, BUFFLEN, 0)) <= 0){
							if(ret == 0)
								printf("Server: socket %i hung up, client not registered\n", new_sockfd);
							else error("Server");
							close(new_sockfd);
						}
						else
							// printf("test2: %s, size: %i, len: %i\n", buffer, (int)sizeof(buffer), (int)strlen(buffer));

							if(register_client(buffer, cl_addr.sin_addr, new_sockfd) == 0)
								send(new_sockfd, "Success", sizeof("Success"), 0);
					}
				}
				else{
					if(i == 0){
						fgets(buffer, BUFFLEN - 1, stdin);
						printf("Received input: %s\n", buffer);
						if(strcmp(buffer, "quit\n") == 0){
							printf("Server is now exiting.\n");
							int i;
							for(i = 3; i < fdmax; i++) 
								close(i);
							FD_ZERO(&read_fds);
							// close(sockfd);
							return 0;
						}
					}
					else{
						memset(buffer, 0, BUFFLEN);
						if((ret = recv(i, buffer, sizeof(buffer), 0)) <= 0){
							if(ret == 0){
								printf("Server: socket %i hung up\n", i);
								remove_client(i);
							}
							else error("Server");
							FD_CLR(i, &read_fds);
						}
						else{
							printf("Received from socket %i message: %s\n", i, buffer);
							// if(i > 2)
							// 	send(i, "Success", sizeof("Success"), 0);
							cmdtype = parse_command(buffer, recv_input);
							switch(cmdtype){
								case 1:
									printf("Starting infoclients\n");
									infoclients(i);
									break;
								case 2:
									printf("Starting getshare\n");
									get_share(i, recv_input);
									break;
								case 3:
									printf("Starting share\n");
									share(i, recv_input);
									break;
								case 4:
									printf("Starting unshare\n");
									unshare(i, recv_input);
									break;
								case 5:
									printf("Starting getfile\n");
									break;
								case 6:
									printf("Starting history\n");
									break;
								case 7:
									printf("Starting quit\n");
									break;
								default:
									printf("Wrong command\n");
									break;
							}
						}
					}
				}
			}
		}
	}

	printf("Server closed\n");
	return 0;
}