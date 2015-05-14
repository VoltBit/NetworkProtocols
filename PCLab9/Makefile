all:
	gcc -g -Wall dns.c 

build: dns.c
	gcc -g -Wall -o exq dns.c 

run: build
	./exq

clean:
	rm -f exq