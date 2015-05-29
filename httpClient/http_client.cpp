#include "sysops.h"
#include "page_node.h"
using namespace std;

vector<bool> ops(3, false);
set<string> visited;
set<string> nodes;
set<string> files;
string root_addr;
string root_path;
int node_count = 0;
int depth_level = 0;

fstream log_file;

int connect_to_host(char* addr){
	char hostip[BUFFLEN];
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	struct hostent* host_name;
	struct in_addr** addr_list;
    struct sockaddr_in serv_addr;

    printf("\nConnecting to: %s\n", addr);

	host_name = gethostbyname(addr);
	if(host_name == '\0'){
		cerr << "Gethost error\n";
		if(ops[2]){
			log_file << "Gethost error\n";
		}
		close(sockfd);
		return 0;
	} 
	else{
		printf("\nName: %s", host_name->h_name);
	}
	addr_list = (struct in_addr**)host_name->h_addr_list;
	strcpy(hostip, inet_ntoa(*addr_list[0]));
	printf("\nIP: %s\n", hostip);
	
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(80);
    inet_aton(hostip, &serv_addr.sin_addr);

	if (connect(sockfd,(struct sockaddr*) &serv_addr,sizeof(serv_addr)) < 0){
		cerr << "Connect error\n";
		if(ops[2]){
			log_file << "Connect error " << strerror(errno) << endl;
		}
	}
    else printf("Connected\n");		
	/* Finnished establishing connection. */
	return sockfd;
}
/* Used to determine if a link is a html page, a file or nither. */
link_t type_check(string link){
	if(link.find("http") != string::npos || link.find("#") != string::npos || 
		link.find("/?") != string::npos || link.find("http") != string::npos ||
		link.find("mailto") != string::npos || link.find("ftp") != string::npos ||
	 	link.find("\n") != string::npos || link.find("//") != string::npos ||
	 	link[link.size() - 1] == '/')
		return TUNDEFINED;
	if(link.find(".html") < link.size()){
		return TNODE;
	}
	return TFILE;
}
/* Convert ../ links to full paths. */
string expand(string link, string path){
	size_t pos, pos2;

	root_addr = string("www.cs.stir.ac.uk");

	do{
		if(path == root_addr){
			cerr << "Not on this server\n";
			if(ops[2]){
				log_file << "Not on this server\n";
			}
			return *(new string());
		}
		pos = link.find("..");
		pos2 = path.rfind("/");
		if(pos2 == string::npos)
			path = root_addr;
		else
			path.assign(path, 0, pos2);
		if(pos == string::npos || pos != 0) 
			break;
		link.erase(0,3);
		cout << "\nExpanded: " << path + "/" + link << endl;
	}while(true);
	// cout << "Test - " << path + "/" + link << endl;
	return path + "/" + link;
}
/* Break link in path and filename for ease of access. */
void break_path(string pathto, string* path, string* file_name){
	size_t pos;
	pos = pathto.rfind("/");
	if(pos == string::npos){
		*path = "/";
		file_name->clear();
	}
	else{
		file_name->assign(pathto, pos, pathto.length());
		path->assign(pathto, 0, pos);
		*path += "/";
	}
}
/* Checks for error codes in the header. */
int check_header(string header){
	cout << "Header info: \n";
	size_t pos = header.find("HTTP/1.1 ");
	if(pos == string::npos){
		cerr << "Wrong header format\n";
		if(ops[2]){
			log_file << "Wrong header format\n";
		}
		return -1;
	}
	pos += 9;
	size_t pos2 = header.find(" ", pos);
	string aux = string(header, pos, pos2 - pos) ;
	int code = stoi(aux);
	if(code > 299 && code < 400){
		cout << "Redirected connection\n";
		if(ops[2]){
			log_file << "Redirected connection. Error code: " << code << endl;
		}		
		return -1;
	}
	else if(code > 399 && code < 500){
		cerr << "Connection error. Code: " << code << endl;
		cerr << header << endl;
		if(ops[2]){
			log_file << "Client error. Error code: " << code << endl;
		}
		else if(code >= 500){
			cerr << "Connection error. Code: " << code << endl;
			if(ops[2]){
				log_file << "Server error. Error code: " << code << endl;
			}
		}
		return -1;
	}
	return 0;
}
/* Parse source files and save the required links. */
void parse_source(string src, string pathto){
	size_t pos, pos2;
	string link, file_name, path;
	string source = src;
	break_path(pathto, &path, &file_name);
	// cout << source;
	const string tag = string("href");
    const string comm_open = string("<!--");
    const string comm_close = string("-->");
	// cout << src << endl << endl;
	/* remove commented text from parsing */
	pos = source.find(comm_open);
	if(pos < source.size()){
		pos2 = source.find(comm_close);
		if(pos2 != string::npos && pos2 < source.length()){
			source.erase(pos, pos2 - pos);
		}
		else
			source.erase(source.begin() + pos, source.end());
	}

	int i = 0;

	while(!source.empty() && i < 300){
		/* set a limit of 300 links before stopping */
		pos = source.find(tag);
		if(pos == source.size() || pos == string::npos)
			break;

		source = source.substr(pos);
		/* Find the the boundaries of the link. If no ' " ' character is found then search for ' ' ' . */
		pos = source.find("\"") + 1;
		if(pos >= source.length() || pos == string::npos) {
			source.clear();
			break;
		}
		source = source.substr(pos);
		pos = source.find("\"");
		if(pos >= source.length() || pos == string::npos) {
			cerr << "Malformed links\n";
			if(ops[2]){
				log_file << "Malformed links\n";
			}
			source.clear();
			break;
		}
		link = source.substr(0, pos);
		link = expand(link, path);
		switch(type_check(link)){
			case TNODE:
				if(visited.empty() || visited.find(link) == visited.end())
					nodes.insert(link);
				break;
			case TFILE:
				files.insert(link);
				break;
			case TUNDEFINED:
				break;
		}
		node_count++;
		i++;
	}
	cout << endl << "Parsing source code complete \nRuns: " << i << endl;
}
/* Receive source code found at a given link. */
int get_source(int sockfd, string link, string* src_code){
    string request, aux, header;
    char buffer[BUFFLEN];
    cout << "Get source from: " << link << endl;
    size_t pos = link.find("/");
    if(pos == string::npos){
    	cerr << "Get source error - wrong link format\t";
    	if(ops[2]){
    		log_file << "Get source error - wrong link format\t";
    	}
    	return -1;
    }
    aux.assign(link, pos, aux.length() - pos);
    request = "GET " + aux + " HTTP/1.0\nHost: " + root_addr + "\n\n";
    cout << request << endl;
	if(send(sockfd, &request[0], BUFFLEN, 0) < 0){
		cerr << "Send error\n" << endl;
		if(ops[2]){
			log_file << "Send error\n";
		}
		return -1;
	}
	FILE* fd = fopen(&link[0], "w+");
	(*src_code).clear();
	int i = 0;
	do{
		memset(buffer, 0, BUFFLEN);
		int r = recv(sockfd, buffer, BUFFLEN - 1, 0);
		if( r == 0){
			cerr << "Host hang out" << endl;
			if(ops[2]){
				log_file << "Host hang out\n";
			}
			fclose(fd);
			return -1;
		} else if(r < 0) {
			cerr << "Receive error\n";
			if(ops[2]){
				log_file << "Receive error\n";
			}
			fclose(fd);
			return -1;
		}
		int n = 0;
		if(i == 0){
			char *p = strstr(buffer, "\r\n\r\n");
			header = string(buffer, 0, p + 4 - buffer);
			// n = p + 4 - buffer;
			strcpy(buffer, p + 4);
			cout << "Header: " << header;
			check_header(header);
		}
		i++;
		buffer[BUFFLEN] = '\0';
		*src_code += string(buffer);
		/* Remove the header. */
		/* Write to file the message. The while loop ensures fprintf finnishes before 
			the next break occurs and function closes. */
		fwrite(buffer + n, r - n, 1, fd);
		if(strstr(buffer, "</HTML>") != NULL || strstr(buffer, "</html>") != NULL){
			printf("exit\n");
			break;
		}
	}while(true);
	fclose(fd);
	return 0;
}

