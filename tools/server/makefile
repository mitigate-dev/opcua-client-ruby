all: server

server: open62541.o
	g++ -I../../ext/opcua_client server.cpp open62541.o -o server

open62541.o:
	gcc -std=c99 -c ../../ext/opcua_client/open62541.c

clean:
	-rm *.o server
