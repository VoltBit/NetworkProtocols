#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void intToString(int x, char* string){
	int j, i = 0;
	while(x > 0){
		string[i] = x % 10 + 48;	
		i++;
		x /= 10;
	}
	char aux;
	for(j = 0; j < i / 2; j++){
		aux = string[j];
		string[j] = string[i - j - 1];
		string[i - j - 1] = aux;
	}
	string[i + 1] = '\0';
	printf("Test: %s\n", string);
}

void binDisplay(size_t x){
	unsigned int i = x;
	for(; x; x >>= 1){
		if(x & 1)
			printf("1");
		else
			printf("0");
	}
	printf("\n");
}

int bit_count(int byte)
    {
        int bit;
        int tally;
        tally = 0;
        for (bit = 0x80; bit > 0; bit = bit >> 1)
        {
            if ((byte & bit) != 0){
            		printf("1");
            		++tally;
            } else printf("0");
        } printf(" ");
        return (tally);
    }

void findParity(char* package){
	char parity = 0;
	int i, n = strlen(package);
	char aux;
	printf("\n size: %i\n", n);
	for(i = 0; i < n; i++){
		aux = package[i];
		while(aux){
			if(aux & 1){
				parity = !parity;
				printf("1");
			}
			else
				printf("0");
			aux >>= 1;
		}
		printf(" ");
	}
	printf("\n Parity: %i\n", parity);
}

void asciiDisp(){
	unsigned int i;
	for(i = 0; i < 255; i++){
		if(i % 10 == 0) printf("\n");
		printf("%c [%u]\t", i, i);
	}
}

/* 

Hamming: mesaje de maxim 700 biti;

un char => 2 char
        1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17
0 1 2 3 4 5 6 7 0 1 2 3 4  5  6  7  0  1  2  3  4
        p p d p d d d p d  d  d  d  d  d  d  p  d


*/

void hammingGen(char a, char* byte1, char* byte2){
	char b1, b2;
	int i;
	/* 
		Configuratie
		b1: [ x x x x _ _ 7 _ ] b2: [ 6 5 4 _ 3 2 1 0 ] 
		Sume biti de control:
		p1: 7 6 4 3 1
		p2: 7 5 4 2 1
		p3: 
		p1: (3 1)b1 + (7 5 3 1)b2
		p2: (2 1)b1 + (6 5 2 1)b2
		p3: (0)b1 + (7 6 5 0)b2
		p4: (3 2 1 0)b2

		Pozitiile in byte ale bitilor de control:
		p1 -> b1[3]
		p2 -> b1[2]
		p3 -> b1[0]
		p4 -> b2[4]
	*/
	b1 = 0x00;
	b2 = 0x00;
	b1 |= ((a >> 7) & 1) << 1;

	b2 |= ((a >> 0) & 1) << 0;
	b2 |= ((a >> 1) & 1) << 1;
	b2 |= ((a >> 2) & 1) << 2;
	b2 |= ((a >> 3) & 1) << 3;
	b2 |= ((a >> 4) & 1) << 5;
	b2 |= ((a >> 5) & 1) << 6;
	b2 |= ((a >> 6) & 1) << 7;
	/* sum p1 */
	int p1, p2, p3, p4;
	printf("b1: "); bit_count(b1); printf("\n");
	printf("b2: "); bit_count(b2); printf("\n");
	p1 = ((b1 >> 1) & 1) + ((b2 >> 7) & 1) + ((b2 >> 5) & 1) + ((b2 >> 3) & 1) + ((b2 >> 1) & 1);
	p2 = ((b1 >> 1) & 1) + ((b2 >> 6) & 1) + ((b2 >> 5) & 1) + ((b2 >> 2) & 1) + ((b2 >> 1) & 1);
	p3 = ((b2 >> 7) & 1) + ((b2 >> 6) & 1) + ((b2 >> 5) & 1) + ((b2 >> 0) & 1);
	p4 = ((b2 >> 3) & 1) + ((b2 >> 2) & 1) + ((b2 >> 1) & 1) + ((b2 >> 0) & 1);
	printf("P: %i %i %i %i", p1 % 2, p2 % 2, p3 % 2, p4 % 2);
	b1 |= (p1 % 2) << 3;
	b1 |= (p2 % 2) << 2;
	b1 |= (p3 % 2) << 0;
	b2 |= (p4 % 2) << 4;
	printf("\n");
	bit_count(b1);
	bit_count(b2);
	*byte1 = b1;
	*byte2 = b2;
}

