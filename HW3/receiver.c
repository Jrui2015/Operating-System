/*
This is the first program receiver.c runs in an infinite loop receiving alpha numeric strings as input from the user,
one line at a time. After reading one line from the standard input, this program sends this information to the
other program only if the line contains the secret code "C00L". The sharing of data between the two processes
takes place via shared memory.
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/sem.h>

#define SHAREM_SIZE 2048  // shared memory size: 2048

int main(void)
{
	key_t key = getuid(); // get user id
	int semaphore_id;  // store semaphore id
	int sem_value = 0;  // store semaphore value
	int shmid;  // shared memory id
	char *sharem = NULL; // point to the shared memory

	/* create semaphore buffer */
	struct sembuf sem_buffer;  // construct the semaphore buffer
	sem_buffer.sem_num = 0;
	sem_buffer.sem_flg = SEM_UNDO;

	char *user_input = malloc(sizeof(char) * BUFSIZ);  // store the user's input

	/* use semget() to use key to get the semaphore id */
	semaphore_id = semget(key, 1, 0666 | IPC_CREAT);

	if (semaphore_id == -1) { // if failed
		perror("semget failed: fail to create semaphore");
		exit(EXIT_FAILURE);
	}

	/* set semaphore value and validate it */
	if (semctl(semaphore_id, 0, SETVAL, 0) == -1) {
		printf("%s\n", "semaphore inits error");
		/* if semaphore inits error, just delete semaphore signal */
		if (semctl(semaphore_id, 0, IPC_RMID, 0) != 0) {
			perror("semctl failed: failt to delete semaphore signal");
			exit(EXIT_FAILURE);
		}
		exit(EXIT_FAILURE);
	}

	/* use shmget() to get the shared memory id */
	shmid = shmget(key, SHAREM_SIZE, 0666 | IPC_CREAT);

	if (shmid == -1) { // if failed
		perror("shmget failed: fail to create shared memory segment");
		exit(EXIT_FAILURE);
	}

	/* put the shared memory in real use */
	sharem = shmat(shmid, NULL, 0);

	if (sharem == (char *) -1) { // if failed
		perror("shmat failed: fail to connect segment to data space");
		exit(EXIT_FAILURE);
	}

	/* infinite loop to wait for user's input */
	while (1)
	{
		/* only if the semaphore value is 0, we do below */
		if ((sem_value = semctl(semaphore_id, 0, GETVAL)) == 0) {
			printf("%s\n", "Please enter an alpha numeric string: (enter \"quit\" to quit)");

            /* get alpha numeric strings from the user one line at a time and then validate through user_input*/
            scanf("%s", user_input);

			/* see if the input is quit */
            if (strncmp(user_input, "quit", 4) == 0) {
                memcpy(sharem, user_input, strlen(user_input) + 1); // also tell the processor to quit
				break; // jump the loop
			}

			/* if there is no "C00L" in user's input, just ignore the input */
            if (strstr(user_input, "C00L") == NULL) {
				continue;
			}

			/* if there is "C00L" in input, we can copy this to the shared memory */
            memcpy(sharem, user_input, strlen(user_input) + 1);

			/* let the "sem_op" in sem_buffer struct to be 1 so we send the signal insead of waitting */
            sem_buffer.sem_op = 1;

            if (semop(semaphore_id, &sem_buffer, 1) == -1) { // if failed
                fprintf(stderr, "semaphore operation failed.\n");
                exit(EXIT_FAILURE);
            }
		}
	}

	/* terminate the use of shared memory */
	if (shmdt(sharem) == -1) { // if failed
		perror("shmdt failed: segment didn't terminate connection to data space");
		exit(EXIT_FAILURE);
	}

	/* terminate the IPC(inter process communication) */
	if (shmctl(shmid, IPC_RMID, 0) == -1) {
		perror("shmctl failed: ipc didn't close");
		exit(EXIT_FAILURE);
	}
}
