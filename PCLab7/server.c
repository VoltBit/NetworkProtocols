#include "includes.h"

int main(int argc, char** argv){
	struct sockaddr_in srv_a;
	struct sockaddr_in cli_adr;
	char buff[MAX_LEN];
	int sfd_s = socket(PF_INET, SOCK_STREAM, 0);
	srv_a.sin_family = AF_INET;
	srv_a.sin_port = htons(atoi(argv[2]));
	memset();
	inet_aton(argv[1], &srv_a.sin_addr);
	// srv_a.
	if(bind(sfd_s, (const struct sockaddr*)&srv_a, sizeof(srv_a)) == -1){
		printf("Binding error");
		return -1;
	} else printf("Bnding successful");

	listen(sfd_s, 10);
	int n;
	char aux[MAX_LEN];
	int len = sizeof(cli_adr);
	int new_fd = accept(sfd_s, (const struct sockaddr*)&cli_adr, &len);
	return 0;
	do{
			recv(new_fd, buff, sizeof(buff), 0);
			printf("[S] Primit: %s\n", buff);
			strcpy(aux, buff);

			memset(&buff, 0, sizeof(buff));
			n = rand() % 2;
			if(n == 0)
				strcpy(buff, "NO");
			else strcpy(buff, "YES");

			printf("[S] Trimis: %s\n", buff);
			send(new_fd, buff, strlen(buff), 0);
	}while(strstr(aux, "exit") == NULL);
	printf("[S] Finished");
	return 0;
}