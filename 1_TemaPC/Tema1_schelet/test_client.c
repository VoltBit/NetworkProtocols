#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include "lib.h"

#define HOST "127.0.0.1"
#define PORT 10001

int main(void)
{
	msg r;
	int i, res;
	
	printf("[SENDER] Starting.\n");
	init(HOST, PORT);
	for (i = 0; i < COUNT; i++) {
		/* wait for message */
		res = send_message(&r);
		if (res < 0) {
			perror("[SENDER] Send error. Exiting.\n");
			return -1;
		}

		/* send dummy ACK */
		res = recv_message(&r);
		if (res < 0) {
			perror("[SENDER] Recieve ACK error. Exiting.\n");
			return -1;
		}
	}

	printf("[SENDER] Finished receiving..\n");
	return 0;
}
