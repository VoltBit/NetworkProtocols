#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <unistd.h>

#define BUFFLEN 1024

int main(int argc, char* argv[]){
	printf("Test\n");
	struct hostent* host_name;
	struct in_addr** addr_list;
    struct sockaddr_in serv_addr;

	printf("\nReceived: %s %s %s\n", argv[1], argv[2], argv[3]);
	char hostname[BUFFLEN], resource[BUFFLEN], hostip[BUFFLEN];
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);

		host_name = gethostbyname(argv[1]);
		if(host_name == '\0'){
			printf("Gethost error");
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
    
    	if (connect(sockfd,(struct sockaddr*) &serv_addr,sizeof(serv_addr)) < 0) 
        	error("ERROR connecting"); 
        else printf("Connected:\n");
        char request[BUFFLEN];
        sprintf(request, "%s %s %s\nHost: %s\n\n", argv[3], argv[2], "HTTP/1.1", argv[1]);
        printf("Sending: %s", request);
		send(sockfd, request, BUFFLEN, 0);
		char buffer[BUFFLEN];
		FILE* fd = fopen("file.html", "w+");
		// int fdd = open("file.html", O_WRONLY | O_CREAT, 0644);
		int check = 0;
		do{
			int r = recv(sockfd, buffer, BUFFLEN, 0);
			printf("%i\n", r);
			fprintf(fd, "%s", buffer);
			// write(fdd, buffer, strlen(buffer);
			if(strstr(buffer, "</HTML>") != NULL || strstr(buffer, "</html>") != NULL){
				check = 1;
				printf("exit\n");
			}
			if(r == -1){
				printf("Receive failed\n");
				return -1;
			}
			else{
				// printf("Received:\n %s\n", buffer);
				printf("Received");
			}
		}while(check == 0);
	return 0;
}