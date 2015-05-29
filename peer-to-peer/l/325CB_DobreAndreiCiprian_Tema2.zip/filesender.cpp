#include "filesender.h"
#include "networking.h"
Filesender::Filesender(){}

Filesender::~Filesender(){}

int Filesender::setup(const char* filename, int sockfd){
	printf("Setting up socket: %i | file: %s\n", sockfd, filename);
	this->filefd = open(filename, O_RDONLY);
	printf("Opened file, filefd: %i\n", this->filefd);
	if(this->filefd < 0){
		printf("File open error\n");
		return -5;
	}
	this->sent_len = 0;
	this->buff_len = 0;
	this->sockfd = sockfd;
	this->current_state = SENDING;
	return 0;
}

/*  Function used on top of send to make sure all the data in the buffer
	was sent. It handles the case when send returns a value diferent from
	buffer size. Not actually used in the process but makes handle_io() 
	easier to understand. */

int send_all(int sockfd, char* buffer, int msglen){
	ssize_t r;
	int mv = 0;
	char *b = buffer;
	while(mv < msglen){
		printf("[*]");
		r = send(sockfd, b + mv, msglen - mv, 0);
		if(r == -1)
			return -1;
		mv += r;
	}
	return 0;
}

// int Filesender::handle_io(){
// 	printf("Sending piece of file\n");
// 	if(this->current_state == IDLE) return 0;
// 	int r;
// 	printf("dodos");
// 	/* Only read another piece of file if the last piece was completely sent. */

// 	buff_len = read(this->filefd, buffer, DATASIZE);
// 	printf("read from %i [%i]\n", this->filefd, buff_len);
// 	if(buff_len == 0){
// 		printf("File %i successfully sent\n", this->filefd);
// 		close(this->filefd);
// 		current_state = IDLE;
// 		return 0;
// 	}

	/* Track how much data was sent and either keep sending the 
// 	remaining bytes of the last file piece or send a new piece. */
// 	if(send_all(sockfd, buffer, buff_len) != 0){
// 		printf("Fatal send error while sending file %i\n", filefd);
// 		return -1;
// 	}
// 	return 0;
// }

int Filesender::handle_io(){
	/* In case of a closed socket the program should not crush when tring to send. */
	signal(SIGPIPE, SIG_IGN);

	printf("Sending piece of file\n");
	if(this->current_state == IDLE) return 0;
	int r = 0;
	/* Only read another piece of file if the last piece was completely sent. */
	if(sent_len == buff_len){
		buff_len = read(this->filefd, buffer, DATASIZE - 1);
		printf("read from %i %s[%i]\n", this->filefd, buffer, buff_len);
		if(buff_len == 0){
			printf("File %i successfully sent\n", this->filefd);
			close(this->filefd);
			current_state = IDLE;
			return 1;
		}
		sent_len = 0;
	}
	/* Track how much data was sent and either keep sending the 
	remaining bytes of the last file piece or send a new piece. */
	// r = send(sockfd, buffer + sent_len, buff_len - sent_len, 0);
	r = send(sockfd, "Hello peer", strlen("Hello peer"), 0);
	if(r == -1){
		printf("Fatal send error\n");
		return -1;
	}
	sent_len += r;
	printf("Finished sending piece of file\n");
	return 0;
}