#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <pwd.h>
#include <libgen.h>
#include <sys/types.h>
#include <sys/wait.h>

#define BUFFERSIZE 128

// first cache the user inputs & shell command possible paths
static char *inputs[BUFFERSIZE], *paths[BUFFERSIZE];

// ****************************************************************
// * This function removes a substring from a string.			  *
// ****************************************************************
void rm_substr(char *str, const char *substr)
{
  while((str = strstr(str, substr)))
    memmove(str, strlen(substr) + str, strlen(strlen(substr) + str) + 1);
}


// ****************************************************************
// * This function prepares the prompt and print it.			  *
// ****************************************************************
void print_prompt(void) 
{
    char hostname[BUFFERSIZE];
    char *path = calloc(BUFSIZ, sizeof(char*));
    char *username = calloc(BUFFERSIZE, sizeof(char*));
    
    gethostname(hostname, sizeof(hostname));
    rm_substr(hostname, ".local");
    
    getcwd(path, BUFSIZ);
    path = strrchr(path, '/') + 1;
    
    username = getlogin();
    
    //printf("%s@%s: %s$ ", username, hostname, path);
    printf("%s:%s %s$ ", hostname, path, username);
}


// ****************************************************************        ????????
// * This function enables the use of the ctrl+c command to 	  *
// * interrupt whatever the shell is doing. Also, it is used to   *
// * print the promp for the first time.						  *
// ****************************************************************
void sig_hdlr(int signo)  // call print_prompt() the first time
{
	if (signo != 0) {
		printf("\n");
    }
	print_prompt();
	fflush(stdout);
}


// ****************************************************************
// * This function creates a child process and executes most of   *
// * the basic commands that comes with the Operating System.     *
// ****************************************************************
void cmd_execute(char* cmd, char **envp)
{
    if (!fork()) {
        if (system(cmd) != 0) {
            printf("$-Command Not Found: %s\n", cmd);
        }    
	exit(0);
    } else {
        wait(NULL);
    }
}


// ****************************************************************
// * This function parses the input into an array of strings and  *
// * returns the input command so that it can be called	by other  *
// * functions.													  *
// ****************************************************************
char *prepare_inputs(char *input) 
{
    char *token, *cmd;
    
    // store each word of the input string into an array of strings
    int i = 0;
	while ((token = strsep(&input, " "))) {
		inputs[i] = calloc(strlen(token) + 1, sizeof(char*));
		strncat(inputs[i], token, strlen(token));
		i++;
	}
    
    
	// return the first word of the input string as the command to be performed
	cmd = strncat(inputs[0], "\0", 1);

	return cmd;
}


// ****************************************************************      ???????
// * This function finds all paths in which shell commands might  *
// * be stored in the system.									  *
// ****************************************************************
void get_paths(char **envp_helper) {
	int i = 0;
	char *token;
	char *path_str;

	// find all possible paths from environment variable
	path_str = strstr(envp_helper[i], "PATH");
	while(!path_str) {
		i++;
		path_str = strstr(envp_helper[i], "PATH");
	}

	i = 0;
	path_str += 5;

	// separate result found above into a static array of strings
	while ((token = strsep(&path_str, ":"))) {
		paths[i] = calloc(strlen(token) + 1, sizeof(char*));
		strncat(paths[i], token, strlen(token));
		strncat(paths[i], "/", 1);
		i++;
	}
	free(token);
}


// ****************************************************************     ????????
// * This function finds the path where the input command is 	  *
// * stored in the system.										  *
// ****************************************************************
void get_cmd_pth(char *cmd)
{
	int i;
	char *path = calloc(BUFFERSIZE, sizeof(char*));
	FILE *file;

	// iterate over all possible paths
	for(i = 0; paths[i]; i++) {
		strcpy(path, paths[i]);
		strncat(path, cmd, strlen(cmd));

		// get the right path by testing it existence
		if((file = fopen(path, "r"))) {
			strncpy(cmd, path, strlen(path));
			fclose(file);
		}
	}
	free(path);
}


