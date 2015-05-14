#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <math.h>
#include "lib.h"

#define HOST "127.0.0.1"
#define PORT 10001

/*  
	global variable which tells what kind of check is required:
	 -> 0 for no check (default)
	 -> 1 for parity check
	 -> 2 for Hamming check
*/
int mode = 0;
/* function used to determine the command received */
int parseMsg(msg message){
	if(mode != 1){
		if(message.payload[0] == 'l')return 1;
			else if(message.payload[0] == 'c' && message.payload[1] == 'd')return 2;
				else if(message.payload[0] == 'c' && message.payload[1] == 'p') return 3;
					else if(message.payload[0] == 's') return 4;
						else if(strcmp("exit exit",message.payload) == 0) return 0;
	}else if( mode == 1){
		if(message.payload[1] == 'l')return 1;
		else if(message.payload[1] == 'c' && message.payload[2] == 'd')return 2;
			else if(message.payload[1] == 'c' && message.payload[2] == 'p') return 3;
				else if(message.payload[1] == 's') return 4;
					else if(strcmp("exit exit",message.payload + 1) == 0) return 0;
	}
	return 5;
}
/* counts number of files from a give directory */
int countFiles(DIR* dirStream){
	struct dirent* dirContents = readdir(dirStream);
	int counter = 0;
	while(dirContents){
		counter++;
		dirContents = readdir(dirStream);
	}
	return counter;
}
/* receives name of a file and returns its size */
long getSizeOfFile(char* name){
	FILE* fileStream = fopen(name, "r");
	fseek(fileStream, 0L, SEEK_END);
	int aux = ftell(fileStream);
	fclose(fileStream);
	return aux;
}
/* converter of integers to character string for proper message format */
void intToString(int x, char* string){
	int j, i = 0;
	while(x > 0){
		string[i] = x % 10 + 48;	
		i++;
		x /= 10;
	}
	char aux;
	for(j = 0; j < i / 2; j++){
		aux = string[j];
		string[j] = string[i - j - 1];
		string[i - j - 1] = aux;
	}
	string[i] = '\0';
}
/* converter of longs to character string for proper message format */
void longToString(long x, char* string){
	int j, i = 0;
	while(x > 0){
		string[i] = x % 10 + 48;	
		i++;
		x /= 10;
	}
	char aux;
	for(j = 0; j < i / 2; j++){
		aux = string[j];
		string[j] = string[i - j - 1];
		string[i - j - 1] = aux;
	}
	string[i] = '\0';
}
/* receives a string content and its size and returns a char representing its parity (0 or 1) */
char getParity(char* package, size_t size){
	char parity = 0;
	int i, n = (int) size, count = 0;
	for(i = 0; i < n; i++){
		count = 0;
		int b;
		for(b = 0x80; b && count < 1400; b >>= 1){ /* 0x80 = 128 */
			if((package[i] & b) != 0){
				parity = !parity;
			} 
			count++;
		}
		if(count > 1400){
			printf("Overflow!!!");
			return 2;
		}
	}
	return parity;
}

