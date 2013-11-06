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

char * CHATcreateJSON ( int * a_statusCode, char* a_sender, char* a_room, char* a_message, char* aout_reply ) {
	aout_reply = malloc(512);
	long messageAloc = strlen(a_message)+20;
	char * statusJSON = malloc(32),
			*senderJSON = malloc(64),
			*roomJSON = malloc(64),
			*messageJSON = malloc(messageAloc);

	bzero(statusJSON, 32);
	bzero(senderJSON, 64);
	bzero(roomJSON, 64);
	bzero(aout_reply,512);
	bzero(messageJSON, messageAloc);

	snprintf(statusJSON, 32, " \"status\": %d,", *a_statusCode);
	if(a_sender) {
		snprintf(senderJSON, 64, " \"sender\": \"%s\",", a_sender);
	}
	if(a_room) {
		snprintf(roomJSON, 64, " \"room\": \"%s\",", a_room);
	}
	if(a_message[0]=='[') {
		snprintf(messageJSON, messageAloc, " \"message\": %s", a_message);
	} else {
		snprintf(messageJSON, messageAloc, " \"message\": \"%s\"", a_message);
	}

	snprintf(aout_reply, 512, "{ %s %s %s %s }", statusJSON, senderJSON, roomJSON, messageJSON);
	
	free(statusJSON);
	free(senderJSON);
	free(roomJSON);
	free(messageJSON);
	return aout_reply;
}

struct CHATcommand * CHATdecodeCommand(char* a_command, struct CHATcommand *cmd) {
    cmd = (struct CHATcommand *) malloc(sizeof(struct CHATcommand));

    int paramLen, i=3, b=0, cmdLen = strlen(a_command), k=0;

    char command[16];
    bzero(command,16);
    while(!(a_command[i]=='%' && a_command[i+1]=='2' && a_command[i+2]=='0') && k<15) {
            command[k]=a_command[i];
            i++; k++;
    }
    cmd->name = malloc(i-2);
    bzero(cmd->name, i-2);

    paramLen = cmdLen-i+1;
    if(paramLen>=0) {
        cmd->param = malloc(paramLen);
        bzero(cmd->param, paramLen);
        bzero(cmd->param, paramLen);
        i=i+3; // remove space -> urldecode(%20)
        while(i<cmdLen) {
                cmd->param[b]=a_command[i];
                i++; 
                b++;
        }
    }
    strncpy(cmd->name, command, strlen(command));
    if(!(a_command[0]=='%' && a_command[1]=='2' && a_command[2]=='5')) {
        if(!strcmp(command, "login")) {
            cmd->commandId = 1;
        } else if(!strcmp(command, "help")) {
            cmd->commandId = 2;
        } else if(!strcmp(command, "join")) {
            cmd->commandId = 3;
        } else if(!strcmp(command, "users")) {
            cmd->commandId = 4;
        } 
    }
    
    return cmd;
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

char* CHATgetUserList (char* aout_list) {
	aout_list = malloc(1024);
	bzero(aout_list, 1024);
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
	return aout_list;
}

void CHATsendUserList(int * a_soc) {
	char* list;
	char* reply;
	int statusCode = 104;
	list = CHATgetUserList(list);
	reply = CHATcreateJSON(&statusCode, NULL, NULL, list, reply);
	WEBSOCsendMessage(a_soc, reply);

	free(list);
	free(reply);
}

void CHATsendMessage(int type, int * a_soc, char * a_sender, char* a_room, char * a_message) {
	char* messageJSON;
	int statusCode;

	if(type==0) {
		statusCode = 198;
		messageJSON = CHATcreateJSON(&statusCode, a_sender, a_room, a_message, messageJSON);
		printf("\npublic:%s\n", messageJSON);
	} else {
		statusCode = 199;
		messageJSON = CHATcreateJSON(&statusCode, a_sender, a_room, a_message, messageJSON);
		printf("\nprivate:%s\n", messageJSON);
	}
	
	WEBSOCsendMessage(a_soc, messageJSON);

	free(messageJSON);
}

void CHATsendCtrlMessage(int * a_soc, char* a_room, char * a_message) {
	char* messageJSON;
	int statusCode = 195;
	messageJSON = CHATcreateJSON(&statusCode, NULL, a_room, a_message, messageJSON);
	WEBSOCsendMessage(a_soc, messageJSON);

	free(messageJSON);
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
		if((*SHM).tabUser[j].fd>0) {
			users[i] = (*SHM).tabUser[j].fd;
			i++;
		}
	}
	IPCv(GLOBALsemid,0);
	//send to all
	for(j=0; j<i; j++) {
		CHATsendUserList(&(users[j]));
	}
}

