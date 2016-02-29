/*
 * ==================================================================
 *       Filename:  myShell.c
 *    Description:  A simple shell program in C
 *        Version:  1.0
 *        Created:  12/02/2016
 *         Author:  Jinglong Li, N13903873
 * ==================================================================
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/wait.h>

void delete_new_line(char* cmd);
void cmd_to_params(char* cmd, char** params);
int cmd_exec(char** params);
void print_prompt(void);
void free_array(char **arr);

#define BUFFERSIZE 128
#define MAX_CMD_LEN 200
#define MAX_PARAMS_AMOUNT 20

// ****************************************************************
// * Main Function                                   			  *
// ****************************************************************
int main()
{
    char cmd[MAX_CMD_LEN + 1];   // Store command
    char* params[MAX_PARAMS_AMOUNT + 1];   // Store parameters after parsing

    // Loop to wait for user's input
    while(1) {
        // Print command prompt
        print_prompt();

        // Use fgets() to read command
        if(fgets(cmd, sizeof(cmd), stdin) == NULL) {
            break;
        }

        // Delete the "\n" at the end
        delete_new_line(cmd);

        // Split cmd into an array of parameters
        cmd_to_params(cmd, params);
        
        // Exit or Quit:
        if (strcmp(params[0], "exit") == 0 || strcmp(params[0], "quit") == 0) {
            break;
        } else {
            cmd_exec(params); // Execute command
        }
    }

    // Avoid memory leak:
    free_array(params);
    
    // End of main
    return 0;
}


// ****************************************************************
// * This function print the command prompt         			  *
// ****************************************************************
void print_prompt() 
{
    char *username = calloc(BUFFERSIZE, sizeof(char*));
    username = getlogin();
    printf("%s@myShell ==> ", username);
}


// ****************************************************************
// * Split cmd into array of parameters by blank      			  *
// ****************************************************************
void cmd_to_params(char* cmd, char** params)
{
    int i = 0;
    while (i < MAX_PARAMS_AMOUNT) {
        params[i] = strsep(&cmd, " "); // split
        if (params[i] == NULL) {
            break;
        }
        i++;
    }
}

// ****************************************************************
// * This function deletes the "\n" at the end of command         *
// ****************************************************************
void delete_new_line(char* cmd)
{
    if(cmd[strlen(cmd)-1] == '\n') {
        cmd[strlen(cmd)-1] = '\0';
    }
}

// ****************************************************************
// * This function creates a child process and executes most of   *
// * the basic commands.                                          *
// ****************************************************************
int cmd_exec(char** params)
{
    if (!fork()) {  // Child process
      execvp(params[0], params); // Execute the cmd
        //char* error = strerror(errno); // If errors 
        printf("myShell: Command Not Found: %s\n", params[0]);
        exit(0);  // Exit the child process
    } else {  // Parent process
        wait(NULL); // Wait for all child processes
    }
    return 0;
}

// ****************************************************************
// * This function frees the memory of all elements of a char	  *
// * array.														  *
// ****************************************************************
void free_array(char **arr)
{
	int i;
	// empty all elements of array, set it to null and then set it free
	for(i = 0 ; arr[i]; i++) {
		memset(arr[i], 0, strlen(arr[i]));
		arr[i] = NULL;
		free(arr[i]);
	}
}



