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


char *WEBSOC_createFrame(char *a_message, char *frame, unsigned long *aout_size) {
    unsigned long l_msgLen = strlen(a_message),
                  l_frameSize = 0;

    unsigned int i_addBytes = 0;

    frame = malloc(sizeof(char) * 10);  // max length + 2 bytes
    bzero(frame, 10);

    frame[0] = 129;  // type of message text - message

    if (l_msgLen <= 125) {
        frame[1] = l_msgLen;
    } else if (l_msgLen > 125 && l_msgLen <= 65535) {
        i_addBytes = 2;
        frame[1] = 126;
        frame[2] = (l_msgLen >> 8) & 255;
        frame[3] = (l_msgLen) & 255;
    } else {
        i_addBytes = 8;
        frame[1] = 127;
        frame[2] = (l_msgLen >> 56) & 255;
        frame[3] = (l_msgLen >> 48) & 255;
        frame[4] = (l_msgLen >> 40) & 255;
        frame[5] = (l_msgLen >> 32) & 255;
        frame[6] = (l_msgLen >> 24) & 255;
        frame[7] = (l_msgLen >> 16) & 255;
        frame[8] = (l_msgLen >> 8) & 255;
        frame[9] = l_msgLen & 255;
    }

    l_frameSize = l_msgLen + i_addBytes + 2;
    *aout_size = l_frameSize;
    frame = realloc( frame, sizeof(char) * l_frameSize );
    if (frame != NULL) {
        strncpy(frame + 2 + i_addBytes, a_message, l_msgLen);
    } else {
        perror("Error allocating memory: ");
    }

    return frame;
}

char *WEBSOCcreateHandshakeResponse(char *key, char *buffer) {
    char *acceptKey;
    char temp[1024];

    buffer = malloc(sizeof(char) * 1024);
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

char *WEBSOCgenerateAcceptKey(char *key, char *aout_acceptKey) {
    char temp[1024];  // temp
    key[strlen((char *)key) - 1] = 0;  // remove null terminator from 1st string...
    aout_acceptKey = malloc(1024);
    bzero(aout_acceptKey, 1024);

    bzero(temp, 1024);
    // add magic string
    strcat(key, "258EAFA5-E914-47DA-95CA-C5AB0DC85B11");

    char digest[SHA_DIGEST_LENGTH];
    SHA_CTX ctx;
    SHA1_Init(&ctx);
    SHA1_Update(&ctx, key, strlen(key));
    SHA1_Final((unsigned char *)digest, &ctx);
    base64_encode(digest, SHA_DIGEST_LENGTH, temp, sizeof(temp));  // SHA_DIGEST_LENGTH = 20

    strcpy(aout_acceptKey, temp);
    return aout_acceptKey;
}

char *WEBSOCgetRequestKey(char *a_request, char *a_key) {
    regex_t r;
    regmatch_t m[1];

    a_key = malloc(sizeof(char) * 512);
    bzero(a_key, 512);

    compile_regex(&r, "Sec-WebSocket-Key:[[:blank:]]");
    int ex = 1;
    ex = regexec(&r, a_request, 1, m, REG_EXTENDED);
    if (!ex) {
        register unsigned int i = 0;
        while (a_request[m[0].rm_eo + i] != '\n') {
            a_key[i] = a_request[m[0].rm_eo + i];
            i++;
        }
        a_key[i] = '\0';
        regfree(&r);
    } else {
        free(a_key);
        a_key = NULL;
    }
    regfree(&r);
    return a_key;
}

int WEBSOCsendMessage(int *a_soc, char *a_message) {
    int ret = 1;
    char *frame;
    unsigned long frameSize = 0;
    frame = WEBSOC_createFrame(a_message, frame, &frameSize);
    if(!SOCsendMessage(a_soc, frame, &frameSize)) {
        ret =0;
    }
    free(frame);
    return ret;
}

char *WEBSOCdecodeFrame(char *a_frame, char *decoded, unsigned long *a_frameLength) {
    int closingFrame = 0;

    unsigned char length;

    unsigned int indexFirstMask = 2,
                 indexFirstData = 0,
                 actualLength = 0,
                 j, k, test;


    unsigned char mask[MASK_SIZE];
    bzero(mask, MASK_SIZE);


    if ((a_frame[0] & 128)) {
        // final frame
    }
    if ((a_frame[0] & 15) == 8) {
        closingFrame = 1;
        decoded = NULL;
    }
    if (!closingFrame) {
        length = a_frame[1] & 127;
        indexFirstMask = 2;
        if (length <= 125) {
            actualLength = length;
            for (k = 0; k < MASK_SIZE; k++) {
                mask[k] = a_frame[indexFirstMask + k];
            }
        } else if (length == 126) {
            // length is on 2bytes
            actualLength = (a_frame[2] << 8) + a_frame[3];
            indexFirstMask = 4;
            for (k = 0; k < MASK_SIZE; k++) {
                mask[k] = a_frame[indexFirstMask + k];
            }
        } else if (length == 127) {
            // not supported
        } else {
            // throw error
            perror("Bad frame length");
            return NULL;
        }

        indexFirstData = indexFirstMask + MASK_SIZE;
        if ((actualLength + indexFirstData) != (*a_frameLength)) {
            perror("Bad frame");
        }
        decoded = malloc(sizeof(char) * ((int) * a_frameLength - indexFirstData + 1));
        bzero(decoded, *a_frameLength - indexFirstData);
        for (k = indexFirstData, j = 0; k < *a_frameLength; k++, j++) {
            decoded[j] = a_frame[k] ^ mask[j % MASK_SIZE];
        }
        decoded[j] = '\0';
    }

    return decoded;
}

int WEBSOChandshake(int soc) {
    unsigned long len = 0;
    char *request = NULL,
          *reply = NULL,
           *key = NULL;
    int ret = 1;

    request = SOCreceiveMessage(&soc, request, &len);
    if(len!=-1 && request!=NULL) {
        if (request != NULL) {
        key = WEBSOCgetRequestKey(request, key);
        } else {
            ret = 0;
        }
        if (key != NULL) {
        reply = WEBSOCcreateHandshakeResponse(key, reply);
        } else {
            ret = 0;
        }
        if (reply != NULL) {
            len = strlen(reply);
            if(!SOCsendMessage(&soc, reply, &len)) {
                perror("Error sending handshake reply.");
            }
        }
    }
    free(request);
    free(reply);
    free(key);
    return ret;
}