void CHATremoveRoom (int a_id) {
	IPCp(GLOBALsemid,1);
	(*SHM).tabRoom[a_id-1].id=0;
	bzero((*SHM).tabRoom[a_id-1].name, 32);
	bzero((*SHM).tabRoom[a_id-1].activeUsers, MAX_USERS);
	(*SHM).tabRoom[a_id-1].users = 0;
	IPCv(GLOBALsemid,1);
}

void CHATremoveUserFromActiveRooms ( int a_pos, int a_fd ) {
	int i,j,k=0,l=0, temp;
	int activeRooms[MAX_ROOMS], toRemove[MAX_ROOMS];

	IPCp(GLOBALsemid,0);
	for(j=0; j<MAX_ROOMS; j++) {
		temp = (*SHM).tabUser[a_pos].activeRooms[j];
		if(temp>=0) {
			activeRooms[k] = temp;
			k++;
			(*SHM).tabUser[a_pos].activeRooms[j] = -1;
		}
	}
	IPCv(GLOBALsemid,0);
	IPCp(GLOBALsemid,1);
	for(i=0; i<k; i++) {
		if(activeRooms[i]>=1 && ((*SHM).tabRoom[activeRooms[i]-1].users<=1) && !((*SHM).tabRoom[activeRooms[i]-1].id==1)) {
			toRemove[l++]=activeRooms[i];
		} else {
			//check all users and remove the one
			for(j=0; j<MAX_USERS; j++) {
				if((*SHM).tabRoom[activeRooms[i]-1].activeUsers[j]==a_fd) {
					(*SHM).tabRoom[activeRooms[i]-1].activeUsers[j] = 0;
					(*SHM).tabRoom[activeRooms[i]-1].users--;
					break;
				}
			}
		}
	}
	IPCv(GLOBALsemid,1);
	for(i=0; i<l; i++) {
		CHATremoveRoom(toRemove[i]);
	}
}

void CHATsendCtrlMessageToAll (int * a_roomId , char* a_roomName, char* a_message) {
	int users[MAX_USERS], i=0, k=0;
	k = CHATgetActiveUsers(a_roomId, users);
	
	printf("do ilu wysle wiadomosc kontrolna: %d\n", k);
	for(i=0; i<k; i++) {
		CHATsendCtrlMessage(&(users[i]), a_roomName, a_message);
	}
}

void CHATremoveUser ( char * a_name, int * a_soc, int * a_pos ) {
	int pos = *a_pos, soc = *a_soc;
	char * controlMessage = malloc(128);
	struct CHATcommand room;
	int mainRoom = 0;
	room.name = malloc(5);
	room.param = malloc(32);
	bzero(room.name, 5);
	strcpy(room.name,"main");

	if(pos>=0) {
		CHATremoveUserFromActiveRooms(pos, soc);
		IPCp(GLOBALsemid,0);
		(*SHM).tabUser[*a_pos].fd = 0;
		strcpy(room.param, (*SHM).tabUser[*a_pos].nick); //need nick to notify others
		bzero((*SHM).tabUser[*a_pos].nick, 32);
		IPCv(GLOBALsemid,0);
	}
	//send to all new list of users
	printf("sending to all \n");
	snprintf(controlMessage, 128, "User %s has logged out.", room.param);
	CHATsendCtrlMessageToAll(&mainRoom, room.name, controlMessage);
	CHATsendUserListToAll();
	DEBUGprintUsers();
	DEBUGprintRoom(0);
	free(controlMessage);
	free(room.name);
	free(room.param);
	close(*a_soc);
}

