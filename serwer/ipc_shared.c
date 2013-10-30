#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include "ipc_shared.h"

extern int GLOBALsemid;

void IPCv(int a_semid, int a_semnum) {
	struct sembuf buf;
	buf.sem_num = a_semnum;
	buf.sem_op = 1;
	buf.sem_flg = 0;
	printf("go go up\n");
	if (semop(a_semid, &buf, 1) == -1){
		perror("Sem up error: ");
		exit(1);
	}
}

void IPCp(int a_semid, int a_semnum){
	struct sembuf buf;
	buf.sem_num = a_semnum;
	buf.sem_op = -1;
	buf.sem_flg = 0;
	printf("go go %d\n", semctl(GLOBALsemid, 0, GETVAL, (int)1));

	if (semop(a_semid, &buf, 1) == -1){
		
		perror("Sem down error: ");
		exit(1);
	}
}

void SHMinit(int a_sid, struct Shared * a_shm) {
	int shmid;

	if( (shmid = shmget(a_sid, sizeof(*a_shm), IPC_CREAT|0666))<0 ) {
		perror("Creating segment of sahred memory. ");
		exit(1);
	}
	
	if( (a_shm = shmat(shmid, NULL, 0)) == NULL) {
		perror("Joining segment of sahred memory. ");
		exit(1);
	}

	if( (GLOBALsemid = semget(a_sid, 3, IPC_CREAT|IPC_EXCL|0666)) == -1 ) {
		if( (GLOBALsemid = semget(a_sid, 3, 0666)) == -1 ) {
			perror("Error creating sem table");
			exit(1);
		}
	} else {
		 	if (semctl(GLOBALsemid, 0, SETVAL, (int)1) == -1) {
		 		perror("Sem 1 (users)"); 
		 		exit(1);
		 	} 
			if (semctl(GLOBALsemid, 1, SETVAL, (int)1) == -1) {
				perror("Sem 2 (room)"); 
				exit(1);
			}
			if (semctl(GLOBALsemid, 2, SETVAL, (int)1) == -1) {
				perror("Sem 3 (log file)"); 
				exit(1);
			} 
	}
	printf("sem if %d\n", GLOBALsemid);
}