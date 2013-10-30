all: serwer

serwer: main.o ./chat.o ./ipc_shared.o ./sockets.o ./tools.o ./websocket.o 
	gcc main.o ./chat.o ./ipc_shared.o ./sockets.o ./tools.o ./websocket.o  -o serwer -lcrypto -Wall

main.o: main.c sockets.h websocket.h tools.h ipc_shared.h chat.h
	gcc -c main.c -o main.o -lcrypto -Wall

chat.o: chat.c sockets.h websocket.h tools.h ipc_shared.h
	gcc -c chat.c -o chat.o

ipc_shared.o: ipc_shared.c ipc_shared.h
	gcc -c ipc_shared.c -o ipc_shared.o

sockets.o: sockets.c sockets.h
	gcc -c sockets.c -o sockets.o

tools.o: tools.c tools.h
	gcc -c tools.c -o tools.o

websocket.o: websocket.c sockets.h websocket.h tools.h
	gcc -c websocket.c -o websocket.o

clean: 
	rm *.o
	rm serwer
