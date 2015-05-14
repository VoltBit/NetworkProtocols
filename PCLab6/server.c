/*
*  	Protocoale de comunicatii: 
*  	Laborator 6: UDP
*	mini-server de backup fisiere
*/

#include "helpers.h"


void usage(char*file)
{
	fprintf(stderr,"Usage: %s server_port file\n",file);
	exit(0);
}

/*
*	Utilizare: ./server server_port nume_fisier
*/
int main(int argc,char**argv)
{
	if (argc!=3)
		usage(argv[0]);
	
	struct sockaddr_in my_sockaddr,from_station ;
	char buf[BUFLEN];


	/*Deschidere socket*/
	int decscriptor = socket(PF_INET, SOCK_DGRAM, 0);
	
	/*Setare struct sockaddr_in pentru a asculta pe portul respectiv */
	my_sockaddr.sin_family = AF_INET;
	my_sockaddr.sin_port = htons(atoi(argv[2]));
	my_sockaddr.sin_addr = inet_addr(argv[1]);
	
	/* Legare proprietati de socket */

	bind(descriptor, &my_sockaddr, sizeof(from_station));
	
	/* Deschidere fisier pentru scriere */
	DIE((fd=open(argv[2],O_WRONLY|O_CREAT|O_TRUNC,0644))==-1,"open file");
	
	/*
	*  cat_timp  mai_pot_citi
	*		citeste din socket
	*		pune in fisier
	*/
	char buffer[100];
	int test;
	while(test){
		test = (int)recvfrom(descriptor, buffer, 100, 0, (sockaddr_in)to_station);
		fwrite(buffer, 100, 1, fd);
	}

	/*Inchidere socket*/	
	close(descriptor);
	
	/*Inchidere fisier*/
	fclose(fd);
	return 0;
}
