/*
a argument
s   string (łańcuch znaków)
sz  string (łańcuch znaków zakończony bajtem zerowym - null'em)
c   char (jeden znak), również const - wartość stała (szczególnie w przypadku użycia wskaźników)
by  byte, unsigned char
n   short
i   int
x, y    int (przy zmiennych określających współrzędne)
cx, cy  int (przy zmiennych określających rozmiar, długość)
l   long
w   word
dw  dword
b   boolean (wartość logiczna: prawda lub fałsz)
f   flaga
fn  funkcja
h   handle (uchwyt)
p   pointer (wskaźnik)
fd - deskryptor pliku (int)
aout - argument tylko do zapisania
*/
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <fcntl.h>
#include "sockets.h"
#include "websocket.h"
#include "tools.h"
#include "ipc_shared.h"
#include "chat.h"

#define BYTETOBINARYPATTERN "%d%d%d%d%d%d%d%d"
#define BYTETOBINARY(byte)  \
  (byte & 0x80 ? 1 : 0), \
  (byte & 0x40 ? 1 : 0), \
  (byte & 0x20 ? 1 : 0), \
  (byte & 0x10 ? 1 : 0), \
  (byte & 0x08 ? 1 : 0), \
  (byte & 0x04 ? 1 : 0), \
  (byte & 0x02 ? 1 : 0), \
  (byte & 0x01 ? 1 : 0) 

#define DEBUG 1

struct Shared * SHM = NULL;
int GLOBALsemid;

void forceQuit(int a_sig) {
    SHMdestroy();
}

void killChildProcess(int a_sig) {
    wait(NULL);
}

void logEvent(char* a_event) {
	char sz_buf[512]; 
	int fd_logfile;
	bzero(sz_buf, 512);
	snprintf(sz_buf, sizeof sz_buf, "%s - %s \n", timestamp(), a_event);
	IPCp(GLOBALsemid,2);
	fd_logfile = open("/tmp/chat.log", O_WRONLY|O_CREAT|O_APPEND, 0666);
	write(fd_logfile, sz_buf, 512);
	IPCv(GLOBALsemid,2);
}

void handleClient( int * a_soc ) {
    char *frame, *message;
    unsigned long messageLength = 0, frameLength = 0;
    while(1) {
        //bzero(message, messageLength); no need to do this cause message is freed.
        frame = SOCreceiveMessage(a_soc, frame, &frameLength);
        if(frameLength!=-1) {
            message = WEBSOCdecodeFrame(frame, message, &frameLength);
            messageLength = strlen(message);

            CHATparseMessage(message, a_soc);
        } else {
            //klient przerwal polaczenie
            int pos = CHATisLogged(NULL, a_soc);
            CHATremoveUser(NULL, a_soc, &pos);
            perror("Connection terminated by client. ");
            free(message);
        	free(frame);
            //remove thread
            break;
        }
        free(message);
        free(frame);
    } //end while
}

void* handshake ( void* clisoc ) {
	int * ptr = (int*)clisoc; 
	int a_soc = *ptr;
	signal(SIGINT, SIG_DFL);
	if((WEBSOChandshake(a_soc))>0) {
        handleClient(&a_soc);
    }
	close(a_soc);
	return NULL;
}

void acceptConnection( int * socketfd ) {
	struct sockaddr_in client_addr; 
	socklen_t clilen = sizeof(client_addr);
	char log[512];
	while(1) {
		int clisoc;
		int * arg_ptr = &clisoc;
		clisoc = accept(*socketfd, (struct sockaddr*)&client_addr, &clilen);
		if(clisoc>=0) {
			pthread_t client;
			pthread_attr_t attr;
 			pthread_attr_init(&attr);
			pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
			bzero(log, 512);
			printf("Connected: %s SOCKETFFD: %d\n", inet_ntoa(client_addr.sin_addr), clisoc);
			snprintf(log, sizeof log, "Connected: %s \n", inet_ntoa(client_addr.sin_addr));
			logEvent(log);	
			fflush(stdout); // just in case 
			if(!pthread_create(&client, &attr, handshake, arg_ptr)) {
				pthread_detach(client);
			}
		} else {
			perror("Accepting error: ");
			exit(0);
		} //end if(clisoc)
	}
}

struct Shared shared; //no need to clean this up, is global

int main( int argc, char *argv[] ) {
	
	SHM = &shared;

	srand(time(NULL));
	signal(SIGINT, forceQuit);
    signal(SIGCHLD, killChildProcess);
    
    int socketfd = socket(PF_INET, SOCK_STREAM, 0), //SCKDGRAM - UDP, STREAM - TCP
        port;

    char * port_garbage = NULL;

    if(argc<2) {
        printf("Usage: ./serwer [port]\n");
        exit(0);
    } else {
    	port = strtol(argv[1], &port_garbage, 10);;
        if(*port_garbage || port<=0) {
            perror("Wrong port number. ");
            exit(0);
        }
    }
 	SHMinit(rand()%1000);
 	CHATprepareMainRoom();
	if(socketfd>0) {
		struct sockaddr_in my_addr; 
		my_addr.sin_family = PF_INET;
		my_addr.sin_addr.s_addr = INADDR_ANY;
		my_addr.sin_port = htons(port);

		if(bind(socketfd, (struct sockaddr*)&my_addr, sizeof(my_addr))>=0) {
			if(!listen(socketfd, 10)) { //return 0 if successful
				printf("Running on %d\n\n", port);
				logEvent("Running on trololo");
				acceptConnection(&socketfd);
			} else {
				perror("Listening error: ");
				exit(0);
			}
		} else {
			perror("Binding error: ");
			exit(0);
		}
	} else {
		perror("Error creating a socket: ");
		exit(0);
	}
	SHMdestroy();
	close(socketfd);
	return 0;
}
