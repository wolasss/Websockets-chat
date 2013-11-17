#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include "sockets.h"

#define DEBUG 0

char * SOCreceiveMessage( int * a_soc, int bytes, char * aout_message, unsigned int * error) {
    *error = 0;
    if(DEBUG) printf("DEBUG: to read %d bytes.\n", bytes);
    if(aout_message!=NULL) {
        int left = bytes, r = 0, alreadyRead = 0;
        char * temp = malloc(sizeof(char)*bytes);
        bzero(temp, bytes);
        bzero(aout_message, bytes);
        while( left>0 && (r=read(*a_soc, temp+alreadyRead, left))>0 ) {
            if(DEBUG) printf("DEBUG: read %d bytes...\n", r);
            alreadyRead += r;
            left -= r;
        }
        if(r==0) {
            *error = 1;
        }
        memcpy(aout_message, temp, bytes);
        free(temp);
    }
    return aout_message;
}

int SOCsendMessage( int * a_soc, char * a_message, unsigned long long * bytes ) {
    int ret = 1;
    if(DEBUG) printf("DEBUG: to write %d bytes.\n", (int)*bytes);
    if(a_message!=NULL) {
        long left = *bytes, s = 0, sent = 0;
        while( left>0 && (s=write(*a_soc, a_message+sent, left))>0 ) {
            if(DEBUG) printf("DEBUG: sent %d bytes...\n", (int)s);
            sent += s;
            left -= s;
        }
        if(s==0) {
            ret = 0;
        }
    }
    return ret;
}

char *SOCreceiveRequest ( int *a_soc, char *aout_message, int a_bufSize ) {
    char * message  = calloc(a_bufSize, sizeof(char));
    int r = 0, alreadyRead = 0;
    while( !(alreadyRead>4 && (message[alreadyRead-4]=='\r' && message[alreadyRead-3]=='\n' && message[alreadyRead-2]=='\r' && message[alreadyRead-1]=='\n')) && (r=read(*a_soc, message+alreadyRead, 512-alreadyRead))>0 ) {
        alreadyRead += r;
        if(DEBUG) printf("DEBUG: read %d bytes...\n", r);
    }
    if(r==0) {
        free(aout_message);
        aout_message = NULL;
    } else {
        if(DEBUG) printf("already read: %d\n", alreadyRead);
        memcpy(aout_message, message, alreadyRead);
    }
    free(message);
    return aout_message;
}

char* SOCreceiveFrame ( int * a_soc, char* aout_message, unsigned long long * aout_size ) {
    
    char *header = calloc(14, sizeof(char)),  // read header of the frame -> max 14 bytes -> 2 bytes + 8 bytes length + 4bytes mask.
         *firstByte = calloc(1, sizeof(char)), 
         *lengthFirst = calloc(1, sizeof(char)),
         *mask = calloc(4, sizeof(char));
    unsigned int finalFrame = 0,
        preLength = 0,
        currentPosition = 0,
        k,
        error=0;
    unsigned long long frameSize = 0,
                  length = 0;

    // read first byte. is this final frame?
    firstByte = SOCreceiveMessage(a_soc, 1, firstByte, &error);
    if(!error) {
        frameSize++;
        finalFrame = (firstByte[0] & 128) ? 1 : 0;
        memcpy(header+currentPosition, firstByte, 1);
        currentPosition++;
        // read length byte.
        lengthFirst = SOCreceiveMessage(a_soc, 1, lengthFirst, &error);
        if(!error) {
            frameSize++;
            memcpy(header+currentPosition, lengthFirst, 1);
            currentPosition++;
            preLength = lengthFirst[0] & 127;
            if(preLength<=125) {
                length = preLength;
            } else if(preLength==126) {
                //length is on two next bytes.
                char *temp = calloc(2, sizeof(char));
                temp = SOCreceiveMessage(a_soc, 2, temp, &error);
                memcpy(header+currentPosition, temp, 2);
                length = ((unsigned char)temp[0] << 8) + (unsigned char)temp[1];
                free(temp);
                frameSize+=2;
                currentPosition+=2;
            } else if(preLength==127) {
                //length is on eight next bytes.
                char *temp = calloc(8, sizeof(char));
                temp = SOCreceiveMessage(a_soc, 8, temp, &error);
                memcpy(header+currentPosition, temp, 8);
                length = (((unsigned long long)temp[0]) << 56) + (((unsigned long long)temp[1]) << 48) + (((unsigned long long)temp[2]) << 40) + (((unsigned long long)temp[3] << 32)) + (((unsigned long long)temp[4]) << 24) + (((unsigned long long)temp[5]) << 16) + (((unsigned long long)temp[6]) << 8) + (unsigned long long)temp[7];
                free(temp);
                frameSize+=8; 
                currentPosition+=8;
                printf("DEBUG: length of message: %d\n", (int)length);
            } else {
                error = 1;
            }
        
            mask = SOCreceiveMessage(a_soc, 4, mask, &error);
            if(!error) {
                memcpy(header+currentPosition, mask, 4);
                currentPosition+=4;
                frameSize+=4;
                frameSize+=length;

                aout_message = calloc(frameSize, sizeof(char));
                memcpy(aout_message, header, currentPosition+1);
                char * frameMessage = aout_message+currentPosition;
                if(length>0) {
                    frameMessage = SOCreceiveMessage(a_soc, length, aout_message+currentPosition, &error);
                }
                printf("len: %d\n", (int)length);
                *aout_size = frameSize;
            } else {
                *aout_size = -1;
                aout_message = NULL;
            }
        } else {
            *aout_size = -1;
            aout_message = NULL;
        }
    } else {
        *aout_size = -1;
        aout_message = NULL;
    }
    
    free(firstByte);
    free(lengthFirst);
    free(mask);
    free(header);
    return aout_message;
}

