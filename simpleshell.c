#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/errno.h>
#include <sys/stat.h>


#define MAX_LINE 80 /* The maximum length command */

// This function takes a character buffer and parses it into tokens using strtok
// It returns the number of arguments read, including the command itself
int parse(char *buff, char *argv[]) {
    int j = -1;     // j is the index of the current argument
    char *point;    // point is a pointer to the current token
    point = strtok(buff, " ");  // strtok returns a pointer to the next token in the buffer
    while (point != NULL) {    // while there are still tokens in the buffer
        j++;    // increment the index of the current argument
        argv[j] = point;   // store the pointer to the token in the argv array
        point = strtok(NULL, " "); // get the next token
    }
    return (j + 1); // return the number of arguments read
}

int main() {
    // This is the main loop that will run until the user exits the shell
    while (1) { // repeat forever
        // Read the user's command and store it in the 'command' variable
        char command[MAX_LINE]; // command is a character buffer that will store the user's command
        char cwd[1024]; // cwd is a character buffer that will store the current working directory
        getcwd(cwd, sizeof(cwd)); // get the current working directory
        printf("%s > ", cwd); // print the current working directory
        fgets(command, MAX_LINE, stdin); // read the user's command from the standard input
        command[strlen(command) - 1] = '\0'; // replace the newline character with a null character

        // Parse the user's command and store the arguments in the 'args' array
        char *args[MAX_LINE / 2 + 1]; // args is an array of character pointers that will store the arguments
        int numArgs = parse(command, args); // parse the user's command and store the number of arguments read

        // Check for the cd command
        if (strcmp(args[0], "cd") == 0) { // strcmp compares two strings and returns 0 if they are equal
            // If there's no directory specified, change to the home directory
            if (args[1] == NULL) { // args[1] is the first argument after the command
                chdir(getenv("HOME")); // getenv returns the value of the specified environment variable
            } else { 
                // Change to the specified directory
                if (chdir(args[1]) != 0) { // chdir returns 0 if the directory was changed successfully
                    perror("Error changing directory"); // perror prints the error message corresponding to the error number stored in errno
                }
            }
            continue; // skip the rest of the loop
        }

        // Check for the pwd command
        if (strcmp(args[0], "pwd") == 0) { // strcmp compares two strings and returns 0 if they are equal
            printf("%s\n", cwd); // print the current working directory
            continue; // skip the rest of the loop
        }

        // Check for the mkdir command
        if (strcmp(args[0], "mkdir") == 0) { // strcmp compares two strings and returns 0 if they are equal
            if (args[1] == NULL) { // args[1] is the first argument after the command
                printf("mkdir: missing operand\n"); // print an error message
            } else {
                if (mkdir(args[1], 0777) != 0) { // mkdir returns 0 if the directory was created successfully
                    perror("Error creating directory"); // perror prints the error message corresponding to the error number stored in errno
                }
            }
            continue; // skip the rest of the loop
        }

        // Check for the echo command
        if (strcmp(args[0], "echo") == 0) { // strcmp compares two strings and returns 0 if they are equal
            // Print each argument separated by a space
            for (int i = 1; i < numArgs; i++) { // args[0] is the command, so we start at args[1]
                printf("%s ", args[i]); // print the argument
            }
            printf("\n"); // print a newline character
            continue; // skip the rest of the loop
        }

        // This is the main process that will spawn two child processes to execute the commands separated by '|'
        printf("I'm in the main process.\n"); // print a message to indicate that we are in the main process

        // Find the pipe symbol '|' in the user's command and split the commands
        char *cmd1[MAX_LINE / 2 + 1]; // cmd1 is an array of character pointers that will store the arguments of the first command
        char *cmd2[MAX_LINE / 2 + 1]; // cmd2 is an array of character pointers that will store the arguments of the second command
        int numCmd1 = 0, numCmd2 = 0; // numCmd1 is the number of arguments in the first command, numCmd2 is the number of arguments in the second command
        int i = 0; // i is the index of the current argument
        while (args[i] != NULL) { // while there are still arguments in the user's command
            if (strcmp(args[i], "|") == 0) { // strcmp compares two strings and returns 0 if they are equal
                cmd1[numCmd1] = NULL; // set the last argument of the first command to NULL
                numCmd2 = numArgs - numCmd1 - 1; // calculate the number of arguments in the second command
                for (int j = 0; j < numCmd2; j++) { // copy the arguments of the second command
                    cmd2[j] = args[i + j + 1]; // copy the arguments of the second command
                }
                break; // stop parsing the user's command
            } else { // if the current argument is not the pipe symbol '|'
                cmd1[numCmd1] = args[i]; // copy the argument to the first command
                numCmd1++; // increment the number of arguments in the first command
            }
            i++; // increment the index of the current argument
        }
        cmd1[numCmd1] = NULL; // set the last argument of the first command to NULL
        cmd2[numCmd2] = NULL; // set the last argument of the second command to NULL

        // Create a pipe to connect the two child processes
        int fd[2]; // fd is an array of two integers that will store the file descriptors of the read and write ends of the pipe
        if (pipe(fd) == -1) { // pipe returns -1 if an error occurred
            perror("Error creating pipe"); // perror prints the error message corresponding to the error number stored in errno
            exit(1); // exit the program
        }

        // Create the first child process to execute the first command
        pid_t pid1 = fork(); // fork returns the process ID of the child process to the parent process and 0 to the child process
        if (pid1 == 0) { // if we are in the child process
            printf("I'm the first child, executing the first command.\n"); // print a message to indicate that we are in the first child process

            // Redirect the output of the first command to the write end of the pipe
            dup2(fd[1], STDOUT_FILENO); // dup2 duplicates the file descriptor specified in the first argument to the file descriptor specified in the second argument
            close(fd[0]); // close the read end of the pipe
            close(fd[1]); // close the write end of the pipe

            // Execute the first command
            execvp(cmd1[0], cmd1); // execvp executes the command specified in the first argument with the arguments specified in the second argument
            // If the execvp function returns, it means that the command was not found
            perror("Error executing command"); // perror prints the error message corresponding to the error number stored in errno
            exit(1); // exit the program
        } else if (pid1 < 0) { // if fork returns -1, it means that an error occurred
            perror("Error creating first child process"); // perror prints the error message corresponding to the error number stored in errno
            exit(1); // exit the program
        }

        // Create the second child process to execute the second command
        pid_t pid2 = fork(); // fork returns the process ID of the child process to the parent process and 0 to the child process
        if (pid2 == 0) { // if we are in the child process
            printf("I'm the second child, executing the second command.\n"); // print a message to indicate that we are in the second child process

            // Redirect the input of the second command to the read end of the pipe
            dup2(fd[0], STDIN_FILENO); // dup2 duplicates the file descriptor specified in the first argument to the file descriptor specified in the second argument
            close(fd[0]); // close the read end of the pipe
            close(fd[1]); // close the write end of the pipe

            // Execute the second command
            execvp(cmd2[0], cmd2); // execvp executes the command specified in the first argument with the arguments specified in the second argument
            // If the execvp function returns, it means that the command was not found
            perror("Error executing command"); // perror prints the error message corresponding to the error number stored in errno
            exit(1); // exit the program
        } else if (pid2 < 0) { // if fork returns -1, it means that an error occurred
            perror("Error creating second child process");  // perror prints the error message corresponding to the error number stored in errno
            exit(1); // exit the program
        }

        // Close the file descriptors used by the parent process
        close(fd[0]);   // close the read end of the pipe
        close(fd[1]);   // close the write end of the pipe

        // Wait for both child processes to complete their execution
        waitpid(pid1, NULL, 0); // waitpid suspends the execution of the parent process until the child process specified in the first argument terminates
        waitpid(pid2, NULL, 0); // waitpid suspends the execution of the parent process until the child process specified in the first argument terminates

        printf("Parent is done waiting.\n"); // print a message to indicate that the parent process is done waiting
    }

    // Exit the shell
    return 0;
}