void hammingCorrector(char* byte1, char* byte2){
	char b1 = *byte1, b2 = *byte2;
	int p1, p2, p3, p4, index = 0;

	p1 = ((b1 >> 1) & 1) + ((b2 >> 7) & 1) + ((b2 >> 5) & 1) + ((b2 >> 3) & 1) + ((b2 >> 1) & 1);
	p2 = ((b1 >> 1) & 1) + ((b2 >> 6) & 1) + ((b2 >> 5) & 1) + ((b2 >> 2) & 1) + ((b2 >> 1) & 1);
	p3 = ((b2 >> 7) & 1) + ((b2 >> 6) & 1) + ((b2 >> 5) & 1) + ((b2 >> 0) & 1);
	p4 = ((b2 >> 3) & 1) + ((b2 >> 2) & 1) + ((b2 >> 1) & 1) + ((b2 >> 0) & 1);
	printf("\nP: %i %i %i %i\n", p1 % 2, p2 % 2, p3 % 2, p4 % 2);
	p1 %= 2;
	p2 %= 2;
	p3 %= 2;
	p4 %= 2;
	bit_count(b1);
	printf("\n");
	int i;
	if(((b1 >> 3) & 1) != p1){
		printf("p1 changed ");
		index += 1;
	}
	if(((b1 >> 2) & 1) != p2){
		printf("p2 changed ");
		index += 2;
	}
	if(((b1 >> 0) & 1) != p3){
		printf("p3 changed ");
		index += 4;
	}
	if(((b2 >> 4) & 1) != p4){
		printf("p4 changed ");
		index += 8;
	}

	if(index != 0){
		printf("Correcting %i\n", index);
		/* correct parity bits if one of them was changed */
		if(index == 1) b1 ^= 1 << 3;
			else if(index == 2) b1 ^= 1 << 2;
				else if(index == 4) b1 ^= 1 << 0;
					else if(index == 8) b2 ^= 1 >> 4;

		/* or correct the bit found at index */
		if(index == 3){ /* correct the only bit left unchecked in b1 */
			b1 ^= 1 << 1;
		} else {
			b2 ^= 1 << (12 - index); /* correct bit from b2 */
		}
		*byte1 = b1;
		*byte2 = b2;
	}
	else return;
}

char hammingDecoder(char b1, char b2){
	char dec;
	dec = 0x00;
	dec |= ((b1 >> 1) & 1) << 7;

	dec |= ((b2 >> 0) & 1) << 0;
	dec |= ((b2 >> 1) & 1) << 1;
	dec |= ((b2 >> 2) & 1) << 2;
	dec |= ((b2 >> 3) & 1) << 3;
	dec |= ((b2 >> 5) & 1) << 4;
	dec |= ((b2 >> 6) & 1) << 5;
	dec |= ((b2 >> 7) & 1) << 6;
	bit_count(dec);
	return dec;
}

/*
	Calculate hamming code for string; check
	receive a message -> calculate hamming code for it -> compare calculated hamming code with the received one -> detect errors and correct
	-> decode message 
*/
int main(char* arg[]){
	char a = 100;
	char aux = a, b1,b2;
	/* "cd .":

	c : 00000001 11000011
	d : 00001000 11010100
	  : 00000101 01000000 
	. : 00001001 01110001

	*/
	printf("Initial data: "); bit_count(-100); printf("\n");
	hammingGen(-100, &b1, &b2);
	aux = -100;
	b2 ^= 1 << 3;
	printf("\nCorrupted: ");
	bit_count(b1); bit_count(b2);
	hammingCorrector(&b1, &b2);
	printf("\nCorrected: ");
	bit_count(b1); bit_count(b2);

	printf("\n\n Decoded: ");
	aux = hammingDecoder(b1, b2);
	printf("\nResult: %c", aux);
	printf("\n");
	return 0;
}