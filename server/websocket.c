#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/sha.h>
#include <regex.h>
#include "sockets.h"
#include "websocket.h"
#include "tools.h"

#define DEBUG 1
#define FRAME_BUFFER_SIZE 1024
#define MAX_ERROR_MSG 0x1000
#define MASK_SIZE 4


char* WEBSOC_createFrame(char* a_message, char* frame, unsigned long * aout_size) {

    unsigned long l_msgLen = strlen(a_message), 
         l_frameSize = 0;

    unsigned int i_addBytes = 0;

    frame = malloc(10); //max length + 2 bytes
    bzero(frame, 10);

    frame[0] = 129; //type of message text - message

    if(l_msgLen<=125) {
        frame[1]=l_msgLen;
    } else if(l_msgLen>125 && l_msgLen<=65535) {
        i_addBytes = 2;
        frame[1] = 126;
        frame[2] = ( l_msgLen >> 8 ) & 255;
        frame[3] = ( l_msgLen      ) & 255;
    } else {
        i_addBytes = 8;
        frame[1] = 127;
        frame[2] = ( l_msgLen >> 56 ) & 255;
        frame[3] = ( l_msgLen >> 48 ) & 255;
        frame[4] = ( l_msgLen >> 40 ) & 255;
        frame[5] = ( l_msgLen >> 32 ) & 255;
        frame[6] = ( l_msgLen >> 24 ) & 255;
        frame[7] = ( l_msgLen >> 16 ) & 255;
        frame[8] = ( l_msgLen >>  8 ) & 255;
        frame[9] = ( l_msgLen       ) & 255;
    }

    l_frameSize = l_msgLen+i_addBytes+2;
    *aout_size = l_frameSize;
    frame = realloc( frame, l_frameSize );
    if(frame != NULL) {
        strncpy(frame+2+i_addBytes, a_message, l_msgLen);
    } else {
        perror("Error allocating memory: ");
    } 

    return frame;
}

char* WEBSOCcreateHandshakeResponse(char* key, char* buffer) {
    char* acceptKey;
    char temp[1024];

    buffer = malloc(1024);
    bzero(buffer, 1024);
    bzero(temp, 1024);
    acceptKey = WEBSOCgenerateAcceptKey(key, acceptKey);
    strcpy(buffer, "HTTP/1.1 101 Switching Protocols\r\n");
    strcat(buffer, "Upgrade: websocket\r\n");
    strcat(buffer, "Connection: Upgrade\r\n");
    sprintf(temp, "Sec-WebSocket-Accept: %s\r\n", acceptKey);
    strcat(buffer, temp);
    strcat(buffer, "Sec-WebSocket-Protocol: chat\r\n");
    strcat(buffer, "\r\n");

    free(acceptKey);
    return buffer;
}

char* WEBSOCgenerateAcceptKey(char* key, char* aout_acceptKey) {
    char temp[1024]; //temp    
    key[strlen((char*)key)-1]=0; //remove null terminator from 1st string...
    aout_acceptKey = malloc(1024);
    bzero(aout_acceptKey, 1024);

    bzero(temp,1024);
    //add magic string
    strcat(key,"258EAFA5-E914-47DA-95CA-C5AB0DC85B11");

    char digest[SHA_DIGEST_LENGTH];
    SHA_CTX ctx;
    SHA1_Init(&ctx);
    SHA1_Update(&ctx, key, strlen(key));
    SHA1_Final((unsigned char*)digest, &ctx);
    base64_encode(digest, SHA_DIGEST_LENGTH, temp, sizeof(temp)); // SHA_DIGEST_LENGTH = 20 

    strcpy(aout_acceptKey, temp);
    return aout_acceptKey;
}

char* WEBSOCgetRequestKey(char* a_request, char* a_key) {
	regex_t r;
    regmatch_t m[1];

    a_key = malloc(512);
    bzero(a_key, 512); 

    compile_regex(&r, "Sec-WebSocket-Key:[[:blank:]]");
    int ex = regexec(&r, a_request, 1, m, REG_EXTENDED);
    if(!ex) {
    	register unsigned int i=0; 
    	while(a_request[m[0].rm_eo+i]!='\n') {
    		a_key[i]=a_request[m[0].rm_eo+i];
    		i++;
    	}
    	a_key[i]='\0';
    	regfree (&r);
    } else {
    	bzero(a_key,512);
    }
    regfree (&r);
    return a_key;
}

void WEBSOCsendMessage( int * a_soc, char* a_message ) {
    char* frame;
    unsigned long frameSize = 0;
    frame = WEBSOC_createFrame(a_message, frame, &frameSize);
    SOCsendMessage(a_soc, frame, &frameSize);
    free(frame);
}

char* WEBSOCdecodeFrame( char* a_frame, char* decoded, unsigned long * a_frameLength ) {
    int closingFrame = 0;

    unsigned char length;

    unsigned int indexFirstMask = 2,
        indexFirstData = 0,
        actualLength = 0,
        j,k, test;

    
    unsigned char mask[MASK_SIZE];    
    bzero(mask, MASK_SIZE);

    
    if( (a_frame[0] & 128)) {
        //final frame
    } 
    if( (a_frame[0] & 15)==8) {
        closingFrame = 1;
        decoded=NULL;
    }
    if(!closingFrame) {
        length = a_frame[1] & 127;
        indexFirstMask = 2;
        if(length<=125) {
            actualLength = length;
            for(k=0; k<MASK_SIZE; k++) {
                mask[k] = a_frame[indexFirstMask+k];
            }
        } else if (length == 126) {
            //length is on 2bytes
            actualLength = (a_frame[2] << 8) + a_frame[3];
            indexFirstMask = 4;
            for(k=0; k<MASK_SIZE; k++) {
                mask[k] = a_frame[indexFirstMask+k];
            }
        } else if (length == 127) {
            /*//not supported yet
            //long - length on 8 bytes
            actualLength =  (a_frame[2] <<56) + (a_frame[3] << 48) + (a_frame[4] << 40) + (a_frame[5] << 32) + (a_frame[6] << 24) + (a_frame[7] <<16) + (a_frame[8] << 8) + (a_frame[9])
            indexFirstMask = 10;
            for(k=0; k<MASK_SIZE; k++) {
                mask[k] = a_frame[indexFirstMask+k];
            }*/
        } else {
            //thro w error
            perror("Bad frame length");
            return NULL;
        }


        indexFirstData = indexFirstMask + MASK_SIZE;
        if((actualLength+indexFirstData)!=(*a_frameLength)) {
            printf("Blad ramki o co chodzi\n");
        }
        decoded = malloc((int)*a_frameLength-indexFirstData+1);
        bzero(decoded, *a_frameLength-indexFirstData);
        for(k=indexFirstData, j=0; k<*a_frameLength; k++, j++) {
            decoded[j] = a_frame[k] ^ mask[j % MASK_SIZE];
        }
        decoded[j]='\0';
    }
    

    return decoded;
}

int WEBSOChandshake( int soc ) {
    unsigned long len;
    char* request, *reply, *key;   
      
    request = SOCreceiveMessage(&soc, request, &len);

    if(DEBUG) {
      printf("Request:\n%s\n", request);
    }

    key = WEBSOCgetRequestKey(request, key);
    //TODO: check if websocket protocol...
    reply = WEBSOCcreateHandshakeResponse(key, reply);

    if(DEBUG) {
      printf("Reply:\n%s\n", reply); //Handshake reply 
    }

    len = strlen(reply);
    SOCsendMessage(&soc,reply,&len);

    free(request);
    free(reply);
    free(key);
    return 1;
}