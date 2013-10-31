#ifndef CHAT_H
#define CHAT_H

#include <stdio.h>
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
#include <fcntl.h>
#include "sockets.h"
#include "websocket.h"
#include "tools.h"
#include "ipc_shared.h"

struct CHATcommand * CHATdecodeCommand(unsigned char * a_command);

int CHATisLogged ( char * a_name );
int CHATfirstEmptySlot();

void CHATsendReply( int a_statusCode, char * a_message, int *a_soc );
void CHATassignUser ( int * a_pos, int * a_fd, char* a_nick );
void CHATloginUser(struct CHATcommand * cmd, int * a_soc);
void CHATexecuteCommand(struct CHATcommand * cmd, int * a_soc);
void CHATparseMessage(unsigned char * a_message, int * a_soc);

#endif