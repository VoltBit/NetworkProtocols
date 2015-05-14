#define DATASIZE 1024

typedef enum state{
	IDLE,
	SENDING
}State;

class Filesender{
	int filefd, sockfd;
	char buffer[DATASIZE];
	int buff_len, sent_len;
	State current_state;
	public:
		Filesender();
		~Filesender();
		int setup(const char* filename, int sockfd);
		int handle_io();
};