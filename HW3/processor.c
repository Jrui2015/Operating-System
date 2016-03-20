/*
The second program processor.c creates an output file secrets.out and waits for user input
to be sent by the receiver program. As soon as one line is received from the receiver, it
counts the number of digits in that line and dumps the digit count along with the original
line in the secrets.out file. This program also runs in an infinite loop.
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>

#define SHAREM_SIZE 2048 // shared memory size: 2048

int main(void)
{
    key_t key = getuid(); // get user id
    int shmid;  // shared memory id
    int semaphore_id; // store semaphore id
    int sem_value; // store semaphore value
	int count; // count the number of digits
    char *sharem = NULL; // point to the shared memory
	FILE * file = NULL; // for manipulate the secrets.out file

    /* create semaphore buffer */
    struct sembuf sem_buffer; // construct the semaphore buffer
    sem_buffer.sem_num = 0;
    sem_buffer.sem_flg = SEM_UNDO;

    file = fopen("./secrets.out", "a+"); // use append mode to append data

    if (file == NULL) { // if failed
        perror("fopen error: fail to open the file");
        exit(EXIT_FAILURE);
    }

    //* use semget() to use key to get the semaphore id */
	semaphore_id = semget(key, 1, 0666 | IPC_CREAT);

	if (semaphore_id == -1) { // if failed
		perror("semget failed: fail to create semaphore");
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

    /* infinite loop to wait for data refresh in the shared memory */
    while (1)
    {
        /* only if the semaphore value is 1, we do below */
        if ((sem_value = semctl(semaphore_id, 0, GETVAL)) == 1) {
            printf("Data received by processor.c: "); // print out the data that the processor.c find in shared memory
            printf("%s\n", sharem);

            count = 0; // set count to 0 every time

            /* count the amount of digits */
            char *ptr = sharem; // set a pointer to the begin of the shared memory
            char *end = ptr + strlen(sharem); // find the end of the data in shared memory
            for (ptr = sharem; ptr != end; ptr++) { // go through the data
                if (isdigit(*ptr)) { // if the char is digit, then count++
                    count++;
                }
            }

            /* output the Original Line and the Digits amount in it to "secrets.out" */
            fprintf(file, "Count Digits: %d  Original: ", count);
            fwrite(sharem, 1, strlen(sharem), file);
            fwrite("\n", 1, 1, file);

            /* flush the memory to refresh the data change to the file immediately */
            fflush(file);

            /* let the "sem_op" in sem_buffer struct to be 1 so it is in waitting condition */
            sem_buffer.sem_op = -1;
            if (semop(semaphore_id, &sem_buffer, 1) == -1) { // if failed
                perror("semop error: semaphore operation failed");
                exit(EXIT_FAILURE);
            }
        }

        /* if "quit" is in shared memory, then quit */
        if (strncmp(sharem, "quit", 4) == 0)
        {
            break;
        }
    }

    fclose(file); // close the file "secrets.out"

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
