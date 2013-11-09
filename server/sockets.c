#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include "sockets.h"

void SOCsendMessage ( int * a_soc, char* a_message, unsigned long * a_length ) {
    if( write( *a_soc, a_message, *a_length )<0 ) {
        perror("Error sending a message: ");
    }
}

char* SOCreceiveMessage ( int * a_soc, char* aout_message, unsigned long * aout_size ) {

    long i_bufSize=128;
    unsigned int i_r, 
                i_msgSize = 1,
                i_bufInt = 0,
                alreadyReceived = 0;
    char buf[i_bufSize];
    aout_message = malloc(sizeof(char)*i_msgSize);

    bzero(aout_message, i_msgSize);
    bzero(buf, i_bufSize);
    while((i_r=read(*a_soc, buf, i_bufSize-1))>0) { //blocking function...
        i_msgSize+=i_r;
        alreadyReceived+=i_r;
        
        if((aout_message = realloc(aout_message, sizeof(char)*i_msgSize))<0) {
            perror("Error allocating memory: ");
        }
        
        memcpy(aout_message+((i_bufSize-1)*i_bufInt),buf,i_r);
        if(i_r<i_bufSize-1) {
            break;
        }
        bzero(buf, i_bufSize);
        i_bufInt++;
    } 
    if(i_r==0) {
        //client terminated a connection
        *aout_size = -1;
    } else {
        *aout_size = (long)i_msgSize-1;
    }
    return aout_message;
}