/*

zapytac sie czy trzeba blokowac semaforami dostep do socketu

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
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <netdb.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include "sockets.h"
#include "websocket.h"
#include "tools.h"

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
#define FRAME_BUFFER_SIZE 1024
#define MAX_ERROR_MSG 0x1000
#define MASK_SIZE 4



char* timestamp()
{
    time_t ltime; /* calendar time */
    ltime=time(NULL); /* get current cal time */
    return asctime( localtime(&ltime) );
}

void killChildProcess(int a_sig) {
    wait(NULL);
}

void logevent(char* a_event) {
	if(fork()==0) {
		char sz_buf[512];
		snprintf(sz_buf, sizeof sz_buf, "%s - %s", timestamp(), a_event);
		//p(semid,3);
		int fd_logfile = open("/tmp/chat.log", O_WRONLY|O_CREAT|O_APPEND, 0666);
		dup2(fd_logfile, 1);
		close(fd_logfile);
		printf("%s\n", sz_buf);
		//v(semid,3);
		exit(1);
	}
}



void handleClient( int * a_soc ) {

    unsigned char *frame, *message;
    unsigned long messageSize = 0, frameLength = 0;
    while(1) {
        bzero(message, messageSize);

        frame = SOCreceiveMessage(a_soc, frame, &frameLength);
        if(frameLength!=-1) {
            message = WEBSOCdecodeFrame(frame, &frameLength);
            messageSize = strlen((char*)message);

            printf("Msg: %s\n", message);
            // and now handle the message man...
            WEBSOCsendMessage(a_soc, (unsigned char*)"Odebralem wiadomosc. hehehheszki. taaaak");
        } else {
            //klient przerwal polaczenie
            printf("Polaczenie przerwane... \n");
            exit(1);
        }
        
    }

}




int main( int argc, char *argv[] ) {
    signal(SIGCHLD, killChildProcess);
    
    socklen_t clilen;

    int socketfd = socket(PF_INET, SOCK_STREAM, 0), //SCKDGRAM - UDP, STREAM - TCP
        clisoc,
        port = atoi(argv[1]);

    if(argc<2) {
        printf("Usage: ./serwer [port]\n");
        exit(0);
    } else {
        if(port<=0) {
            perror("Wrong port number.");
            exit(0);
        }   
    }
 		
	if(socketfd>0) {

		struct sockaddr_in my_addr, client_addr; 
		my_addr.sin_family = PF_INET;
		my_addr.sin_addr.s_addr = INADDR_ANY;
		my_addr.sin_port = htons(port);

		if(bind(socketfd, (struct sockaddr*)&my_addr, sizeof(my_addr))>=0) {
			printf("Running on %d\n\n", port);
			clilen = sizeof(client_addr);
			listen(socketfd, 10);
			while(1) {
				clisoc = accept(socketfd, (struct sockaddr*)&client_addr, &clilen);
				if(clisoc>=0) {
					printf("Connected: %s \n", inet_ntoa(client_addr.sin_addr));
					//char buf[512];
					//snprintf(buf, sizeof buf, "Connected: %s \n", inet_ntoa(client_addr.sin_addr));
					//logevent(buf);	
					fflush(stdout);
					if(fork()==0) {
                        close(socketfd);
						if((WEBSOChandshake(&clisoc))>0) {
                            handleClient(&clisoc);
                        }
						close(clisoc);
						exit(0);
					} else {
						close(clisoc);
					}
				} else {
					perror("Accepting error: ");
					exit(0);
				} //end if(clisoc)
			} // end while(1)
		} else {
			perror("Binding error: ");
			exit(0);
		}
	} else {
		perror("Error creating a socket: ");
		exit(0);
	}
	close(socketfd);
	return 0;
}
