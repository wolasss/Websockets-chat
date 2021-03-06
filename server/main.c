#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>
#include "sockets.h"
#include "websocket.h"
#include "tools.h"
#include "ipc_shared.h"
#include "chat.h"

#define DEBUG 1

struct Shared *SHM = NULL;
int GLOBALsemid;

void forceQuit(int a_sig) {
    SHMdestroy();
    exit(1);
}

void logEvent(char *a_event) {
    char sz_buf[512];
    int fd_logfile;
    bzero(sz_buf, 512);
    snprintf(sz_buf, sizeof sz_buf, "%s - %s \n", timestamp(), a_event);
    IPCp(GLOBALsemid, 2);
    fd_logfile = open("/tmp/chat.log", O_WRONLY | O_CREAT | O_APPEND, 0666);
    if(fd_logfile>=0) {
        write(fd_logfile, sz_buf, 512);
    }
    IPCv(GLOBALsemid, 2);
}

void handleClient( int *a_soc ) {
    char *frame = NULL, *message = NULL;
    unsigned long long frameLength = 0;
    int pos;
    while (1) {
        frame = WEBSOCreceiveFrame(a_soc, frame, &frameLength);
        if (frameLength != -1 && frame!=NULL) {
            message = WEBSOCdecodeFrame(frame, message, &frameLength);
            if (message) {
                unsigned long long len = strlen(message);
                CHATparseMessage(message, a_soc, len);
            } else {
                // closing frame was sent
                pos = CHATisLogged(NULL, a_soc);
                if(pos>=0) {
                    CHATremoveUser(NULL, a_soc, &pos);
                }
                break;
            }
        } else {
            // client terminated connection
            pos = CHATisLogged(NULL, a_soc);
            if(pos>=0) {
                CHATremoveUser(NULL, a_soc, &pos);
                perror("Connection terminated by client. ");
            }
            // remove thread
            free(message);
            free(frame);
            break;
        }
        free(message);
        message = NULL;
        free(frame);
        frame = NULL;
    }  // end while
}

void *handshake ( void *clisoc ) {
    int *ptr = (int *)clisoc;
    int a_soc = *ptr;
    signal(SIGINT, SIG_DFL);
    if ((WEBSOChandshake(a_soc)) > 0) {
        handleClient(&a_soc);
    } else {
        logEvent("Handshake failed.");
        perror("Handshake failed.");
    }
    close(a_soc);
    pthread_exit(NULL);
    return NULL;
}

void acceptConnection( int *socketfd ) {
    struct sockaddr_in client_addr;
    socklen_t clilen = sizeof(client_addr);
    
    while (1) {
        int clisoc;
        int *arg_ptr = &clisoc;
        clisoc = accept(*socketfd, (struct sockaddr *)&client_addr, &clilen);
        if (clisoc >= 0) {
            pthread_t client;
            pthread_attr_t attr;
            pthread_attr_init(&attr);
            pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
            char * log = calloc(128, sizeof(char));
            if(DEBUG) printf("Connected: %s SOCKETFFD: %d\n", inet_ntoa(client_addr.sin_addr), clisoc);
            snprintf(log, sizeof log, "Connected: %s \n", inet_ntoa(client_addr.sin_addr));
            logEvent(log);
            free(log);
            fflush(stdout);  // just in case
            if (pthread_create(&client, &attr, handshake, arg_ptr)==-1) {
                perror("Error - creating thread");
            }
            pthread_detach(client);
        } else {
            perror("Accepting error: ");
            logEvent("Accepting error.");
            exit(0);
        } //end if(clisoc)
    }
}

struct Shared shared; // no need to clean this up, is global so all props will be set to 0

int main( int argc, char *argv[] ) {
    SHM = &shared;

    srand(time(NULL));
    signal(SIGINT, forceQuit);

    int socketfd = socket(PF_INET, SOCK_STREAM, 0), // SCKDGRAM - UDP, STREAM - TCP
        port;

    char *port_garbage = NULL;

    if (argc < 2) {
        printf("Usage: ./serwer [port]\n");
        exit(0);
    } else {
        port = strtol(argv[1], &port_garbage, 10);;
        if (*port_garbage || port <= 0) {
            perror("Wrong port number. ");
            exit(0);
        }
    }
    SHMinit(rand() % 1000);
    CHATprepareMainRoom();
    if (socketfd > 0) {
        struct sockaddr_in my_addr;
        my_addr.sin_family = PF_INET;
        my_addr.sin_addr.s_addr = INADDR_ANY;
        my_addr.sin_port = htons(port);

        if (bind(socketfd, (struct sockaddr *)&my_addr, sizeof(my_addr)) >= 0) {
            if (!listen(socketfd, 10)) {  // return 0 if successful
                printf("Running on %d\n\n", port);
                acceptConnection(&socketfd);
            } else {
                perror("Listening error: ");
                logEvent("Listening error. ");
                exit(0);
            }
        } else {
            perror("Binding error: ");
            logEvent("Binding error.");
            exit(0);
        }
    } else {
        perror("Error creating a socket: ");
        logEvent("Error creating a socket:");
        exit(0);
    }
    SHMdestroy();
    close(socketfd);
    return 0;
}