/* Creates a message containing the "ACK" string and sends it */
int sendACK(){
	msg message;
	memcpy(message.payload, "ACK", 3);
	message.payload[3] = '\0';
	message.len = 4;
	int res = send_message(&message);
	if(res < 0){
		perror("ACK send error");
		return -1;
	}
	return 0;
}
/* Creates a message containing the "NACK" string and sends it */
int sendNACK(){
	msg message;
	memcpy(message.payload, "NACK", 4);
	message.payload[4] = '\0';
	message.len = 5;
	int res = send_message(&message);
	if(res < 0){
		perror("ACK send error");
		return -1;
	}
	return 0;
}
/* Function that generates Hamming code for a give byte. The encoding is made using two bytes.
	- a: initial byte or information
	- byte1: first byte of the encoding
	- byte2: second byte of the encoding
*/
void hammingGen(char a, char* byte1, char* byte2){
	char b1, b2;
	/* 
		Configuratie
		b1: [ x x x x _ _ 7 _ ] b2: [ 6 5 4 _ 3 2 1 0 ] 
		Sume biti de control:
		p1: 7 6 4 3 1
		p2: 7 5 4 2 1
		p3: 
		p1: (3 1)b1 + (7 5 3 1)b2
		p2: (2 1)b1 + (6 5 2 1)b2
		p3: (0)b1 + (7 6 5 0)b2
		p4: (3 2 1 0)b2

		Pozitiile in byte ale bitilor de control:
		p1 -> b1[3]
		p2 -> b1[2]
		p3 -> b1[0]
		p4 -> b2[4]
	*/
	b1 = 0x00;
	b2 = 0x00;
	b1 |= ((a >> 7) & 1) << 1;

	b2 |= ((a >> 0) & 1) << 0;
	b2 |= ((a >> 1) & 1) << 1;
	b2 |= ((a >> 2) & 1) << 2;
	b2 |= ((a >> 3) & 1) << 3;
	b2 |= ((a >> 4) & 1) << 5;
	b2 |= ((a >> 5) & 1) << 6;
	b2 |= ((a >> 6) & 1) << 7;
	/* sum p1 */
	int p1, p2, p3, p4;
	/* calculate the check bits */
	p1 = ((b1 >> 1) & 1) + ((b2 >> 7) & 1) + ((b2 >> 5) & 1) + ((b2 >> 3) & 1) + ((b2 >> 1) & 1);
	p2 = ((b1 >> 1) & 1) + ((b2 >> 6) & 1) + ((b2 >> 5) & 1) + ((b2 >> 2) & 1) + ((b2 >> 1) & 1);
	p3 = ((b2 >> 7) & 1) + ((b2 >> 6) & 1) + ((b2 >> 5) & 1) + ((b2 >> 0) & 1);
	p4 = ((b2 >> 3) & 1) + ((b2 >> 2) & 1) + ((b2 >> 1) & 1) + ((b2 >> 0) & 1);
	/* insert check bits into the two bytes */
	b1 |= (p1 % 2) << 3;
	b1 |= (p2 % 2) << 2;
	b1 |= (p3 % 2) << 0;
	b2 |= (p4 % 2) << 4;
	*byte1 = b1;
	*byte2 = b2;
}
/* Function correcting a given Hamming code. The code is received through its two encoding bytes. */
void hammingCorrector(char* byte1, char* byte2){
	char b1 = *byte1, b2 = *byte2;
	int p1, p2, p3, p4, index = 0;
	/* calculate the check bits */
	p1 = ((b1 >> 1) & 1) + ((b2 >> 7) & 1) + ((b2 >> 5) & 1) + ((b2 >> 3) & 1) + ((b2 >> 1) & 1);
	p2 = ((b1 >> 1) & 1) + ((b2 >> 6) & 1) + ((b2 >> 5) & 1) + ((b2 >> 2) & 1) + ((b2 >> 1) & 1);
	p3 = ((b2 >> 7) & 1) + ((b2 >> 6) & 1) + ((b2 >> 5) & 1) + ((b2 >> 0) & 1);
	p4 = ((b2 >> 3) & 1) + ((b2 >> 2) & 1) + ((b2 >> 1) & 1) + ((b2 >> 0) & 1);
	p1 %= 2;
	p2 %= 2;
	p3 %= 2;
	p4 %= 2;
	if(((b1 >> 3) & 1) != p1) index += 1;
	if(((b1 >> 2) & 1) != p2) index += 2;
	if(((b1 >> 0) & 1) != p3) index += 4;
	if(((b2 >> 4) & 1) != p4) index += 8;

	if(index != 0){
		/* correct parity bits if one of them was changed */
		if(index == 1) b1 ^= 1 << 3;
			else if(index == 2) b1 ^= 1 << 2;
				else if(index == 4) b1 ^= 1 << 0;
					else if(index == 8) b2 ^= 1 >> 4;

		/* or correct the bit found at index */
		if(index == 3){ /* correct the only bit left unchecked in b1 */
			b1 ^= 1 << 1;
		} else {
			b2 ^= 1 << (12 - index); /* correct bit from b2 */
		}
		*byte1 = b1;
		*byte2 = b2;
	}
	else return;
}
/* receives a Hamming code and returns a char representing the decoded value from the code */
char hammingDecoder(char b1, char b2){
	char dec;
	dec = 0x00;
	dec |= ((b1 >> 1) & 1) << 7;

	dec |= ((b2 >> 0) & 1) << 0;
	dec |= ((b2 >> 1) & 1) << 1;
	dec |= ((b2 >> 2) & 1) << 2;
	dec |= ((b2 >> 3) & 1) << 3;
	dec |= ((b2 >> 5) & 1) << 4;
	dec |= ((b2 >> 6) & 1) << 5;
	dec |= ((b2 >> 7) & 1) << 6;
	return dec;
}
/* receives an array of chars and return the corresponding Hamming code */
void hammingEncode(char* content, size_t size, char* buffer){
	int i, j = 0;
	for(i = 0; i < size; i++, j += 2){
		hammingGen(content[i], &buffer[j], &buffer[j + 1]);
	}
}
/* receives a message and converts both the payload and the length such that it contains the decoded information instead of the Hamming code */
void hammingProcess(msg* message){
	int i, j = 0;
	char buffer[701];
	for(i = 0; i < message->len; i += 2, j++){
		hammingCorrector(&message->payload[i], &message->payload[i + 1]);
		buffer[j] = hammingDecoder(message->payload[i], message->payload[i + 1]);
	}
	message->len = message->len / 2;
	memcpy(message->payload, buffer, message->len);
	message->payload[message->len] = '\0';
}
/* Overlay function for the basic rec_message provided that adds functionality for parity check and also sends ACK / NACK messages.*/
int receiveMessage(msg* message){
	int ok = 0;
	switch(mode){
		case 0:
		case 2:
			if(recv_message(message) < 0){
				perror("Receive error. Exiting.\n");
				return -1;
			}
			/* send ACK */
			if(sendACK() < 0) return -1;
			if(mode == 2) hammingProcess(message);
		break;
		case 1:
			/* receives a messege, checks its parity and if it is wrong then sends a NACK and asks for a resend*/
			do{
				ok = 1;
				if(recv_message(message) < 0){
					perror("Receive error. Exiting.\n");
					return -1;
				}
				if(message->payload[0] != getParity(message->payload + 1, message->len - 1)){
					ok = 0;
					/* send NACK */
					if(sendNACK() < 0) return -1;
				}
			}while(!ok);
			/* send ACK */
			if(sendACK() < 0) return -1;
		break;
	}
	return 0;
}

