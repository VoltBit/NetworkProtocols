#include "includes.h"

int main(int argc, char** argv){
	struct sockaddr_in to;
	char buff[MAX_LEN];
	int sfd_c = socket(PF_INET, SOCK_STREAM, 0);
	to.sin_family = AF_INET;
	to.sin_port = htons(atoi(argv[2]));
	inet_aton(argv[1], &to.sin_addr);
	if(connect(sfd_c, (const struct sockaddr*)&to, sizeof(to)) == -1) 
		printf("Connecting failed");
	else printf("Connect to server successful");
	char aux[MAX_LEN];
	do{

		memset(&buff, 0, sizeof(buff));
		read(0, buff, sizeof(buff));
		strcpy(aux, buff);
		printf("Trimit mesaj: %s\n", buff);

		send(sfd_c, buff, strlen(buff), 0);

		memset(&buff, 0, sizeof(buff));

		recv(sfd_c, buff, sizeof(buff), 0);
		printf("Primesc de la server: %s", buff);

	}while(strstr(aux, "exit") == NULL);
	printf("[S] Finished");
	return 0;
}