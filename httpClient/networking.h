#include <cstdio>
#include <cstdlib>
#include <netdb.h>
#include <cstring>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h> 
#include <unistd.h>
#include <fcntl.h>
#include <string>
#include <list>
#include <vector>
#include <map>
#include <set>
#include <bitset>
#include <queue>
#include <iostream>
#include <fstream>
#include <errno.h>
#include <dirent.h>
#include <algorithm>
#include <regex>
#define BUFFLEN 1024
#define BUFFSRC 10000

typedef enum{
	TNODE, 		/* another html page */
	TFILE, 		/* any kind of file */
	TUNDEFINED	/* in case of malformed format, query, 'http://' links, directories*/
}link_t;