/* For parity checks it tries to send a message until receives "ACK" or reaches 500 attempts. 
	- content: the information received as string that is to be send
	- size: the size of content string
*/
int sendMessage(char* content, size_t size){
	int count = 0, max_attempts = 500;
	msg message, confirmation;
	/* create the package accordingly with the mode */
	if(mode == 0){
		message.len = (int) size;
		memcpy(message.payload, content, message.len);
		message.payload[message.len] = '\0';
	}
	else if(mode == 1){
		message.len = (int) size + 1;
		memcpy(message.payload + 1, content, message.len - 1);
		message.payload[0] = getParity(content, size);
		message.payload[message.len] = '\0';
	}
	else{
		char buffer[1401];
		message.len = (int) size * 2;
		hammingEncode(content, size, buffer);
		memcpy(message.payload, buffer, message.len);
	}
	do{
		if(send_message(&message) < 0){
			perror("Sending error. Exiting.\n");
			return -1;
		}
		if(recv_message(&confirmation) < 0){
			perror("Receive error. Exiting.\n");
			return -1;
		}
		count++; /* the counter makes sure the loop will end and exists with an error of timeout */
	}while(confirmation.len == 5 && count < max_attempts);
	if(count == max_attempts){
		printf("Message sending timed out. Exiting\n");
		return -1;
	}
	return 0;
}

int ls(char* name){
	int num_files = 0;
	char converted[255];
	/* count number of files */
	DIR* dirStream = opendir(name);
	if(dirStream == NULL){
		printf("Directory open error!\n");
		return -1;
	}
	num_files = countFiles(dirStream);
	intToString(num_files, converted);
	/* send number of files */
	if(sendMessage(converted, strlen(converted)) < 0) return -1;
	/* itereate through files, send them one by one and wait for ACK after each send*/
	dirStream = opendir(name);
	struct dirent* dirContents = readdir(dirStream);
	if(dirContents == NULL)printf(">>>Read directory error!<<<");
	int aux = 0;
	while(aux < num_files){
		aux++;
		/* send file name */
		if(sendMessage(dirContents->d_name, sizeof(dirContents->d_name)) < 0)return -1;
		dirContents = readdir(dirStream);
	}
	closedir(dirStream);
	return 0;
}

