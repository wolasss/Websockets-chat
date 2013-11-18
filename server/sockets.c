#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include "sockets.h"

#define BYTETOBINARYPATTERN "%d%d%d%d%d%d%d%d\n"
#define BYTETOBINARY(byte)  \
  (byte & 0x80 ? 1 : 0), \
  (byte & 0x40 ? 1 : 0), \
  (byte & 0x20 ? 1 : 0), \
  (byte & 0x10 ? 1 : 0), \
  (byte & 0x08 ? 1 : 0), \
  (byte & 0x04 ? 1 : 0), \
  (byte & 0x02 ? 1 : 0), \
  (byte & 0x01 ? 1 : 0)

#define DEBUG 0

char *SOCreceiveMessage( int *a_soc, int a_bytes, char *aout_message, unsigned int *error) {
    *error = 0;
    if (DEBUG) printf("DEBUG: to read %d bytes.\n", a_bytes);
    if (aout_message != NULL) {
        int left = a_bytes, r = 0, alreadyRead = 0;
        char *temp = malloc(sizeof(char) * a_bytes);
        bzero(temp, a_bytes);
        bzero(aout_message, a_bytes);
        while ( left > 0 && (r = read(*a_soc, temp + alreadyRead, left)) > 0 ) {
            if (DEBUG) printf("DEBUG: read %d bytes...\n", r);
            alreadyRead += r;
            left -= r;
        }
        if (r == 0) {
            *error = 1;
        }
        memcpy(aout_message, temp, a_bytes);
        free(temp);
    }
    return aout_message;
}

int SOCsendMessage( int *a_soc, char *a_message, unsigned long long *bytes ) {
    int ret = 1;
    if (DEBUG) printf("DEBUG: to write %d bytes.\n", (int)*bytes);
    if (a_message != NULL) {
        unsigned long long left = *bytes, s = 0, sent = 0;
        while ( left > 0 && (s = write(*a_soc, a_message + sent, left)) > 0 ) {
            if (DEBUG) printf("DEBUG: sent %d bytes...\n", (int)s);
            sent += s;
            left -= s;
        }
        if (s == 0) {
            ret = 0;
        }
    }
    return ret;
}

char *SOCreceiveRequest( int *a_soc, char *aout_message, int a_bufSize ) {
    char *message  = calloc(a_bufSize, sizeof(char));
    int r = 0, alreadyRead = 0;

    // http request ends with \r\n\r\n.
    while ( !(alreadyRead > 4 && (message[alreadyRead - 4] == '\r' && message[alreadyRead - 3] == '\n' && message[alreadyRead - 2] == '\r' && message[alreadyRead - 1] == '\n')) && (r = read(*a_soc, message + alreadyRead, 512 - alreadyRead)) > 0 ) {
        alreadyRead += r;
        if (DEBUG) printf("DEBUG: read %d bytes...\n", r);
    }
    if (r == 0) {
        free(aout_message);
        aout_message = NULL;
    } else {
        if (DEBUG) printf("already read: %d\n", alreadyRead);
        memcpy(aout_message, message, alreadyRead);
    }
    free(message);
    return aout_message;
}