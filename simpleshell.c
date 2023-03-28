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
            // This is the main process that will spawn a child process to execute the command
            printf("I'm in the main process.\n");

            // Create a child process using fork
            pid_t pid = fork();

            // If pid > 0, this is the parent process
            if (pid > 0) {
                printf("I'm the parent, waiting for the child process.\n");
                // The parent waits for the child process to complete its execution
                waitpid(pid, NULL, 0);
                printf("Parent is done waiting.\n");
            }
            // If pid == 0, this is the child process
            else if (pid == 0) {
                printf("I'm the child, executing the command.\n");

                int input = -1, output = -1;

                // Loop through the arguments to look for input/output redirection symbols
                for (int i = 0; i < numArgs; i++) {
                    // If the argument is '<', set up input redirection
                    if (strcmp(args[i], "<") == 0) {
                        args[i] = NULL;
                        // Open the input file
                        input = open(args[i + 1], O_RDONLY);
                        if (input < 0) {
                            perror("Error opening input file");
                            exit(1);
                        }
                        // Duplicate the file descriptor for input to STDIN_FILENO
                        dup2(input, STDIN_FILENO);
                        // Close the original file descriptor
                        close(input);
                    }
                    // If the argument is '>', set up output redirection
                    else if (strcmp(args[i], ">") == 0) {
                        args[i] = NULL;
                        // Open the output file
                        output = open(args[i + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
                        // If the output file cannot be opened, print an error message and exit
                        if (output < 0) {
                            perror("Error opening output file");
                            exit(1);
                        }
                        // Duplicate the file descriptor for output to STDOUT_FILENO
                        dup2(output, STDOUT_FILENO);
                        close(output);
                    }
                }
                // Execute the command
                execvp(args[0], args);
                // If the execvp function returns, it means that the command was not found
                perror("Error executing command");
                exit(1);
                // If pid < 0, there was an error creating the child process
            } else {
                perror("Error creating child process");
                exit(1);
            }
        }
    }
    // Exit the shell
    return 0;
}
