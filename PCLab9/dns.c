
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

int main(){

	int check = 1, i = 0;
	char input[100];
	struct hostent* host_name;
	struct in_addr** addr_list;
	while(check){
		scanf("%s", input);
		if(strcmp(input, "exit") == 0){
			return 0;
		}
		else if(input[0] == 'n'){
			scanf("%s", input);
			printf("Ok - %s\n", input);
			host_name = gethostbyname(input);
			if(host_name == '\0'){
				printf("Gethost error");
				return 0;
			} else {
				printf("Name: %s", host_name->h_name);

				printf("\nAlias list: \n");
				if(host_name->h_aliases[0] == '\0'){
					printf("Nu are lista de aliasuri");
				} else {
					while(host_name->h_aliases[i] != NULL){
						printf("%s", host_name->h_aliases[i]);
						i++;
					}
				}

				addr_list = (struct in_addr**)host_name->h_addr_list;

				printf("\n Addresses list: \n");
				if(host_name->h_addr_list[0] == NULL){
					printf("Nu are lista de adrese");
				} else {
					while(addr_list[i] != NULL){
						printf("%s\n", inet_ntoa(*addr_list[i]));
						i++;
					}
				}
			}

		} else if(input[0] == 'a'){
			scanf("%s", input);
			printf("OK - %s\n", input);
			struct in_addr addr;
			inet_aton(input, &addr);
			host_name = gethostbyaddr((void *)&addr, sizeof(addr), AF_INET);
			if(host_name == NULL){
				printf("Gethost error\n");
				return 0;
			} else {
				printf("Name: %s", host_name->h_name);

				printf("\nAlias list: \n");
				if(host_name->h_aliases[0] == NULL){
					printf("Nu are lista de aliasuri");
				} else {
					while(host_name->h_aliases[i] != NULL){
						printf("%s", host_name->h_aliases[i]);
						i++;
					}
				}

				addr_list = (struct in_addr**)host_name->h_addr_list;

				printf("\n Addresses list: \n");
				if(host_name->h_addr_list[0] == NULL){
					printf("Nu are lista de adrese");
				} else {
					while(addr_list[i] != NULL){
						printf("%s\n", inet_ntoa(*addr_list[i]));
						i++;
					}
				}
			}
		}
	}
	return 0;
}