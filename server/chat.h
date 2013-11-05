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

struct CHATcommand {
	int commandId;
	char * name;
	char * param;
};

int CHATisLogged ( char * a_name, int * a_soc );
int CHATfirstEmptySlot();
int CHATalreadyInRoom ( int a_roomId, int * a_pos );
int CHATassignToRoom(int a_id, int * a_fd);


void CHATprepareMainRoom();
void CHATremoveUser ( char * a_name, int * a_soc, int * a_pos );
void CHATremoveUserFromActiveRooms ( int a_pos, int a_fd );
void CHATremoveRoom (int a_id);
void CHATsendReply( int a_statusCode, char * a_message, int *a_soc );
void CHATassignUser ( int * a_pos, int * a_fd, char* a_nick );
void CHATloginUser(struct CHATcommand * cmd, int * a_soc);
void CHATexecuteCommand(struct CHATcommand * cmd, int * a_soc);
void CHATparseMessage(char* a_message, int * a_soc);
void CHATjoinToRoom(struct CHATcommand * cmd, int * a_soc);
void CHATuserAddRoom( int * a_pos , int * a_roomPos );
int CHATgetActiveUsers(int * a_roomId, int * users);

struct CHATcommand * CHATdecodeCommand(char* a_command, struct CHATcommand *cmd);
char* CHATcreateJSONresponse( int * a_statusCode, char* a_message, char* reply);

void DEBUGprintUsers();
void DEBUGprintRoom(int a_pos);

#endif