int cp(char* name){
	int bufferSize = 0;
	/* different buffer size according to the maximum usable ammount for each of the three modes */
	if(mode == 0) bufferSize = 1400;
	else if(mode == 1) bufferSize = 1399;
	else if(mode == 2) bufferSize = 700;

	char buffer[1401], converted[255];
	int readFileDescriptor = open(name, O_RDONLY);
	long fileSize = getSizeOfFile(name);
	longToString(fileSize, converted);
	/* send size of file */
	if(sendMessage(converted, strlen(converted)) < 0) return -1;
	int aux = read(readFileDescriptor, buffer, bufferSize);
	while(aux != 0){
		/* send piece of file */
		if(sendMessage(buffer, aux) < 0) return -1;
		aux = read(readFileDescriptor, buffer, aux);
	}
	close(readFileDescriptor);
	return 0;
}

int sn(char* name){
	msg r;
	int fileSize;
	char newName[1405] = "new_";
	strcat(newName, name); /* prepare the new name */
	int writeFileDescriptor = open(newName, O_CREAT|O_WRONLY|O_TRUNC, 0766);

	/* Receive file size. */
	if(receiveMessage(&r) < 0) return -1;
	if(mode == 0 || mode == 2)
		fileSize = atoi(r.payload);
	else if(mode == 1)
		fileSize = atoi(r.payload + 1);

	long counter = 0;
	while(counter < fileSize){
		/* Receive file piece of file. */
		if(receiveMessage(&r) < 0) return -1;
		if(mode == 0 || mode == 2){
			write(writeFileDescriptor, r.payload, r.len);
			counter += r.len;
		} else if(mode == 1){
			write(writeFileDescriptor, r.payload + 1, r.len - 1);
			counter += r.len - 1;		
		}
	}
	return 0;
}

int main(int argc, char* argv[])
{
	msg r;
	int i;
	char name[1401];
	printf("[RECEIVER] Starting.\n");
	init(HOST, PORT);

	if(argc == 2){
		if(strcmp(argv[1], "parity") == 0) mode = 1;
		else if(strcmp(argv[1], "hamming") == 0) mode = 2;
		else{
			printf("Check mode error - wrong arguments. Exiting.");
			return -1;
		}
	}else mode = 0;

	for (i = 0; i < COUNT; i++) {
		memset(&r, 0, sizeof(msg));

		/* recieve a command */
		if(receiveMessage(&r) < 0) return -1;
		/* find out what command has been received */
		int ch = parseMsg(r);
		switch(ch){
			case 1:
				/* list files */
				if(mode == 0 || mode == 2) memcpy(name, r.payload + 3,r.len);
				else if(mode == 1) memcpy(name, r.payload + 4, r.len);
				if(strcmp(name,".") == 0){
					getcwd(name, 255 * sizeof(char));
				}
				ls(name);
				break;

			case 2:
				/* change directory */
				if(mode == 0 || mode == 2)memcpy(name, r.payload + 3, r.len);
				else if(mode == 1)memcpy(name, r.payload + 4, r.len);
				if(chdir(name) != 0) printf("change directory error: \n%i\t", errno);
				break;

			case 3:
				/* copy file */
				if(mode == 0 || mode == 2) cp(r.payload + 3);
				else if(mode == 1) cp(r.payload + 4);
				break;

			case 4:
				/* write new file */
				if(mode == 0 || mode == 2) sn(r.payload + 3);
				else if(mode == 1) sn(r.payload + 4);				
				break;

			case 5:
				printf("Command error. Exiting.\n");
				return 0;

			case 0:
				if(sendACK() < 0) return -1;
				printf("\nExit\n");
				return 0;
		}
	}

	printf("[RECEIVER] Finished receiving..\n");
	return 0;
}