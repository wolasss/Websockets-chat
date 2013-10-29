#ifndef IPC_STRUCT
#define IPC_STRUCT
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <stdlib.h>

#define MAX_USER 20
#define MAX_ROOM 10

struct sembuf buf;

struct User {
	char nick[17];
	int fd;
};

struct Room {
	unsigned int id;
	char name[17];
};

struct Shared {
	struct User tabUser[MAX_USER];
	struct Room tabRoom[MAX_ROOM];
};

void SHMinit(int a_sid, struct Shared * a_shm);
void SHMv(int a_semid, int a_semnum);
void SHMp(int a_semid, int a_semnum);

#endif