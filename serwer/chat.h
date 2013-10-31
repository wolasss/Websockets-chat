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

void CHATloginUser(struct CHATcommand * cmd, int * a_soc);
void CHATexecuteCommand(struct CHATcommand * cmd, int * a_soc);
void CHATparseMessage(unsigned char * a_message, int * a_soc);

#endif