void CHATassignUser ( int * a_pos, int * a_fd, char* a_nick ) {
	int i;
	IPCp(GLOBALsemid,0);
	(*SHM).tabUser[*a_pos].fd = *a_fd;
	strncpy((*SHM).tabUser[*a_pos].nick, a_nick, 32);
	(*SHM).tabUser[*a_pos].activeRooms[0] = 0;
	for(i=0; i<MAX_ROOMS; i++) {
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
	char* reply;
	reply = CHATcreateJSON(&a_statusCode, NULL, NULL, a_message, reply);
	WEBSOCsendMessage(a_soc, reply);
	free(reply);
}

int CHATgetActiveUsers(int * a_roomId, int * users) {
	int i=0, activeUsers=0;
	IPCp(GLOBALsemid,1);
	for(i=0; i<MAX_USERS; i++) {
		//printf("fd: %d\n", (*SHM).tabRoom[*a_roomId].activeUsers[i]);
		if((*SHM).tabRoom[*a_roomId].activeUsers[i]>0) {
			users[activeUsers]=(*SHM).tabRoom[*a_roomId].activeUsers[i];
			activeUsers++;
		}
	}
	IPCv(GLOBALsemid,1);
	return activeUsers;
} 

void DEBUGprintRoom(int a_pos) {
	int i=0, j=0;
	IPCp(GLOBALsemid,1);
	for(i=0; i<MAX_ROOMS; i++) {
		printf("\n%d : %s : users: %d \nAU: ", (*SHM).tabRoom[i].id, (*SHM).tabRoom[i].name, (*SHM).tabRoom[i].users);
		for(j=0; j<MAX_USERS; j++) {
			printf("%d ", (*SHM).tabRoom[i].activeUsers[j]);
		}
	printf("\n");
	}
	IPCv(GLOBALsemid,1);
}

void DEBUGprintUsers() {
	int i=0, j=0;
	IPCp(GLOBALsemid,0);
	for(i=0; i<MAX_USERS; i++) {
		printf("%d : %s : \nAR: ", (*SHM).tabUser[i].fd, (*SHM).tabUser[i].nick);
		for(j=0; j<MAX_ROOMS; j++) {
			printf("%d ", (*SHM).tabUser[i].activeRooms[j]);
		}
		printf("\n");
	}
	IPCv(GLOBALsemid,0);
}
void CHATclearActiveRooms (int * a_pos) {
	int i=0;
	IPCp(GLOBALsemid,0);
	for(i=0; i<MAX_ROOMS; i++) {
		(*SHM).tabUser[*a_pos].activeRooms[i] = -1;
	}
	IPCv(GLOBALsemid,0);
}
void CHATloginUser(struct CHATcommand * cmd, int * a_soc) {
	int firstFree, i, logged=CHATisLogged(cmd->param, NULL), mainRoom = 0;
	char * controlMessage = malloc(128);
	bzero(controlMessage, 128);
	struct CHATcommand room;
	room.param = malloc(5);
	bzero(room.param, 5);
	strcpy(room.param,"main");
	if(logged>=0) {
		CHATsendReply(501, "User already exists. Please choose another nickname.", a_soc);
	} else {
		firstFree = CHATfirstEmptySlot();
		if(firstFree>=0) {
			CHATclearActiveRooms(&firstFree);
			CHATassignUser(&firstFree, a_soc, cmd->param);
			//notify all users
			snprintf(controlMessage, 128, "User %s has logged in.", cmd->param);
			CHATsendCtrlMessageToAll(&mainRoom, room.param, controlMessage);
			//add to main room
			CHATjoinToRoom(&room, a_soc);
			CHATsendReply(101, cmd->param, a_soc);
			//TODO send new user lists.
			CHATsendUserListToAll();
			DEBUGprintUsers();
		} else {
			CHATsendReply(502, "There are no empty slots available. Try again later.", a_soc);
		}
	}
	free(room.param);
	free(controlMessage);
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
	for(i=0; i<MAX_ROOMS; i++) {
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

int CHATassignToRoom(int a_id, int * a_fd) {
	int success = 1, k;
	IPCp(GLOBALsemid,1);
	if((*SHM).tabRoom[a_id].users==MAX_USERS) {
		success=0;
	} else {
		(*SHM).tabRoom[a_id].id = a_id+1; //TODO
		(*SHM).tabRoom[a_id].users++;
		for(k=0; k<MAX_USERS; k++) {
			if((*SHM).tabRoom[a_id].activeUsers[k]==0) {
				(*SHM).tabRoom[a_id].activeUsers[k] = *a_fd;
				break;
			}
		}
	}
	IPCv(GLOBALsemid,1);
	return success;
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
		(*SHM).tabRoom[freeSlot].users=1;
	}
	IPCv(GLOBALsemid,1);

	return freeSlot;
}

void CHATuserAddRoom( int * a_pos , int * a_roomPos ) {
	int i=0;
	IPCp(GLOBALsemid,0);
	for(i=0; i<MAX_ROOMS; i++) {
		if((*SHM).tabUser[*a_pos].activeRooms[i]==-1) {
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
				if(CHATassignToRoom(roomPos, a_soc)) {
					CHATuserAddRoom(&pos, &roomPos);
					printf("chat add to room: %d\n", roomPos);
					DEBUGprintRoom(0);
				} else {
					CHATsendReply(503, "Room is full.", a_soc);
				}
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

int CHATfindRoom (char * a_name) {
	int pos = -1, i;
	IPCp(GLOBALsemid,1);
	for(i=0; i<MAX_ROOMS; i++) {
		if(!strcmp((*SHM).tabRoom[i].name,a_name)) {
			pos = i;
			break;
		}
	}
	IPCv(GLOBALsemid,1);
	return pos;
}


void CHATsendToAll (int * a_roomId , char* a_roomName, int * a_sender, char* a_message) {
	int users[MAX_USERS], i=0, k=0;
	k = CHATgetActiveUsers(a_roomId, users);
	
	char * nick = malloc(32);
	bzero(nick ,32);

	IPCp(GLOBALsemid,0);
	strncpy(nick,(*SHM).tabUser[*a_sender].nick,32);
	IPCv(GLOBALsemid,0);

	printf("do ilu wysle: %d\n", k);
	for(i=0; i<k; i++) {
		CHATsendMessage(0, &(users[i]), nick, a_roomName, a_message);
	}
	free(nick);
}

void CHATsendPrivate (int * a_sender, int *a_receiver, char* a_message) {
	int fd_receiver, fd_sender;
	char * nick = malloc(32);
	char * nick_r = malloc(32);
	bzero(nick,32);
	bzero(nick_r ,32);
	IPCp(GLOBALsemid,0);
	strncpy(nick,(*SHM).tabUser[*a_sender].nick,32);
	strncpy(nick_r,(*SHM).tabUser[*a_receiver].nick,32);
	fd_sender = (*SHM).tabUser[*a_sender].fd;
	fd_receiver = (*SHM).tabUser[*a_receiver].fd;
	IPCv(GLOBALsemid,0);

	CHATsendMessage(1, &fd_receiver, nick, nick, a_message); //send to receiver
	CHATsendMessage(1, &fd_sender, nick, nick_r, a_message); //send to sender

	free(nick);
	free(nick_r);
}




void CHATparseMessage(char* a_message, int * a_soc) {
	struct CHATcommand * cmd;
	if(a_message[0]=='%' && a_message[1]=='2' && a_message[2]=='F') {
		//%40 - urldecode(/)
		cmd = CHATdecodeCommand(a_message, cmd);
		if(cmd->commandId>0) {
			CHATexecuteCommand(cmd, a_soc);
		} else {
			perror("Unknown command.");
		}
		free(cmd->param);
		free(cmd->name);
		free(cmd);
	} else if(a_message[0]=='%' && a_message[1]=='4' && a_message[2]=='0') {
		//%40 - urldecode(@)
		cmd = CHATdecodeCommand(a_message, cmd);
		int idSender = CHATisLogged(NULL, a_soc);
		int idReceiver = CHATisLogged(cmd->name, NULL);
		if(idReceiver>=0) {
			printf("\nprivate:\n%s\n", a_message);
			CHATsendPrivate(&idSender, &idReceiver, cmd->param);
		} else {
			//cant send private message
		}
		free(cmd->param);
		free(cmd->name);
		free(cmd);
	} else if(a_message[0]=='%' && a_message[1]=='2' && a_message[2]=='5' ) {
		//%25 - urldecode(%)
		cmd = CHATdecodeCommand(a_message, cmd);
		int idRoom = CHATroomExists(cmd->name);
		int idSender = CHATisLogged(NULL, a_soc);
		if(idRoom>=0) {
			CHATsendToAll(&idRoom, cmd->name, &idSender, cmd->param);
		}
		printf("public:\n%s\n\n", a_message);
		printf("room name: %s\n", cmd->name);

		free(cmd->param);
		free(cmd->name);
		free(cmd);
	} else {
		//dafuq?
	}
}