// ****************************************************************
// * This function implements the change directory (cd) command.  *
// ****************************************************************
void cd(char *path)
{
    char last_backslash[BUFSIZ];
	char path_fix[BUFSIZ];
	char cwd[BUFSIZ];
	char *cwdptr = calloc(BUFSIZ, sizeof(char*));
    
    // condition for when there is no slash in the argument
    if (strncmp(path, "/", 1) != 0 && strncmp(path, "..", 1) != 0) {
        getcwd(cwdptr, BUFSIZ);
        strncat(cwdptr, "/", 1);
        strncat(cwdptr, path, strlen(path));
        strncat(cwdptr, "\0", strlen(path));
        strncpy(cwd, cwdptr, BUFSIZ);
        if (chdir(cwd) == -1) {
            perror("chdir");
        }
    } else if (strncmp(path, "..", 2) == 0) {  // condition for when the argument is '..'
        getcwd(cwd, sizeof(cwd));
        strncpy(last_backslash, strrchr(cwd, '/'), BUFFERSIZE);
        cwd[strlen(cwd)-strlen(last_backslash)] = '\0';
        if (chdir(cwd) == -1) {
            perror("chdir");
        }
    } else {
        // condition for when the argument is either a full path or there
		// is no need to append a slash in the argument
        if (chdir(path) == -1) {
			getcwd(cwdptr, BUFSIZ);
			strncat(cwdptr, path, strlen(path));
			strncat(cwdptr, "\0", strlen(path));
			strncpy(cwd, cwdptr, BUFSIZ);
			if(chdir(cwd) == -1) {
				perror("chdir");
            }
		}
    }
}


// ****************************************************************      ???????
// * This function frees the memory of all elements of a char	  *
// * array.														  *
// ****************************************************************
void free_arr(char **arr)
{
	int i;

	// empty all elements of array, set it to null and then set it free
	for(i = 0 ; arr[i]; i++) {
		memset(arr[i], 0, strlen(arr[i]));
		arr[i] = NULL;
		free(arr[i]);
	}
}


// ****************************************************************
// * This function clears the screen, for our new shell to open   *
// * and prints the prompt for the first time.  				  *
// * Also, it gets all the paths where the system commands may be *
// * stored, by calling get_paths().							  *
// ****************************************************************
void initilize(char **envp)
{
	char *cmd = calloc(BUFFERSIZE, sizeof(char*));

	get_paths(envp);
	system("clear");
	sig_hdlr(0);
	free_arr(inputs);
}




// ****************************************************************
// * The main function puts everything together so that the shell *
// * can work.													  *
// ****************************************************************
int main(int argc, char** argv, char** envp)
{
    int fd, i;
	char c;
	char *input_str = calloc(BUFFERSIZE, sizeof(char*));
	char *cmd = calloc(BUFFERSIZE, sizeof(char*));
    
	// ignore the standard interrupt signal (ctrl+c)
	signal(SIGINT, SIG_IGN);
	// assign interrupt signal (ctrl+c) to the signal handler function
	signal(SIGINT, sig_hdlr);
    
    // prepare screen and fetch necessary data
	initilize(envp);
    
    while (1) {
        c = getchar();
        
        if (c == '\n') {
	    if (strcmp(input_str, "exit") == 0 || c == EOF) {
		break;
	    }
	    if (strcmp(input_str, "quit") == 0) {
		break;
	    }
            if (input_str[0] != '\0') {
                // erase cmd variable
                memset(cmd, 0, BUFFERSIZE);
                // parse the command line
                cmd = prepare_inputs(input_str);
                if (strncmp(cmd, "cd", 2) == 0) {
                    cd(inputs[1]);
                } else {
                    get_cmd_pth(cmd);
                    cmd_execute(cmd, envp);
                }
                free_arr(inputs);
            }
            // print the prompt again after hitting enter
            print_prompt();
            memset(input_str, 0, BUFFERSIZE);
        } else {
            strncat(input_str, &c, 1);
        }
    }
    
    free(cmd);
    free(input_str);
    free_arr(paths);
    
    // print new line if 'ctrl+d' is pressed
	if(c == EOF) printf("\n");
    
    // end of main
	return 0;
}
