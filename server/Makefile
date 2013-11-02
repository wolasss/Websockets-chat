all: server

server: main.o ./chat.o ./ipc_shared.o ./sockets.o ./tools.o ./websocket.o 
	gcc main.o ./chat.o ./ipc_shared.o ./sockets.o ./tools.o ./websocket.o  -o server -pthread -g -lcrypto -Wall

main.o: main.c sockets.h websocket.h tools.h ipc_shared.h chat.h
	gcc -c main.c -o main.o -lcrypto -g -Wall -pthread

chat.o: chat.c chat.h sockets.h websocket.h tools.h ipc_shared.h
	gcc -c chat.c -o chat.o -g

ipc_shared.o: ipc_shared.c ipc_shared.h
	gcc -c ipc_shared.c -o ipc_shared.o -g

sockets.o: sockets.c sockets.h
	gcc -c sockets.c -o sockets.o -g

tools.o: tools.c tools.h
	gcc -c tools.c -o tools.o -g

websocket.o: websocket.c sockets.h websocket.h tools.h
	gcc -c websocket.c -o websocket.o -g

clean: 
	rm *.o
	rm server
