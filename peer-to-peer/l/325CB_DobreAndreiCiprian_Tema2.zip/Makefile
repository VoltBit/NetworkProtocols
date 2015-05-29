build: client.cpp server.cpp networking.h
	gcc -o exq -Wall client.cpp server.cpp

build-server: server.cpp networking.h filesender.cpp filesender.h
	g++ -Wall -fpermissive -o server server.cpp filesender.cpp

build-client: client.cpp networking.h filesender.cpp filesender.h
	g++ -Wall -fpermissive -o client client.cpp filesender.cpp

run-server: build-server
	# valgrind --leak-check=full -v ./server 12345
	./server 12345

run-client: build-client
	# valgrind --leak-check=full -v ./client client1 share1 12500 127.0.0.1 12345
	./client client1 share1 12500 127.0.0.1 12345

clean:
	rm -f server
	rm -f client