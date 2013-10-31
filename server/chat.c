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

#define SEM_USERS 0
#define SEM_ROOMS 1
#define SEM_LOGFILE 2

#define MAX_USERS 20
#define MAX_ROOMS 10

extern struct Shared * SHM;
extern int GLOBALsemid;


struct CHATcommand {
	int commandId;
	char * param;
};

struct CHATcommand * CHATdecodeCommand(unsigned char * a_command) {
	static struct CHATcommand cmd;

	int paramLen, i=1, b=0;

	char command[16];
	bzero(command,16);

	while(a_command[i]!=' ' && i<16) {
    		command[i-1]=a_command[i];
    		i++;
    }
    paramLen = strlen((char*)a_command)-i+1;
    cmd.param = malloc(paramLen);
    bzero(cmd.param, paramLen);
    i++; // remove space 
    while(a_command[i]!='\0') {
    		cmd.param[b]=a_command[i];
    		i++; 
    		b++;
    }

    if(!strcmp(command, "login")) {
    	cmd.commandId = 1;
    } else if(!strcmp(command, "help")) {
    	cmd.commandId = 2;
    }

    return &cmd;
}

int CHATisLogged ( char * a_name, int * a_soc ) {
	int loginPosition = -1, i;
	IPCp(GLOBALsemid,0);
	for(i=0; i<MAX_USERS; i++) {
		if((a_soc && (*SHM).tabUser[i].fd==(*a_soc)) || (a_name && !strcmp((*SHM).tabUser[i].nick, a_name)) ) {
			loginPosition = i;
			break;
		}
	}
	IPCv(GLOBALsemid,0);

	return loginPosition;
}

void CHATremoveUser ( char * a_name, int * a_soc, int * pos ) {
	int i;
	if(pos) {
		IPCp(GLOBALsemid,0);
		(*SHM).tabUser[*pos].fd = 0;
		bzero((*SHM).tabUser[*pos].nick, 32);
		IPCv(GLOBALsemid,0);
	} else {
		IPCp(GLOBALsemid,0);
		for(i=0; i<MAX_USERS; i++) {
			if((a_soc && (*SHM).tabUser[i].fd==(*a_soc)) || (a_name && !strcmp((*SHM).tabUser[i].nick, a_name)) ) {
				(*SHM).tabUser[i].fd = 0;
				bzero((*SHM).tabUser[i].nick, 32);
				break;
			}
		}
		IPCv(GLOBALsemid,0);
	}
}

void CHATassignUser ( int * a_pos, int * a_fd, char* a_nick ) {
	int i;
	IPCp(GLOBALsemid,0);
	(*SHM).tabUser[*a_pos].fd = *a_fd;
	strncpy((*SHM).tabUser[*a_pos].nick, a_nick, 32);
	for(i=1; i<MAX_ROOMS; i++) {
		(*SHM).tabUser[*a_pos].activeRooms[i] = -1;
		//implicitly assign user to main Room (0)
	}
	IPCv(GLOBALsemid,0);
}

int CHATfirstEmptySlot() {
	int firstFree = -1, i;
	IPCp(GLOBALsemid,0);
	for(i=0; i<MAX_USERS; i++) {
		if((*SHM).tabUser[i].fd==0) {
			firstFree = i;
			break;
		}
	}
	IPCv(GLOBALsemid,0);
	return firstFree;
}

unsigned char * createJSONresponse( int * a_statusCode, unsigned char* a_message ) {
	static unsigned char reply[512];
	bzero(reply, 512);
	snprintf((char*)reply, sizeof reply, "{ \"status\": %d, \"message\": \"%s\" }", *a_statusCode, a_message);
	return reply;
}


void CHATsendReply( int a_statusCode, char * a_message, int *a_soc ) {
	unsigned char * reply;
	reply = createJSONresponse(&a_statusCode, (unsigned char*)a_message);
	WEBSOCsendMessage(a_soc, reply);
	printf("reply message:\n%s\n", reply);
}

void CHATloginUser(struct CHATcommand * cmd, int * a_soc) {
	int firstFree, i, logged=CHATisLogged(cmd->param, NULL);
	if(logged>=0) {
		CHATsendReply(501, "User already exists. Please choose another nickname.", a_soc);
	} else {
		firstFree = CHATfirstEmptySlot();
		if(firstFree>=0) {
			CHATassignUser(&firstFree, a_soc, cmd->param);
			CHATsendReply(101, "You are logged in.", a_soc);
			//TODO send new user lists.
		} else {
			CHATsendReply(502, "There are no empty slots available. Try again later.", a_soc);
		}
	}
}

void CHATexecuteCommand(struct CHATcommand * cmd, int * a_soc) {
	/*
	1 - login
	2 - help

	*/
	switch(cmd->commandId) {
		case 1:
			CHATloginUser(cmd, a_soc);
			break;
		case 2:

			break;
	}
}

void CHATprepareMainRoom() {
	IPCp(GLOBALsemid,0);
	(*SHM).tabRoom[0].id=1;
	strncpy((*SHM).tabRoom[0].name,"main", 5);
	IPCv(GLOBALsemid,0);
}

void CHATparseMessage(unsigned char * a_message, int * a_soc) {
	if(a_message[0]=='/') {
		struct CHATcommand * cmd = CHATdecodeCommand(a_message);
		if(cmd->commandId>0) {
			CHATexecuteCommand(cmd, a_soc);
		} else {
			perror("Unknown command.");
		}
	} else if(a_message[0]=='@') {
		printf("to jest prywatna wiadomosc");
	} else {
		printf("to jest publiczna wiadomosc\n");
	}
}