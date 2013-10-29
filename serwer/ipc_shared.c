#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include "ipc_shared.h"

void SHMv(int a_semid, int a_semnum) {
	buf.sem_num = a_semnum;
	buf.sem_op = 1;
	buf.sem_flg = 0;
	if (semop(a_semid, &buf, 1) == -1){
		perror("Sem up error: ");
		exit(1);
	}
}

void SHMp(int a_semid, int a_semnum){
	buf.sem_num = a_semnum;
	buf.sem_op = -1;
	buf.sem_flg = 0;
	if (semop(a_semid, &buf, 1) == -1){
		perror("Sem down error: ");
		exit(1);
	}
}

void SHMinit(int a_sid, struct Shared * a_shm) {
	int shmid, semid;

	if( (shmid = shmget(a_sid, sizeof(*a_shm), IPC_CREAT|0666))<0 ) {
		perror("Creating segment of sahred memory. ");
	}
	
	if( (a_shm = shmat(shmid, NULL, 0)) == NULL) {
		perror("Joining segment of sahred memory. ");
	}

	if( (semid = semget(a_sid, 4, IPC_CREAT|IPC_EXCL|0666)) == -1 ) {
		if( (semid = semget(a_sid, 4, 0666)) == -1 ) {
			perror("Error creating sem table");
		}
	} else {
			if (semctl(semid, 0, SETVAL, (int)1) == -1) {
				perror("Sem 0 (serwer)"); 
			} 
		 	if (semctl(semid, 1, SETVAL, (int)1) == -1) {
		 		perror("Sem 1 (users)"); 
		 	} 
			if (semctl(semid, 2, SETVAL, (int)1) == -1) {
				perror("Sem 2 (room)"); 
			}
			if (semctl(semid, 3, SETVAL, (int)1) == -1) {
				perror("Sem 3 (log file)"); 
			} 
	}
}