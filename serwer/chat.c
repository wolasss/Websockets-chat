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

void CHATloginUser(struct CHATcommand * cmd, int * a_soc) {
	printf("Loguje uzytkownika %s \n", cmd->param);
	int firstFree = -1, i;
	IPCp(GLOBALsemid,0);
	printf("im in! uzytkownika %s \n", cmd->param);
	/*for(i=0; i<20; i++) {
		if(SHM->tabUser[i].fd==0) {
			firstFree = i;
			break;
		}
	}
	SHM->tabUser[firstFree].fd = *a_soc;
	strncpy(SHM->tabUser[firstFree].nick, cmd->param, 32);
	printf("First free: %d \n", firstFree);*/
	IPCv(GLOBALsemid,0);
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