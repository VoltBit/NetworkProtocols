/*
*  	Protocoale de comunicatii: 
*  	Laborator 6: UDP
*	client mini-server de backup fisiere
*/
#include "helpers.h"

void usage(char*file)
{
	fprintf(stderr,"Usage: %s ip_server port_server file\n",file);
	exit(0);
}

/*
*	Utilizare: ./client ip_server port_server nume_fisier_trimis
*/
int main(int argc,char**argv)
{
	if (argc!=4)
		usage(argv[0]);
	
	int fd;
	struct sockaddr_in to_station;
	char buf[BUFLEN];


	/*Deschidere socket*/
	int decscriptor = socket(PF_INET, SOCK_DGRAM, 0);
	
	
	/* Deschidere fisier pentru citire */
	DIE((fd=open(argv[3],O_RDONLY))==-1,"open file");
	
	/*Setare struct sockaddr_in pentru a specifica unde trimit datele*/
	to_station.sin_family = AF_INET;
	to_station.sin_port = htons(atoi(argv[2]));
	to_station.sin_addr = inet_addr(argv[1]);

	
	/*
	*  cat_timp  mai_pot_citi
	*		citeste din fisier
	*		trimite pe socket
	*/	
	char buffer[100];
	while(!fd.eof()){
		fread(buffer, 100, 1, fd);
		sendto(descriptor, buffer, 100, 0, (sockaddr_in)to_station);
	}

	/*Inchidere socket*/
	close(descriptor);
	
	/*Inchidere fisier*/
	fclose(fd);
	return 0;
}