#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include "lib.h"

#define HOST "127.0.0.1"
#define PORT 10001

int parseMsg(msg message){
	if(message.payload[0] == 'l')return 11;
		else if(message.payload[0] == 'c' && message.payload[1] == 'd')return 2;
			else if(message.payload[0] == 'c' && message.payload[1] == 'p') return 3;
				else if(message.payload[0] == 's') return 4;
					else if(strcmp("exit exit",message.payload) == 0) return 0;
	return 5;
}

int countFiles(DIR* dirStream){
	struct dirent* dirContents = readdir(dirStream);
	int counter = 0;
	while(dirContents){
		counter++;
		dirContents = readdir(dirStream);
	}
	return counter;
}

int main(void)
{
	msg r;
	int i, res, num_files;
	char com[3], *name;
	printf("[RECEIVER] Starting.\n");
	init(HOST, PORT);
	printf("Check\n");
	FILE* fileStream = fopen("conf_out", "w");
	for (i = 0; i < COUNT; i++) {
		memset(&r, 0, sizeof(msg));
		/* recieve a command */
		res = recv_message(&r);
		if(res < 0){
			perror("[SERVER] Receive error. Exiting.\n");
			return -1;
		}

		/* find out what command has been received */
		int ch = parseMsg(r);
		switch(ch){

			case 11:

				/* send ACK for receiving the command */
				res = send_message(&r);
				if(res < 0){
					perror("[SERVER] Sending ACK error. Exiting \n");
					return -1;
				}				

				printf("list files");

				memcpy(name, r.payload + 3, sizeof(r.payload) - 3);
				if(strcmp(name,".") == 0){
					getcwd(name, 255 * sizeof(char));
				}
				printf("\nNameStringTest: %s\n", name);

				DIR* dirStream2 = opendir(name);
				if(dirStream2 == NULL){
					printf("Directory open error!");
				}
				num_files = countFiles(dirStream2);
				r.len = sizeof(int); // QQQQQQQQ len este numarul de octeti?

				memcpy(r.payload, &num_files, r.len);
				int test;
				memcpy(&test, r.payload, r.len);
				printf("ls: %i, should be: %i [%i]", test, num_files, r.len);
				/* send number of files */
				res = send_message(&r);
				if (res < 0) {
					perror("[RECEIVER] Send number of files error. Exiting.\n");
					return -1;
				}

				res = recv_message(&r);
				if (res < 0) {
					perror("[RECEIVER] Send number of files error. Exiting.\n");
					return -1;
				}

				int aux = num_files;
				struct dirent* dirContents2 = readdir(dirStream2);
				while(aux < num_files){
					r.len = sizeof(dirContents2->d_name);
					memcpy(r.payload, dirContents2->d_name, r.len);
					/* send file name */
					res = send_message(&r);
					if (res < 0) {
						perror("[RECEIVER] Send file name error. Exiting.\n");
						return -1;
					}
					/* wait for ACK */
					res = recv_message(&r);
					if (res < 0) {
						perror("[SERVER] Receiving ACK error. Exiting.\n");
						return -1;
					}
					dirContents2 = readdir(dirStream2);
					aux++;
				}

				closedir(dirStream2);
			break;

			case 1: // received ls command

				/* send ACK for receiving the command */
				res = send_message(&r);
				if(res < 0){
					perror("[SERVER] Sending ACK error. Exiting \n");
					return -1;
				}

				printf("list files");
				strcpy(com, "ls");
				name = malloc(strlen(r.payload));
				memcpy(name, r.payload + 2, sizeof(r.payload) - 2);

				/* count number of files */
				DIR* dirStream = opendir(name);
				num_files = countFiles(dirStream);
			
				r.len = sizeof(int); // QQQQQQQQ len este numarul de octeti?
				memcpy(r.payload, &num_files, sizeof(int));

				/* send number of files */
				res = send_message(&r);
				if (res < 0) {
					perror("[RECEIVER] Send number of files error. Exiting.\n");
					return -1;
				}
				/* wait for ACK */
				res = recv_message(&r);
				if (res < 0) {
					perror("[SERVER] Receiving ACK error. Exiting.\n");
					return -1;
				}
				fprintf(fileStream, "%s", "testin");
				/* itereate through files, send them one by one and wait for ACK after each send*/
				struct dirent* dirContents = readdir(dirStream);
				while(dirContents){
					printf("%s ", dirContents->d_name);
					r.len = sizeof(dirContents->d_name);
					memcpy(r.payload, dirContents->d_name, r.len);
					fprintf(fileStream, "%s", r.payload);
					/* send file name */
					res = send_message(&r);
					if (res < 0) {
						perror("[RECEIVER] Send file name error. Exiting.\n");
						return -1;
					}
					/* wait for ACK */
					res = recv_message(&r);
					if (res < 0) {
						perror("[SERVER] Receiving ACK error. Exiting.\n");
						return -1;
					}
					dirContents = readdir(dirStream);
				}
				// free(name);
				closedir(dirStream);
				break;


			case 2:
				/* send ACK for receiving the command */
				res = send_message(&r);
				if(res < 0){
					perror("[SERVER] Sending ACK error. Exiting \n");
					return -1;
				}	
				printf("[change directory]");			
				// name = malloc(strlen(r.payload));
				name = malloc(255);
				memcpy(name, r.payload + 3, r.len);
				if(chdir(name) != 0) printf("change directory error: \n%i\t", errno);
				printf("Test: %s", getcwd(name, 255 * sizeof(char)));
				printf("\n\n");
				free(name);		
				break;

			case 3:
				printf("copy");
				res = send_message(&r);
				if (res < 0) {
					perror("[SENDER] Send error. Exiting.\n");
					return -1;
				}
				break;

			case 4:
				printf("write new file");
				res = send_message(&r);
				if (res < 0) {
					perror("[SENDER] Send error. Exiting.\n");
					return -1;
				}
				break;

			case 5:
				printf("Error");
				res = send_message(&r);
				if (res < 0) {
					perror("[SENDER] Send error. Exiting.\n");
					return -1;
				}
				break;

			case 0:
				printf("Exit\n");
				
				res = send_message(&r);
				if (res < 0) {
					perror("[SENDER] Send error. Exiting.\n");
					return -1;
				}
				return 0;
				break;
			default:
				
				res = send_message(&r);
				if (res < 0) {
					perror("[SENDER] Send error. Exiting.\n");
					return -1;
				}
				printf("nope");
				break;
		}
	}
	fclose(fileStream);
	printf("[RECEIVER] Finished receiving..\n");
	return 0;
}
