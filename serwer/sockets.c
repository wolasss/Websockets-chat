#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <openssl/sha.h>
#include <time.h>
#include <netdb.h>
#include <unistd.h>
#include <signal.h>
#include <regex.h>
#include <fcntl.h>
#include "sockets.h"

void SOCsendMessage ( int * a_soc, unsigned char* a_message, unsigned long * a_length ) {
    int r;
    if( (r=write( *a_soc, a_message, *a_length ))<0 ) {
        printf("Blad wyslania wiadomosci\n");
    }
}

unsigned char* SOCreceiveMessage ( int * a_soc, unsigned char* aout_message, unsigned long * aout_size ) {

    printf("receveing...\n");

    long i_bufSize=128;
    unsigned int i_r, 
                i_msgSize = 1,
                i_bufInt = 0,
                alreadyReceived = 0;
    unsigned char buf[i_bufSize];
    aout_message = malloc(i_msgSize);

    bzero(aout_message, i_msgSize);
    bzero(buf, i_bufSize);

    while((i_r=read(*a_soc, buf, i_bufSize-1))>0) { //blocking function...
        i_msgSize+=i_r;
        alreadyReceived+=i_r;

        printf("recv %d \n", i_r);

        if((aout_message = realloc(aout_message, i_msgSize))<0) {
            printf("blaaaaad realloca \n");
        }
        
        memcpy(aout_message+((i_bufSize-1)*i_bufInt),buf,i_r);
        if(i_r<i_bufSize-1) {
            break;
        }
        bzero(buf, i_bufSize);
        i_bufInt++;
    } 
    if(i_r==0) {
        //klient przerwal polaczenie
        *aout_size = -1;
    } else {
        *aout_size = (long)i_msgSize-1;
    }
    return aout_message;
}