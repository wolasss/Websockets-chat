#ifndef IPC_STRUCT
#define IPC_STRUCT
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>

#define USER_LENGTH 32
#define MAX_USER 20
#define MAX_ROOM 10

struct User {
    char nick[USER_LENGTH];
    int activeRooms[MAX_ROOM];
    int fd;
};

struct Room {
    unsigned int id;
    char name[USER_LENGTH];
    int activeUsers[MAX_USER];  // fd of users
    int users;  // how many
};

struct Shared {
    struct User tabUser[MAX_USER];
    struct Room tabRoom[MAX_ROOM];
};

void SHMinit(int a_sid);
void SHMdestroy();
void IPCv(int a_semid, int a_semnum);
void IPCp(int a_semid, int a_semnum);

#endif
