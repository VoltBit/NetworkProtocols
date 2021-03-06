// Nume: 
// Grupa: 32x CB
// Program laborator 2 PC: emitator protocol start-stop
// Transmite fisierul de intrare catre receptor, folosind simulatorul mediului ideal de comunicatie

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "lib.h"

#define HOST "127.0.0.1"
#define PORT 10000

char checkParity(frame fr){
  int j;
  char i = fr.type ^ fr.data[0];
  for(j = 1; j < strlen(fr.data); j++){
    i ^= fr.data[j];
  }
  return i;
}

int main(int argc,char** argv){
  msg t, r;
  int fd_in, nr, i=0;

  init(HOST,PORT);

  // validare argumente linia de comanda
  if (argc != 2) {
       printf ("\nWrong number of parameters.\n");
       printf ("Usage:\n\t%s <input_file>\n", argv[0]);
       return 1;
    }
     
   // deschid fisierul de intrare
   fd_in = open(argv[1], O_RDONLY);
   if (fd_in < 0){
      perror("Error opening input file.\n");
      return 2;
   }
  
  //construiesc frame-ul

   frame ft;

   ft.type = FNAME;
   sprintf(ft.data,"%s",argv[1]);
   ft.parity = checkParity(ft);

  // trimitere nume fisier
  memcpy(t.payload, &ft, 1400);
  t.len = strlen(t.payload)+1;
  send_message(&t);
  printf("[%s] Sent file name %s (payload is %s)\n", argv[0], argv[1], t.payload);

  // tratare confirmare
  if (recv_message(&r)<0) { // grav
    perror("Receive error");
    return 3;
  }

  if (r.payload[0] != 'A') {
    printf("[%s] Received wrong ACK (payload is %s), exiting.\n", argv[0], r.payload);
    return 4;
  }
  printf("[%s] Received ACK for filename.\n", argv[0]);

  // trimitere date din fisier, cu confirmare
  while ((nr = read(fd_in, ft.data, MAX_LEN - 1)) > 0) {
    
    t.len = nr;
    ft.type = FDATA;
    
    ft.parity = checkParity(ft);
    memcpy(t.payload, &ft, 1400);
    

    send_message(&t);
    printf("[%s] Sent message number %d\n", argv[0], ++i);

    // tratare confirmare
    if (recv_message(&r)<0) { // grav
      perror("Receive error");
      return 5;
    }

    //memcpy(t.payload, &ft, )
    if (r.payload[0] ) {
      printf("[%s] Received wrong ACK for data (payload is %s), exiting.\n", argv[0], r.payload);
      return 6;
    }
    printf("[%s] Received ACK for message %d.\n", argv[0], i);
  }

  // trimit mesaj de terminare
  t.payload[0] = 'E';
  t.len = 1;
  send_message(&t);
  printf("[%s]  Sent ending message.\n", argv[0]);

  // tratare confirmare
  if (recv_message(&r)<0) { // grav
    perror("Receive error");
    return 7;
  }
  if (r.payload[0] != 'A') {
    printf("[%s]  Received wrong ACK for ending message (payload is %s), exiting.\n", argv[0], r.payload);
    return 8;
  }
  printf("[%s]  Received ACK for ending message. Goodbye!\n", argv[0]);
  return 0;
}
