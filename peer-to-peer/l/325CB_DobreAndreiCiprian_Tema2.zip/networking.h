#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <map>
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <algorithm>
#include <iterator>
#include <errno.h>
#include <libgen.h>
#include <signal.h>
#define MAX_CLIENTS 30
#define BUFFLEN 1024
#define MAX_FILES 50

typedef struct {
	int fd;
	char cl_name[255];
	char ip_addr[20];
	char cl_port[11];
	// std::map<std::vector<char>, size_t> files;
}client_info;

typedef struct {
	char unit[4];
	int size;
}size_format;