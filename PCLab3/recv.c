// Nume: 
// Grupa: 32x CB
// Program laborator 2 PC: receptor protocol start-stop
// Receptioneaza fisierul de intrare trimis de emitator, folosind simulatorul mediului ideal de comunicatie

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "lib.h"

#define HOST "127.0.0.1"
#define PORT 10001

char checkParity(frame fr){
  int j;
  char i = fr.type ^ fr.data[0];
  for(j = 1; j < strlen(fr.data); j++){
    i ^= fr.data[j];
  }
  return i;
}

int main(int argc,char** argv){
  msg r,t;
  int fd_out, i=0;
  frame fr;

  init(HOST,PORT);

  // receptie nume fisier
  if (recv_message(&r)<0) { // grav
    perror("Receive error");
    return 1;
  }
  memcpy(t.payload, &fr, 1400);
  char aux = checkParity(fr);
  if (aux != fr.parity) {
    printf("[%s] Received corrupt message (payload is %s), exiting. <<<<<<\n", argv[0], r.payload);
    return 2;
  }

  // deschidere fisier de iesire
  fd_out = open(r.payload, O_WRONLY | O_CREAT, 0644); // nu r.payload+1 pentru a avea nume diferite
  if (fd_out < 0){
    perror("Error opening output file, exiting.\n");
    t.payload[0] = 'X';
    t.len = 1;
    send_message(&t);
    printf("[%s] Sent exit message to sender.\n", argv[0]);
    return 3;
  }
  
  // trimitere confirmare
  t.payload[0] = 'A';
  t.len = 1;
  send_message(&t);
  printf("[%s] Sent ACK for file name (payload is %s)\n", argv[0], r.payload);

  // receptie date, cu confirmare
  while (1) {
    if (recv_message(&r)<0) { // grav
      perror("Receive error");
      return 4;
    }
    
    memcpy(t.payload, &fr, 1400);
    aux = checkParity(fr);

    if (aux != fr.parity) {
      printf("[%s] Received currupt data, exiting.\n", argv[0]);
      t.len = 1;
      send_message(&t);
      printf("[%s] Sent exit message to sender.\n", argv[0]);
      return 5;
    }
    // scriere date in fisier
    write (fd_out, r.payload + 1, r.len - 1);
  
    // trimitere confirmare
    fr.type = ACK;
    fr.data = 0;
    fr.parity = checkParity(fr);

    memcpy(t.payload, &fr, 1400);
    send_message(&t);
    printf("[%s] Sent ACK for message %d.\n", argv[0], ++i);
    
    if (r.len != MAX_LEN)
      break;
  }

  if (recv_message(&r)<0) { // grav
    perror("Receive error");
    return 6;
  }

  memcpy(fr, r.payload, 1400);

  if (fr.parity == checkParity(fr)) {
    printf("[%s]  Received corrupt data, exiting.\n", argv[0]);

    fr.data = 0;
    fr.type = NACK;
    fr.parity = checkParity(fr);
    memcpy(t.payload, fr, 1400);
    
    send_message(&t);

    printf("[%s]  Sent exit message to sender.\n", argv[0]);

    return 7;
  }
  // trimitere confirmare
  fr.data = 0;
  fr.type = ACK;
  fr.parity = checkParity(fr);
  memcpy(fr,t.payload,1400);
  send_message(&t);
  printf("[%s]  Sent ACK for ending message. Goodbye!\n", argv[0]);
  return 0;
}
