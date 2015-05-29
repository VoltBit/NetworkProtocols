build: http_client.cpp networking.h page_node.cpp page_node.h sysops.cpp sysops.cpp
	g++ -std=gnu++11 -o myclient http_client.cpp page_node.cpp sysops.cpp -Wall

run: build
	./myclient -o -e http://www.cs.stir.ac.uk/~kjt/index.html
	rm -f myclient
