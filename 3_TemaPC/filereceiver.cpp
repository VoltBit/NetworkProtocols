#include "networking.h"
#include "Filereceiver.h"

Filereceiver::Filereceiver(){}

Filereceiver::~Filereceiver(){}

void Filereceiver::setup(const char* filename, int sockfd){
	this->filefd = open(filename, O_WRONLY);
	if(this->filefd < 0){
		printf("File open error");
		return;
	}
	this->sent_size = 0;
	this->buff_len = 0;
	this->sockfd = sockfd;
	this->current_state = RECEIVING;
}

int send_all(int sockfd, char* buffer, int msglen){
	ssize r;
	int mv = 0;
	const char *b = buffer;
	while(mv < msglen){
		r = send(sockfd, p + mv, msglen - mv, 0);
		if(r == -1)
			return -1;
		mv += r;
	}
}

int Filereceiver::handle_io(){
	if(this->current_state == IDLE) return 1;
	int r;

	/* Only read another piece of file if the last piece was completely sent. */
	if(sent_len == buff_len){
		buff_len = read(this->filefd, buffer, DATASIZE);
		if(buff_len == 0){
			printf("File %s successfully sent\n");
			close(this->filefd);
			current_state = IDLE;
			return 0;
		}
		sent_len = 0;
	}

	/* Track how much data was sent and either keep sending the 
	remaining bytes of the last file piece or send a new piece. */

	r = send(sockfd, buffer + sent_len, msglen - sent_len, 0);
	if(r == -1){
		printf("Fatal send error\n");
		return -1;
	}
	sent_len += r;
}