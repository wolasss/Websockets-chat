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
#include "chat.h"
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

unsigned char * createJSONresponse( int * a_statusCode, unsigned char* a_message ) {
	static unsigned char reply[512];
	bzero(reply, 512);
	if(a_message[0]=='[') {
		snprintf((char*)reply, sizeof reply, "{ \"status\": %d, \"message\": %s }", *a_statusCode, a_message);
	} else {
		snprintf((char*)reply, sizeof reply, "{ \"status\": %d, \"message\": \"%s\" }", *a_statusCode, a_message);
	}
	
	return reply;
}

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
    if(paramLen>=0) {
    	cmd.param = malloc(paramLen);
	    bzero(cmd.param, paramLen);
	    i++; // remove space 
	    while(a_command[i]!='\0') {
	    		cmd.param[b]=a_command[i];
	    		i++; 
	    		b++;
	    }
    }
    if(!strcmp(command, "login")) {
    	cmd.commandId = 1;
    } else if(!strcmp(command, "help")) {
    	cmd.commandId = 2;
    } else if(!strcmp(command, "join")) {
    	cmd.commandId = 3;
    } else if(!strcmp(command, "users")) {
    	cmd.commandId = 4;
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

void CHATgetUserList (char * aout_list) {
	int i;
	strcpy(aout_list, "[\"");
	IPCp(GLOBALsemid,0);
	for(i=0; i<MAX_USERS; i++) {
		if((*SHM).tabUser[i].fd!=0) {
			strcat(aout_list, (*SHM).tabUser[i].nick);
			strcat(aout_list, "\",\"");
		}
	}
	IPCv(GLOBALsemid,0);
	aout_list[strlen(aout_list)-2] = ']';
	aout_list[strlen(aout_list)-1] = '\0';
}

void CHATsendUserList(int * a_soc) {
	char * list = malloc(512);
	unsigned char * reply;
	int statusCode = 104;
	bzero(list,512);
	CHATgetUserList(list);

	reply = createJSONresponse(&statusCode, (unsigned char*)list);
	WEBSOCsendMessage(a_soc, reply);

	printf("soc: %d, reply:\n%s\n", *a_soc, reply);
}

void CHATsendUserListToAll () {
	int users[MAX_USERS], i=0, j=0;
	for(i=0; i<MAX_USERS; i++) {
		users[i]=-1;
	}

	i=0;
	//get all users
	IPCp(GLOBALsemid,0);
	for(j=0; j<MAX_USERS; j++) {
		if((*SHM).tabUser[i].fd!=0) {
			users[i] = (*SHM).tabUser[i].fd;
			i++;
		}
	}
	IPCv(GLOBALsemid,0);

	//send to all
	for(j=0; j<i; j++) {
		CHATsendUserList(&(users[j]));
	}
}

void CHATremoveUser ( char * a_name, int * a_soc, int * pos ) {
	int i;
	if(pos>=0) {
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
		//implicitly assign user to main Room (0), because we starting from i=1
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




void CHATsendReply( int a_statusCode, char * a_message, int *a_soc ) {
	unsigned char * reply;
	reply = createJSONresponse(&a_statusCode, (unsigned char*)a_message);
	WEBSOCsendMessage(a_soc, reply);
	printf("reply message:\n%s\n", reply);
}

void CHATloginUser(struct CHATcommand * cmd, int * a_soc) {
	int firstFree, i, logged=CHATisLogged(cmd->param, NULL), mainRoom = 0;
	if(logged>=0) {
		CHATsendReply(501, "User already exists. Please choose another nickname.", a_soc);
	} else {
		firstFree = CHATfirstEmptySlot();
		if(firstFree>=0) {
			CHATassignUser(&firstFree, a_soc, cmd->param);
			//addTomain
			CHATuserAddRoom(&logged, &mainRoom);
			CHATsendReply(101, "You are logged in.", a_soc);
			//TODO send new user lists.
			CHATsendUserListToAll();
		} else {
			CHATsendReply(502, "There are no empty slots available. Try again later.", a_soc);
		}
	}
}

void CHATexecuteCommand(struct CHATcommand * cmd, int * a_soc) {
	/*
	1 - login
	2 - help
	3 - join
	4 - usersList
	*/
	switch(cmd->commandId) {
		case 1:
			CHATloginUser(cmd, a_soc);
			break;
		case 2:

			break;
		case 3:
			CHATjoinToRoom(cmd, a_soc);
			break;
		case 4:
			CHATsendUserList(a_soc);
			break;
	}
}

int CHATroomExists( char* a_name ) {
	int exists = -1, i=1;
	
	IPCp(GLOBALsemid,1);
	for(i=1; i<MAX_ROOMS; i++) {
		if((*SHM).tabRoom[i].id!=0) {
			if(!strcmp((*SHM).tabRoom[i].name, a_name)) {
				exists=i;
				break;
			}
		}
	}
	IPCv(GLOBALsemid,1);
	
	return exists;
}

void CHATassignToRoom(int a_id, int * a_fd) {
	IPCp(GLOBALsemid,1);
	(*SHM).tabRoom[a_id].id = a_id+1; //TODO
	IPCv(GLOBALsemid,1);
}

int CHATcreateRoom( char* a_name, int * a_creator ) {
	int freeSlot = -1, i; 
	IPCp(GLOBALsemid,1);
	for(i=1; i<MAX_ROOMS; i++) {
		if((*SHM).tabRoom[i].id==0) {
			freeSlot = i;
			break;
		}
	}
	if(freeSlot>=0) {
		(*SHM).tabRoom[freeSlot].id = freeSlot+1;
		strncpy((*SHM).tabRoom[freeSlot].name, a_name, 32);
		(*SHM).tabRoom[freeSlot].activeUsers[0] = *a_creator;
	}
	IPCv(GLOBALsemid,1);

	return freeSlot;
}

void CHATuserAddRoom( int * a_pos , int * a_roomPos ) {
	int i=0;
	IPCp(GLOBALsemid,0);
	for(i=0; i<MAX_ROOMS; i++) {
		if((*SHM).tabUser[*a_pos].activeRooms[i]>=0) {
			(*SHM).tabUser[*a_pos].activeRooms[i] = *a_roomPos+1; // add id of room, not position
			break;
		}
	}
	IPCv(GLOBALsemid,0);
}

int CHATalreadyInRoom ( int a_roomId, int * a_pos ) {
	int alreadyIn = 0, i=0; 
	IPCp(GLOBALsemid,0);
	for(i=0; i<MAX_ROOMS; i++) {
		if((*SHM).tabUser[*a_pos].activeRooms[i]==a_roomId) {
			alreadyIn = 1;
			break;
		}
	}
	IPCv(GLOBALsemid,0);
	return alreadyIn;
}

void CHATjoinToRoom(struct CHATcommand * cmd, int * a_soc) {
	int roomPos = CHATroomExists(cmd->param);
	int pos = CHATisLogged(NULL, a_soc);
	if(pos>=0) {
		if(roomPos>=0) {
			printf("Room istnieje i dodaje\n");
			if(!CHATalreadyInRoom(roomPos+1, &pos)) {
				CHATassignToRoom(roomPos, a_soc);
				CHATuserAddRoom(&pos, &roomPos);
			} else {
				CHATsendReply(503, "You are already in that room.", a_soc);
			}
		} else {
			//tworzymy room
			roomPos = CHATcreateRoom(cmd->param, a_soc);
			if(roomPos<0) {
				CHATsendReply(503, "You cannot join the room. Reached maximum number of rooms.", a_soc);
			} else {
				printf("Room nie istnial ale juz istnieje i dodaje\n");
				CHATuserAddRoom(&pos, &roomPos);
			}
		}
	} else {
		perror("User cannot join to room, because he is not logged in.");
	}
}

void CHATprepareMainRoom() {
	IPCp(GLOBALsemid,1);
	(*SHM).tabRoom[0].id=1;
	strncpy((*SHM).tabRoom[0].name,"main", 5);
	IPCv(GLOBALsemid,1);
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