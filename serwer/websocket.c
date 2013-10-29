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
#include "websocket.h"
#include "tools.h"

#define DEBUG 1
#define FRAME_BUFFER_SIZE 1024
#define MAX_ERROR_MSG 0x1000
#define MASK_SIZE 4


unsigned char* WEBSOC_createFrame(unsigned char* a_message, unsigned long * aout_size) {

    unsigned long l_msgLen = strlen((char*)a_message), 
         l_frameSize = 0;

    unsigned int i_addBytes = 0;

    unsigned char* s_startFrame = malloc(10); //max length + 2 bytes
    static unsigned char* frame;

    s_startFrame[0] = 129; //type of message text - message

    if(l_msgLen<=125) {
        s_startFrame[1]=l_msgLen;
    } else if(l_msgLen>125 && l_msgLen<=65535) {
        i_addBytes = 2;
        s_startFrame[1] = 126;
        s_startFrame[2] = ( l_msgLen >> 8 ) & 255;
        s_startFrame[3] = ( l_msgLen      ) & 255;
    } else {
        i_addBytes = 8;
        s_startFrame[1] = 127;
        s_startFrame[2] = ( l_msgLen >> 56 ) & 255;
        s_startFrame[3] = ( l_msgLen >> 48 ) & 255;
        s_startFrame[4] = ( l_msgLen >> 40 ) & 255;
        s_startFrame[5] = ( l_msgLen >> 32 ) & 255;
        s_startFrame[6] = ( l_msgLen >> 24 ) & 255;
        s_startFrame[7] = ( l_msgLen >> 16 ) & 255;
        s_startFrame[8] = ( l_msgLen >>  8 ) & 255;
        s_startFrame[9] = ( l_msgLen       ) & 255;
    }

    l_frameSize = l_msgLen+i_addBytes+2;
    *aout_size = l_frameSize;
    frame = realloc( s_startFrame, l_frameSize );
    if(frame != NULL) {
        strncpy((char*)frame+2+i_addBytes, (char*)a_message, l_msgLen);
    } else {
        perror("Error allocating memory: ");
    } 
    return frame;
}

unsigned char* WEBSOCcreateHandshakeResponse(unsigned char* key) {
    static char buffer[1024];
    char temp[1024];
    bzero(buffer, 1024);
    bzero(temp, 1024);
    unsigned char * acceptKey = WEBSOCgenerateAcceptKey(key);
    strcpy(buffer, "HTTP/1.1 101 Switching Protocols\r\n");
    strcat(buffer, "Upgrade: websocket\r\n");
    strcat(buffer, "Connection: Upgrade\r\n");
    sprintf(temp, "Sec-WebSocket-Accept: %s\r\n", acceptKey);
    strcat(buffer, temp);
    strcat(buffer, "Sec-WebSocket-Protocol: chat\r\n");
    strcat(buffer, "\r\n");

    return (unsigned char*)buffer;
}

unsigned char* WEBSOCgenerateAcceptKey(unsigned char* key) {    
    key[strlen((char*)key)-1]=0; //remove null terminator from 1st string...
    static char res[1024];

    //add magic string
    strcat((char*)key,"258EAFA5-E914-47DA-95CA-C5AB0DC85B11");

    unsigned char digest[SHA_DIGEST_LENGTH];
    SHA_CTX ctx;
    SHA1_Init(&ctx);
    SHA1_Update(&ctx, key, strlen((char*)key));
    SHA1_Final(digest, &ctx);
 
    bzero(res, 1024);
    base64_encode(digest, SHA_DIGEST_LENGTH, res, sizeof(res)); // SHA_DIGEST_LENGTH = 20
    return (unsigned char*)res; 
}

unsigned char* WEBSOCgetRequestKey(unsigned char* a_request) {
	regex_t r;
    regmatch_t m[1];

    static char key[512];
    bzero(key, 512); 

    compile_regex(&r, "Sec-WebSocket-Key:[[:blank:]]");
    int ex = regexec(&r, (char*)a_request, 1, m, REG_EXTENDED);
    if(!ex) {
    	register unsigned int i=0; 
    	while(a_request[m[0].rm_eo+i]!='\n') {
    		key[i]=a_request[m[0].rm_eo+i];
    		i++;
    	}
    	key[i]='\0';
    	regfree (&r);
    } else {
    	bzero(key,512);
    }
    regfree (&r);
    return (unsigned char*)key;
}

void WEBSOCsendMessage( int * a_soc, unsigned char* a_message ) {
    if(fork()==0) {
        unsigned long frameSize = 0;
        unsigned char * frame = WEBSOC_createFrame(a_message, &frameSize);
        SOCsendMessage(a_soc, frame, &frameSize);
        exit(0);
    }
}

unsigned char* WEBSOCdecodeFrame( unsigned char* a_frame, unsigned long * a_frameLength ) {

    unsigned char length;

    unsigned int indexFirstMask = 2,
        indexFirstData = 0,
        actualLength = 0,
        j,k, test;

    
    unsigned char mask[MASK_SIZE];    
    bzero(mask, MASK_SIZE);

    printf("frame: %d\n", *a_frameLength);
    test = a_frame[0] & 15;
    if(test==0) {
        printf("text frame\n");
    } else if(test==9) {
        printf("ping frame\n");
    } else if(test==8) {
        printf("connection close frame\n");
    }
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
        printf("trololo \n");
    }


    indexFirstData = indexFirstMask + MASK_SIZE;
    if((actualLength+indexFirstData)!=(*a_frameLength)) {
        printf("Blad ramki o co chodzi\n");
        printf("allocated: %d", *a_frameLength-indexFirstData);
    }
    printf("allocated: %d", (int)*a_frameLength-indexFirstData);
    unsigned char* decoded = malloc((int)*a_frameLength-indexFirstData);
      printf("allocated: %d", (int)*a_frameLength-indexFirstData);
    bzero(decoded, *a_frameLength-indexFirstData);
    for(k=indexFirstData, j=0; k<*a_frameLength; k++, j++) {
        decoded[j] = a_frame[k] ^ mask[j % MASK_SIZE];
    }
    decoded[j]='\0';

    return decoded;
}

int WEBSOChandshake( int * soc ) {
    unsigned long len;
    unsigned char* request, *reply, *key;   
      
    request = SOCreceiveMessage(soc, request, &len);
    
    if(DEBUG) {
      printf("Request:\n%s\n", request);
    }

    key = WEBSOCgetRequestKey(request);
    //TODO: check if websocket protocol...
    reply = WEBSOCcreateHandshakeResponse(key);

    if(DEBUG) {
      printf("Reply:\n%s\n", reply); //Handshake reply 
    }

    len = strlen((char*)reply);
    SOCsendMessage(soc,reply,&len);

    return 1;
}