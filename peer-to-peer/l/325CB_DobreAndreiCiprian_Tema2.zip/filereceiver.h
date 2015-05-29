#define DATASIZE 1024

typedef enum state{
	IDLE,
	RECEIVING
}State;

class Filereceiver{
	int filefd;
	char buffer[DATASIZE];
	int buff_len, sent_len;
	State current_state;
	public:
		Filesender();
		~Filesender();
		void setup(const char* filename, int sockfd);
		void set_state(State x);
		int handle_io();
};