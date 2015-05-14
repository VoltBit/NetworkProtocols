#include <sys/socket.h> 
#include <netinet/in.h>
#include <arpa/inet.h>  
#include <unistd.h>     
#include <sys/types.h>  

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#define SMTP_PORT 25
#define MAXLEN 500

/**
 * Citeste maxim maxlen octeti din socket-ul sockfd. Intoarce
 * numarul de octeti cititi.
 */
ssize_t Readline(int sockd, void *vptr, size_t maxlen) {
    ssize_t n, rc;
    char    c, *buffer;

    buffer = vptr;

    for ( n = 1; n < maxlen; n++ ) {	
	if ( (rc = read(sockd, &c, 1)) == 1 ) {
	    *buffer++ = c;
	    if ( c == '\n' )
		break;
	}
	else if ( rc == 0 ) {
	    if ( n == 1 )
		return 0;
	    else
		break;
	}
	else {
	    if ( errno == EINTR )
		continue;
	    return -1;
	}
    }

    *buffer = 0;
    return n;
}

/**
 * Trimite o comanda SMTP si asteapta raspuns de la server.
 * Comanda trebuie sa fie in buffer-ul sendbuf.
 * Sirul expected contine inceputul raspunsului pe care
 * trebuie sa-l trimita serverul in caz de succes (de ex. codul
 * 250). Daca raspunsul semnaleaza o eroare se iese din program.
 */
void send_command(int sockfd, char sendbuf[], char *expected) {
  char recvbuf[MAXLEN];
  int nbytes;
  char CRLF[3];
  
  CRLF[0] = 13; CRLF[1] = 10; CRLF[2] = 0;
  strcat(sendbuf, CRLF);
  printf("Trimit: %s", sendbuf);
  write(sockfd, sendbuf, strlen(sendbuf));
  nbytes = Readline(sockfd, recvbuf, MAXLEN - 1);
  recvbuf[nbytes] = 0;
  printf("Am primit: %s", recvbuf);
  if (strstr(recvbuf, expected) != recvbuf) {
    printf("Am primit mesaj de eorare de la server!\n");
    exit(-1);
  }
}

void get_msg(char* buffer){
  char tmp[500];
  do{
    // scanf("%s", tmp);
    gets(tmp);
    strcat(buffer, tmp);
    if(strcmp(tmp,".") != 0)
      strcat(buffer, "\n");
  }while(strcmp(tmp, "."));
}

int main(int argc, char **argv) {
  int sockfd;
  int port = SMTP_PORT;
  struct sockaddr_in servaddr;
  char server_ip[20];
  char sendbuf[MAXLEN]; 
  char recvbuf[MAXLEN];

  if (argc != 2) {
    printf("Utilizare: ./send_msg adresa_server");
    exit(-1);
  }
  strcpy(server_ip, argv[1]);

  if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
	printf("Eroare la  creare socket.\n");
	exit(-1);
  }  

  /* formarea adresei serverului */
  memset(&servaddr, 0, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(port);

  if (inet_aton(server_ip, &servaddr.sin_addr) <= 0 ) {
    printf("Adresa IP invalida.\n");
    exit(-1);
  }
    
  /*  conectare la server  */
  if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0 ) {
    printf("Eroare la conectare\n");
    exit(-1);
  }

  Readline(sockfd, recvbuf, MAXLEN -1);
  printf("Am primit: %s\n", recvbuf);

  sprintf(sendbuf, "HELO <adresa_ip_aici>");
  send_command(sockfd, sendbuf, "250");

  char tmp_command[1000], tmp_headers[255], msg_buffer[2500];
  while(1){
    printf("Expeditor: ");
    scanf("%s", tmp_headers);
    printf("Input received: %s\n", tmp_headers);
    strcpy(tmp_command, "MAIL FROM: ");
    strcat(tmp_command, tmp_headers);
    send_command(sockfd, tmp_command, "250");

    printf("Destinatar: ");
    scanf("%s", tmp_headers);
    printf("Input received: %s\n", tmp_headers);
    if(strcmp(tmp_headers, "RSET") == 0){
      printf("\n\nStart over: \n\n");
      send_command(sockfd, tmp_headers, "250");
      continue;
    }
    strcpy(tmp_command, "RCPT TO: ");
    strcat(tmp_command, tmp_headers);
    send_command(sockfd, tmp_command, "250");

    printf("Mesaj: \n");
    get_msg(msg_buffer);
    printf("Input received: %s\n", msg_buffer);
    strcpy(tmp_command, "DATA\n");
    send_command(sockfd, tmp_command, "354");
    send_command(sockfd, msg_buffer, "250");

    printf("Alt mesaj? [Y/n]");
    scanf("%s", tmp_headers);
    if(strcmp(tmp_headers, "n") == 0 || strcmp(tmp_headers, "N") == 0){
      strcpy(tmp_command, "QUIT");
      send_command(sockfd, tmp_command, "221");
      break;
    }
    else if(strcmp(tmp_headers, "y") || strcmp(tmp_headers, "Y") || 
      strcmp(tmp_headers, "\n")) continue;
      else {
        strcpy(tmp_command, "QUIT");
        send_command(sockfd, tmp_command, "221");
        break;
      } 
  }

  close(sockfd);
  return 0;
}
  
