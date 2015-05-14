#include "networking.h"
#include "filesender.h"
using namespace std;

char client_name[50], dir_name[50], client_port[50];
char server_ip[50], server_port[50];
client_info clients[21];
std::map<std::vector<char>, size_t> files[MAX_CLIENTS];
int client_count = 0;
map<std::vector<char>, size_t> shared_files;
vector<string> con_peers;
Filesender filesenders[100];
int outgoing = 0;
FILE* log_stream;

void error(const char* msg){
    perror(msg);
    exit(0);
}

void unblock(int fd){
	int flags = fcntl(fd, F_GETFL, 0);
	fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

int is_registered(char* name){
	int i;
	for(i = 0; i < client_count; i++)
		if(strcmp(name, clients[i].cl_name) == 0) return i;
	return -1;
}

void get_info(char* argv[]){
	strcpy(client_name, argv[1]);
	strcpy(dir_name, argv[2]);
	strcpy(client_port, argv[3]);
	strcpy(server_ip, argv[4]);
	strcpy(server_port, argv[5]);
}

int get_input(char* buffer){
	fgets(buffer, BUFFLEN - 1, stdin);
	// strtok(buffer, "\n");
	char *pos;
	if ((pos=strchr(buffer, '\n')) != NULL)
	    *pos = '\0';
	printf("Input recieved: %s\n", buffer);
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
			printf("Received unknown command\n");
		}
	}
	if(strcmp(input[0], "infoclients") == 0) return 1;

	if(strcmp(input[0], "getshare") == 0 && strlen(input[1]) != 0){
		return 2;
	}
	else if(strlen(input[1]) == 0){
		printf("No client received. Usage: getshare <client>\n");
		return -1;
	}

	if(strcmp(input[0], "share") == 0 && strlen(input[1]) != 0){
		return 3;
	}
	else if(strlen(input[1]) == 0){
		printf("No filename received. Usage: share <filename>\n");
		return -1;
	}

	if(strcmp(input[0], "unshare") == 0 && strlen(input[1]) != 0){
		return 4;
	}
	else if(strlen(input[1]) == 0){
		printf("No filename received. Usage: unshare <filename>\n");
		return -1;
	}

	if(strcmp(input[0], "getfile") == 0 && strlen(input[1]) != 0){
		return 5;
	}
	else if(strlen(input[1]) == 0 || strlen(input[2]) == 0){
		printf("Wrong getfile command. Usage: getfile <client> <filename>\n");
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

void solve_error(int n){
	switch(n){
		case -1:
			printf("Eroare la conectare");
			fprintf(log_stream, "%i : %s\n", -1, "Eroare la conectare");
			break;
		case -2:
			printf("Eroare la citire/scriere pe socket");
			fprintf(log_stream, "%i : %s\n", -2, "Eroare la citire/scriere pe socket");
			break;
		case -3:
			printf("Client inexistent");
			fprintf(log_stream, "%i : %s\n", -3, "Client inexistent");
			break;
		case -4:
			printf("Client necunoscut");
			fprintf(log_stream, "%i : %s\n", -4, "Client necunoscut");
			break;
		case -5:
			printf("Fisier inexistent");
			fprintf(log_stream, "%i : %s\n", -5, "Fisier inexistent");
			break;
		case -6:
			printf("Fisier partajat cu acelasi nume");
			fprintf(log_stream, "%i : %s\n", -6, "Fisier partajat cu acelasi nume");
			break;
		default:
			printf("Unknown error: %i\n", n);
			break;
	}
}

void infoclients(int fd, char input[4][BUFFLEN]){
	char buffer[BUFFLEN];
	char info[MAX_CLIENTS][300];
	int n, aux;
	uint32_t count, i;
	
	send(fd, input[0], sizeof(input[0]), 0);

	n = recv(fd, &aux, sizeof(int), 0);
	send(fd, "ACK", 4, 0);
	count = ntohl(aux);
	printf("Nr of clients: %i\n", count);
	for(i = 0; i < count; i++){
		n = recv(fd, buffer, sizeof(buffer) - 1, 0);
		buffer[n + 1] = '\0';
		printf("%s\n", buffer);
		send(fd, "ACK", 4, 0);
		strcpy(info[i], buffer);
	}
	printf("Receive client info complete.\n");
	for(i = 0; i < count; i++){
		fprintf(log_stream, "%s\n", info[i]);
		sscanf(info[i], "%s%s%s", clients[i].cl_name, clients[i].ip_addr, clients[i].cl_port);
	}
	printf("Test:\n");
	for(i = 0; i < count; i++){
		printf("%s %s %s\n", clients[i].cl_name, 
			clients[i].ip_addr, clients[i].cl_port);
	}
	client_count = count;
}

void display_all_files(){
	int i;
	map<vector<char>, size_t>::iterator it;
	for(i = 0; i < client_count; i++){
		cout << clients[i].cl_name << "\n";
		for(it = files[i].begin(); it != files[i].end(); it++){
			cout << "\t";
			std::copy(it->first.begin(), it->first.end(), std::ostream_iterator<char>(std::cout));
			cout << " " << it->second << endl;
		}
	}
}

map<vector<char>, size_t> deserialize_map(char* srmap){
	int n, i;
	map<vector<char>, size_t> M;
	char tmp1[BUFFLEN], tmp2[BUFFLEN];
	// printf("To parse: %s\n", srmap);
	sscanf(srmap, "%s", tmp1);
	n = atoi(tmp1);
	strcpy(srmap, srmap + strlen(tmp1) + 1);
	strtok(srmap, " ");
	for(i = 0; i < n; i++){
		sscanf(srmap, "%s", tmp1);
		srmap = strtok(NULL, " ");
		sscanf(srmap, "%s", tmp2);
		srmap = strtok(NULL, " ");
		// printf("%s %s\n", tmp1, tmp2);
		vector<char> vect;
		vect.assign(tmp1, tmp1 + strlen(tmp1));
		M.insert(pair<vector<char>, size_t>(vect, atol(tmp2)));
	}
	return M;
}

int getshare(int fd, char input[4][BUFFLEN]){
	int n;
	char buffer[BUFFLEN];
	size_format sf;

	sprintf(buffer, "%s %s", input[0], input[1]);
	send(fd, buffer, strlen(buffer), 0);
	memset(buffer, 0, BUFFLEN);
	n = recv(fd, buffer, BUFFLEN, 0);
	if(n < 0){
		printf("receiving shared files failed.\n");
		return -7;
	}
	printf("%s\n", buffer);
	if(buffer[0] == 'O' || buffer[1] == 'K'){
		// printf("OK\n");
		strcpy(buffer, buffer + 3);

	}
	else{
		int err = atoi(buffer);
		// printf("Error: %i\n", err);
		solve_error(err);
		return err;
	}
	/* If the client does not exist already in the local register then an entry with its name
		is added. */
	int index = is_registered(input[1]);
	if(index == -1){
		strcpy(clients[client_count].cl_name, input[1]);
		memset(clients[client_count].ip_addr, 0 ,sizeof(clients[client_count].ip_addr));
		memset(clients[client_count].cl_port, 0 ,sizeof(clients[client_count].cl_port));
		index = client_count;
		client_count++;
	}
	files[index] = deserialize_map(buffer);
	// printf("Received:\n");
	// display_all_files();
	map<vector<char>, size_t>::iterator it;
	printf("%zu\n", files[index].size());
	fprintf(log_stream, "%zu\n", files[index].size());
	for(it = files[index].begin(); it != files[index].end(); it++){
		if(convert_size(it->second, &sf) == -1){
			printf("Conversion error\n");
			return 1;
		}
		std::copy(it->first.begin(), it->first.end(), std::ostream_iterator<char>(std::cout));
		std::copy(it->first.begin(), it->first.end(), buffer);
		cout << " " << sf.size << sf.unit << endl;
		fprintf(log_stream, "%s %i%s\n", buffer, sf.size, sf.unit);
	}
	return 0;
}

int share(int fd, char input[4][BUFFLEN]){
	struct stat fileinfo;
	char buffer[BUFFLEN] ,tmp[BUFFLEN];

	memset(buffer, 0, sizeof(buffer));
	memset(tmp, 0, sizeof(buffer));
	sprintf(tmp, "./%s/%s", dir_name, input[1]);

	printf("File to share: %s \n", tmp);

	if(stat(tmp, &fileinfo) != 0){
		printf("File does not exist or could not be opened.\n");
		solve_error(-5);
		return -5;
	}
	printf("size: %zd\n", fileinfo.st_size);
	sprintf(buffer, "%s %s %zd",input[0], input[1], fileinfo.st_size);
	printf("Sending: %s\n", buffer);
	send(fd, buffer, strlen(buffer), 0);
	recv(fd, buffer, sizeof(buffer), 0);
	printf("%s\n", buffer);
	printf("Completed sharing\n");
	fprintf(log_stream, "%s\n", buffer);

	vector<char> vect;
	vect.assign(input[1], input[1] + strlen(input[1]));
	shared_files.insert(pair<vector<char>, size_t>(vect, fileinfo.st_size));	

	return 0;
}

int unshare(int fd, char input[4][BUFFLEN]){
	char buffer[BUFFLEN];
	int r;
	sprintf(buffer, "%s %s", input[0], input[1]);
	send(fd, buffer, BUFFLEN, 0);
	r = recv(fd, buffer, BUFFLEN, 0);
	if(r <= 0){
		printf("Unshare message not confirmed\n");
		return -7;
	}
	if(buffer[0] == 'O' || buffer[1] == 'K'){
		// printf("OK\n");
		strcpy(buffer, buffer + 3);
		vector<char> vect;
		vect.assign(input[1], input[1] + strlen(input[1]));
		shared_files.erase(vect);
		fprintf(log_stream, "%s\n", "Succes");
		return 0;
	}
	else{
		int err = atoi(buffer);
		solve_error(err);
		return err;		
	}
}

int getfile(char input[4][BUFFLEN]){
	struct stat fileinfo;
	char tmp[BUFFLEN];
	sprintf(tmp, "./%s/%s", dir_name, input[2]);
	if(stat(tmp, &fileinfo) == 0){
		printf("File already exists");
			return -6;
	}
	int i;
	for(i = 0 ; i < client_count; i++){
		if(strcmp(clients[i].cl_name, input[1]) == 0){
			vector<char> vect;
			vect.assign(input[2], input[2] + strlen(input[2]));
			if(files[i].count(vect) == 0){
				printf("File %s not shared\n", input[2]);
				display_all_files();
				return -5;
			}
			else
				return 0;
		}
	}
	printf("Client unknown\n");
	return -4;
}

int main(int argc, char* argv[]){
	int r = 0, i;
	if(argc != 6){
		printf("Wrong client input. Exiting.\n");
		return -1;
	}
	get_info(argv);
	char log_name[260];
	strcpy(log_name, client_name);
	strcat(log_name, ".log");
	/*Create new log file if it does not exists. */
	log_stream = fopen(log_name, "a+");

	char buffer[BUFFLEN], response[BUFFLEN], input[4][BUFFLEN], tmp[BUFFLEN];
	int srvfd, sockfd, new_sockfd, fdmax;
	struct sockaddr_in serv_addr, peer_addr, cl_addr;

    fd_set read_fds;
    fd_set tmp_fds;

    /* Start establishing connection to server. */

	srvfd = socket(AF_INET, SOCK_STREAM, 0);
	
	if (srvfd < 0) 	
        error("Client: opening socket error.");
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(server_port));
    inet_aton(server_ip, &serv_addr.sin_addr);

   	if(connect(srvfd, (const struct sockaddr*) &serv_addr, (socklen_t)sizeof(serv_addr)) != 0)
   		error("Client");

   	strcpy(buffer, client_name);
   	strcat(buffer, " ");
   	strcat(buffer, client_port);
   	send(srvfd, buffer, sizeof(buffer), 0);
   	if(recv(srvfd, response, sizeof(response), 0) < 0){
   		printf("Client: recv error.");
   		return -1;
   	}
   	printf("Establishing connection: %s\n", response);

    /* Finished establishing connection to server. */

    /* Open socket for listening to incomming connections from other peers */

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0){
    	printf("Socket error. Exiting.\n");
    	return -1;
    }
    // unblock(sockfd);
    // unblock(srvfd);
    memset((char*) &peer_addr, 0, sizeof(serv_addr));
    peer_addr.sin_family = AF_INET;
    peer_addr.sin_port = htons(atoi(client_port));
    peer_addr.sin_addr.s_addr = INADDR_ANY;

    if(bind(sockfd, (struct sockaddr*) &peer_addr, sizeof(struct sockaddr)) < 0){
    	error("Error on binding");
    	exit(-1);
    }

    if(listen(sockfd, MAX_CLIENTS) == -1){
    	error("Error on listen");
    	exit(-1);
    }

   	FD_ZERO(&read_fds);
   	FD_ZERO(&tmp_fds);
    FD_SET(srvfd, &read_fds);
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
				if(i == sockfd){
					printf("\nGot a peer:\n");
					socklen_t cl_len = sizeof(cl_addr);
					new_sockfd = accept(sockfd, (struct sockaddr*)&cl_addr, &cl_len);
					if(new_sockfd == -1){
						error("Accept error");
					}
					else {
						unblock(new_sockfd);
						FD_SET(new_sockfd, &read_fds);
						if(fdmax < new_sockfd) fdmax = new_sockfd;
						printf("Peer successfully connected on socket %i\n", new_sockfd);
						memset(buffer, 0, BUFFLEN);
						memset(tmp, 0, BUFFLEN);
						r = recv(new_sockfd, buffer, BUFFLEN - 1, 0);
						if(r <= 0){
							printf("Receive error from peer\n");
							break;
						}
						buffer[r + 1] = '\0';
						sprintf(tmp, "./%s/%s", dir_name, buffer);
						printf("File name: %s, full path: %s\n", buffer, tmp);
						r = filesenders[outgoing].setup(tmp, new_sockfd);
						if(r != 0){
							solve_error(r);
						}
						outgoing++;
					}
				}
				else if(i == 0){
					get_input(buffer);
					if(strcmp(buffer, "quit") == 0){
						printf("Received quit message. Closing client %s\n", client_name);
						fprintf(log_stream, "%s> %s\n", client_name, buffer);
						FD_CLR(srvfd, &read_fds);
						close(srvfd);
						return 0;
					}
					int cmd = parse_command(buffer, input);
					switch(cmd){
						case 1:
							fprintf(log_stream, "%s> %s\n", client_name, input[0]);
							printf("Starting infoclients\n");
							infoclients(srvfd, input);
							break;
						case 2:
							fprintf(log_stream, "%s> %s %s\n", client_name, input[0], input[1]);
							printf("Starting getshare\n");
							getshare(srvfd, input);
							break;
						case 3:
							fprintf(log_stream, "%s> %s %s\n", client_name, input[0], input[1]);
							printf("Starting share\n");
							share(srvfd, input);
							break;
						case 4:
							fprintf(log_stream, "%s> %s %s\n", client_name, input[0], input[1]);
							printf("Starting unshare\n");
							unshare(srvfd, input);
							break;
						case 5:
							fprintf(log_stream, "%s> %s %s %s\n", client_name, input[0], input[1], input[2]);
							printf("Starting getfile\n");
							r = getfile(input);
							if(r == 0){
								int j;
								for(j = 0; j < client_count; j++){
									if(strcmp(input[1], clients[j].cl_name) == 0){
										new_sockfd = socket(AF_INET, SOCK_STREAM, 0);
										cl_addr.sin_family = AF_INET;
										cl_addr.sin_port = htons(atoi(clients[j].cl_port));
										inet_aton(clients[j].ip_addr, &cl_addr.sin_addr);
										FD_SET(new_sockfd, &read_fds);
										if(connect(new_sockfd, (const struct sockaddr*)&cl_addr, (socklen_t)sizeof(sockaddr_in)) != 0){
											solve_error(-1);
											printf("%s\n", strerror(errno));
										}
										else{
											unblock(new_sockfd);
											printf("Connection with %s established\n", input[1]);
											send(new_sockfd, input[2], strlen(input[2]) + 1, 0);
										}
										break;
									}
								}
							}
							else{
								solve_error(r);
							}
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
				else if(i == srvfd){
					// printf("I got here this way: %i\n", srvfd);
					if(recv(srvfd, buffer, sizeof(buffer), 0) == 0){
						fprintf(log_stream, "%s> quit\n", client_name);
						printf("Server closed. Exiting.\n");
						FD_CLR(srvfd, &read_fds);
						return 0;
					}
					printf("Received from server: %s\n", buffer);
				}
				else{
					if((r = recv(i, buffer, sizeof(buffer), 0)) <= 0){
						if(r == 0)
							printf("Peer on socket %i hung up\n", i);
						else
							printf("Receive error form peer.\n");
						close(i);
						FD_CLR(i, &read_fds);
					}
					else{
						printf("recived from peer on socket %i: %i bytes\n", i, r);
					}

				}
			}
		} //



		/* Continue to send of files. */
		for(i = 0; i < outgoing; i++){
			if(filesenders[i].handle_io() == 1){
				outgoing--;
				printf("Finished sending file\n");
			}
			printf("F%i ", i);
		}
	}
	fclose(log_stream);
	return 0;
}