void display_links(){
	set<string>::iterator it;
	for(it = nodes.begin(); it != nodes.end(); it++){
		cout << "\t\t" << *it << endl;
	}
	cout << "Total links: " << nodes.size() << endl;
}

void display_files(){
	set<string>::iterator it;
	for(it = files.begin(); it != files.end(); it++){
		cout << "\t\t" << *it << endl;
	}
	cout << "Total files: " << files.size() << endl;
}
/* Write a single file to hard-disk by using multiple recv calls. */
int get_file(string link, int sockfd){
	cout << "Starting transmission of: " << link << endl;
	size_t pos;
	char buffer[BUFFLEN];
	long total_recv = 0;
	string request, header;
	string file_name, path, aux;
	int fd = open(&link[0], O_WRONLY | O_CREAT, 0644);
	if(fd == -1){
		cerr << "File open error: " << strerror(errno) << endl;
		if(ops[2]){
			log_file << "File open error: " << strerror(errno) << endl;
		}
		return -1;
	}
	break_path(link, &path, &file_name);
	cout << "\nPath: " << path << "\n" << "File name: " << file_name << "\n\n";
	pos = link.find("/");
	aux = string(link, pos, link.length() - pos);
	request = "GET " + aux + " HTTP/1.0\nHost: " + root_addr + "\n\n";
	cout << "\nSending request: " << request << endl; 
	if(send(sockfd, &request[0], request.size(), 0) < 0){
		cerr << "Request send error\n" << endl;
		if(ops[2]){
			log_file << "Request send error " << strerror(errno) << endl;
		}
		return -1;
	}
	int i = 0;
	long file_size, to_write = 0, r;
	bool check = false;
	do{
		check = false;
		memset(buffer, 0, BUFFLEN);
		r = recv(sockfd, buffer, BUFFLEN, 0);
		// if(send(sockfd, NULL, 1, 0) < 0){
		// 	cerr << "File send error\n" << endl;
		// 	if(ops[2]){
		// 		log_file << "Send error " << strerr(errno) << endl;
		// 	}
		// 	return -1;
		// }
		if(r == 0){
			cerr << "Host hang out" << endl;
			if(ops[2]){
				log_file << "Host hang out\n";
			}
			close(fd);
			return -1;
		} else if(r < 0) {
			cerr << "Receive error" << endl;
			if(ops[2]){
				log_file << "Receive error\n";
			}
			close(fd);
			return -1;
		}
		int n = 0;
		if(i == 0){
			char* p = strstr(buffer, "\r\n\r\n");
			if(p != NULL){
				header = string(buffer, 0, p + 4 - buffer);
				if(check_header(header) != 0){
					if(ops[2]){
						log_file << "File download failed for file " << file_name << "\n";
					}
					cerr << "File download fail\n";
					return -1;
					close(fd);
				}
				cout << header << endl;
				n = p - buffer + 4;
				pos = header.find("Content-Length: ");
				aux = string(header, pos);
				aux.erase(aux.find("\n") + 1, aux.length());
				aux.erase(0, aux.find(" ") + 1);
				file_size = atol(&aux[0]);
				cout << "File size: " << file_size << endl;
				r -= n;
			}
		}
		total_recv += r;
		i++;
		if(r > file_size) to_write = file_size;
		else to_write = r;
		file_size -= to_write;
		int written = 0, auxb;
		long counter = 0; /* Set un upper limit of the file size. */
		while(counter < 1000){ /* Call write function until its buffer was exhausted. */
			auxb = write(fd, buffer + n + written, to_write - written);
			if(auxb == -1){
				cerr << "\n\n" << strerror(errno) << "\n\n";
				cerr << "file write error\n";
				if(ops[2]){
					log_file << "Incomplete file writing. File \n" << file_name << " corrupted\n";
				}
				/* this is a fatal error, could cause the programm to crash */
				fsync(fd);
				close(fd);
				return -1;
			}
			written += auxb;
			if(written == to_write) break; 
			counter++;
		}
		if(counter == 1000){
			check = true;
		}
	}while(file_size > 0);
	if(check == true){
		cerr << "File " << file_name << " might be corrupt.\n";
	}
	close(fd);
	cout << "Chunks: " << i << "\nTotal received: " << total_recv << endl;
	cout << "\n******************\n";
	return 0;
}
/* Go through the set of files and call get_file() for each one. */
void receive_files(){
	set<string>::iterator it;
	string file_name, path;
	int sockfd, i;
	for(it = files.begin(), i = 0; it != files.end(); it++, i++){
		sockfd = connect_to_host(&root_addr[0]);
		if(sockfd == 0){
			cerr << "Fatal read error. Exiting.\n";
			if(ops[2]){
				log_file << "Fatal read error. Exiting.\n";
			}
		}
		break_path(*it, &path, &file_name);
		Sysops::mkdir_full(&path[0], &root_addr[0]);
		if(get_file(*it, sockfd) == -1){
			if(ops[2]){
				log_file << "File " << file_name << " failed to download\n";
			}
		}
		close(sockfd);
	}
}

