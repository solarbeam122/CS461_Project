#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/errno.h>
#define MAX_LINE 80 /* The maximum length command */

// This function takes a character buffer and parses it into tokens using strtok
// It returns the number of arguments read
int parse(char *buff, char *argv[]) {
    int j = -1;
    char *point;
    point = strtok(buff, " ");
    while (point != NULL) {
        j++;
        argv[j] = point;
        point = strtok(NULL, " ");
    }
    argv[j + 1] = '\0';
    return (j);
}

int main() {
    // This is the main loop that will run until the user exits the shell
    while (1) {
        // Read the user's command and store it in the 'command' variable
        char command[MAX_LINE];
        char cwd[1024];
        getcwd(cwd, sizeof(cwd));
        printf("%s > ", cwd);
        fgets(command, MAX_LINE, stdin);
        command[strlen(command) - 1] = '\0';

        // Parse the user's command and store the arguments in the 'args' array
        char *args[MAX_LINE / 2 + 1];
        int numArgs = parse(command, args);

        // Check if the command is 'cd' (change directory)
        if (strcmp(args[0], "cd") == 0) {
            // If there's no directory specified, change to the home directory
            if (args[1] == NULL) {
                chdir(getenv("HOME"));
            } else {
                // Change to the specified directory
                chdir(args[1]);
            }
        } else {
            // This is the main process that will spawn two child processes to execute the commands separated by '|'
            printf("I'm in the main process.\n");

            // Find the pipe symbol '|' in the user's command and split the commands
            char *cmd1[MAX_LINE / 2 + 1];
            char *cmd2[MAX_LINE / 2 + 1];
            int numCmd1 = 0, numCmd2 = 0;
            int i = 0;
            while (args[i] != NULL) {
                if (strcmp(args[i], "|") == 0) {
                    cmd1[numCmd1] = NULL;
                    numCmd2 = numArgs - numCmd1 - 1;
                    for (int j = 0; j < numCmd2; j++) {
                        cmd2[j] = args[i + j + 1];
                    }
                    break;
                } else {
                    cmd1[numCmd1] = args[i];
                    numCmd1++;
                }
                i++;
            }
            cmd1[numCmd1] = NULL;
            cmd2[numCmd2] = NULL;

                        // Create a pipe to connect the two child processes
            int fd[2];
            if (pipe(fd) == -1) {
                perror("Error creating pipe");
                exit(1);
            }
            // Create the first child process to execute the first command
            pid_t pid1 = fork();
            if (pid1 == 0) {
                printf("I'm the first child, executing the first command.\n");

                // Redirect the output of the first command to the write end of the pipe
                dup2(fd[1], STDOUT_FILENO);
                close(fd[0]);
                close(fd[1]);

                // Execute the first command
                execvp(cmd1[0], cmd1);
                // If the execvp function returns, it means that the command was not found
                perror("Error executing command");
                exit(1);
            } else if (pid1 < 0) {
                perror("Error creating first child process");
                exit(1);
            }

            // Create the second child process to execute the second command
            pid_t pid2 = fork();
            if (pid2 == 0) {
                printf("I'm the second child, executing the second command.\n");

                // Redirect the input of the second command to the read end of the pipe
                dup2(fd[0], STDIN_FILENO);
                close(fd[0]);
                close(fd[1]);

                // Execute the second command
                execvp(cmd2[0], cmd2);
                // If the execvp function returns, it means that the command was not found
                perror("Error executing command");
                exit(1);
            } else if (pid2 < 0) {
                perror("Error creating second child process");
                exit(1);
            }

            // Close the file descriptors used by the parent process
            close(fd[0]);
            close(fd[1]);

            // Wait for both child processes to complete their execution
            waitpid(pid1, NULL, 0);
            waitpid(pid2, NULL, 0);

            printf("Parent is done waiting.\n");
        }
    }

    // Exit the shell
    return 0;
}


           
