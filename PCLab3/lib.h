#define MAX_LEN 1400

#ifndef LIB
#define LIB
#define FNAME 1
#define FDATA 2
#define ACK 3
#define NACK 4

typedef struct {
  // int type;
  int len;
  char payload[MAX_LEN];
} msg;
typedef struct {
	char type;
	char parity;
	char data[1398];
}frame;
void init(char* remote,int remote_port);
void set_local_port(int port);
void set_remote(char* ip, int port);
int send_message(const msg* m);
int recv_message(msg* r);
//msg* receive_message_timeout(int timeout);

#endif


