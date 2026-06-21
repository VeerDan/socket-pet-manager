.PHONY: all client server clean

all: server client

server:
	./build_server.sh

client:
	./build_client.sh

clean:
	./clean.sh
