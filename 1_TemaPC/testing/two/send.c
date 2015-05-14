  #include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

#include "lib.h"

#define HOST "127.0.0.1"
#define PORT 10000

void ls(){
	char* name;

	name = malloc(255 * sizeof(char));
	printf("Insert name of directory: ");
	scanf("%s", name);
	DIR* dirStream = opendir(name);
	struct dirent* dirContents = readdir(dirStream);
	while(dirContents){
		printf("%s ", dirContents->d_name);
		dirContents = readdir(dirStream);	
	}
	printf("\n");
}

void cd(){
	char* name;
	name = malloc(255 * sizeof(char));
	printf("change directory path: ");
	scanf("%s", name);
	printf("Test:%sTest\n",name);
	chdir(name);
	printf("%s", getcwd(name, 255 * sizeof(char)));
	printf("\n\n");
}

void cp(){
	char name[255];
	void* buffer = malloc(100 * sizeof(char));
	printf("file to copy: ");
	scanf("%s", name);
	FILE* fileStream = fopen(name, "r");
	char newName[255] = "new_";
	strcat(newName, name);
	int fileDescriptor = open(newName, O_CREAT|O_WRONLY|O_TRUNC);
	while(!feof(fileStream)){
		fread(buffer, 100, 1, fileStream);
		write(fileDescriptor, buffer, 100);
	}
	close(fileDescriptor);
}

int parseMsg(msg message){
	if(message.payload[0] == 'l')return 1;
		else if(message.payload[0] == 'c' && message.payload[1] == 'd')return 2;
			else if(message.payload[0] == 'c' && message.payload[1] == 'p') return 3;
				else if(message.payload[0] == 's') return 4;
					else if(strcmp("exit exit",message.payload) == 0) return 0;
	return 5;
}

int main(int argc, char *argv[])
{
	msg t;
	int i, res = 0;

	// ls();
	// cd();
	// cp();

	printf("[SENDER] Starting.\n");	
	init(HOST, PORT);
	/* printf("[SENDER]: BDP=%d\n", atoi(argv[1])); */
	
	char *comms[] = {"cd 1", "cd ..", "cd 1", "ls .", "exit exit"};
	int mark = 0, n;
	for (i = 0; i < COUNT; i++) {

		/* read commands from file */
		if(mark < sizeof(comms) / sizeof(comms[0])){
			t.len = strlen(comms[mark]);
			// printf("test: %s ", comms[mark]);
			memcpy(t.payload, comms[mark], t.len + 1);
			n = parseMsg(t);
			// printf("%i  (%i %s)\n", n, t.len, t.payload);
			mark++;
		}
		else{
			n = -1;
			t.len = MSGSIZE;
			memset(&t, 0, sizeof(msg));
		}
		/* send command with argument */
		res = send_message(&t);
		if (res < 0) {
			perror("[SENDER] Send error. Exiting.\n");
			return -1;
		}
		/* wait for ACK */
		res = recv_message(&t);
		if (res < 0) {
			perror("[SENDER] Receive error. Exiting.\n");
			return -1;
		}
		/* if I send exit command I need to stop the sender */
		if(n == 0) break;
		/* if I sent ls command I need to receive list of files */
		
		if(n == 1){
			res = recv_message(&t);
			int num;
			memcpy(&num, t.payload, t.len);
			printf("\n\tnumber of files: %i\n", num);
			if (res < 0) {
				perror("[SENDER] Receive error. Exiting.\n");
				return -1;
			}
			printf("[SENDER] received message (nr of files): %i ", num);
			res = send_message(&t);
			if (res < 0) {
				perror("[SENDER] Send error. Exiting.\n");
				return -1;
			}
			int aux = num;
			while(aux < num){
				res = recv_message(&t);
				printf("[SENDER] Received file %s [%i]\n", t.payload, aux);
				res = send_message(&t);
				aux++;
			}
		}

		if(n == 222){
			/*receive number of files */
			res = recv_message(&t);
			if (res < 0) {
				perror("[SENDER] Receive error. Exiting.\n");
				return -1;
			}
			/* send ACK */
			t.len = MSGSIZE;
			memset(&t, 0, sizeof(msg));
			res = send_message(&t);
			if (res < 0) {
				perror("[SENDER] Send error. Exiting.\n");
				return -1;
			}
			int aux = 0, num;
			num = atoi(t.payload);
			while(aux < num){
				res = recv_message(&t);
				if (res < 0) {
					perror("[SENDER] Receive error. Exiting.\n");
					return -1;
				}
				/* send ACK */
				t.len = MSGSIZE;
				memset(&t, 0, sizeof(msg));
				res = send_message(&t);
				if (res < 0) {
					perror("[SENDER] Send error. Exiting.\n");
					return -1;
				}
				printf("file[%i]: %s", aux, t.payload);
				aux++;
			}
		}
	}
	printf("[SENDER] Job done, all sent.\n");
		
	return 0;
}