int main(int argc, char* argv[]){
    string arg;
	string src_code;
	string aux, res_path, file_name, path;
    for(int i = 1; i < argc; i++){
    	if(string(argv[i]) == string("-r")) ops[0] = true;
    	else
	    	if(string(argv[i]) == string("-e")) ops[1] = true;
		    else
		    	if(string(argv[i]) == string("-o")) ops[2] = true;
		    	else{
		    		arg = string(argv[i]);
		    		/* First remove http tag from link. */
		    		size_t addr_pos = arg.find("http://");
		    		if(addr_pos == string::npos){
		    			addr_pos = arg.find("https://");
		    			if(addr_pos == string::npos){
		    				cerr << "\nWrong address format, include http / https\n";
		    				if(ops[2]){
								log_file << "Wrong address format. No http tag.\n";
							}
		    				return -1;
		    			}
		    			else
			    			arg.erase(0,8);
		    		}
		    		else
		    			arg.erase(0,7);
		    		/* Get web address of the host. */
		    		size_t pos = arg.find("/");
		    		if(pos == string::npos || pos == arg.length()){
		    			root_addr = arg + "/";
		    		}
		    		else
		    			root_addr = arg.substr(0,pos);
		    	}
    }

	int sockfd;
	string full_path = arg;
	string link = full_path;
	int counter = 0;


	if(ops[2]){
		log_file.open("logfile.log", fstream::out | fstream::app);
	}
	bool check = true;
	if(!ops[0] && !ops[1]){
		check = false; /* The next "do while" will only run once. */
	}
	int reconnect_limit = 20, reconnect_count = 0;
	do{
		sockfd = connect_to_host(&root_addr[0]);
		if(sockfd == 0){
			reconnect_count++;
			if(reconnect_limit == reconnect_count){
				cerr << "Connection error.\n";
				if(ops[2]){
					log_file << "Connection error.\n";
				}
				cout << "Connection error. Exiting.\n\n";
				return -1;
			}
			close(sockfd);
			continue;
		}
		else reconnect_count = 0;
		src_code.clear();
		break_path(link, &path, &file_name);
		Sysops::mkdir_full(&path[0], &root_addr[0]);
		if(get_source(sockfd, link, &src_code) == 1) break;
		if(src_code.length() == 0){
			cerr << "Source retreive error. Exiting.\n";
			if(ops[2]){
				log_file << "Source retreive error. Exiting.\n";
			}
			exit(0);
		}
		parse_source(src_code, link);
		cout << "\nNodes: " << nodes.size() << endl;
		display_links();
		visited.insert(link);
		cout << "\nVisited: " << link << endl;
		if(nodes.empty()) break;
		link = *nodes.begin();
		nodes.erase(*nodes.begin());
		cout << "[" << counter << "]" << "__________________________________________\n";
		counter++;
		close(sockfd);
	}while(check);
	node_count++;
	cout << "\nFile list: \n";
	/* Only download files if -e option is received. */
	if(ops[1])
		receive_files();
	if(ops[2]){
		log_file.close();
	}
	return 0;
}