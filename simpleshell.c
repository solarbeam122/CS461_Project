#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

/*
Justus Contreras
This program is a basic implementation of a 
shell in the C programming language.*/

#define MAX_LINE 80 /* The maximum length command */

int main(){

    while(1){
        // Read the user input
        char command[MAX_LINE];
        // Get the current working directory
        char cwd[1024];
        getcwd(cwd, sizeof(cwd));
        // Print the prompt with the current directory
        printf("%s > ", cwd);
        // Read the command from stdin
        fgets(command, MAX_LINE, stdin);
        // Remove the trailing newline character from the command
        command[strlen(command)-1] = '\0';
        // Split the command into arguments
        char *args[MAX_LINE/2 + 1];
        int i = 0;
        char *token = strtok(command, " ");
        while(token != NULL){
            args[i] = token;
            token = strtok(NULL, " ");
            i++;
        }
        args[i] = NULL;
        // Check if the command is "cd"
        if(strcmp(args[0], "cd") == 0){
            // Change the current working directory
            if(args[1] == NULL){
                // If no argument is given, change to the home directory
                chdir(getenv("HOME"));
            }
            else{
                // Change to the given directory
                chdir(args[1]);
            }
        }
        // Otherwise, execute the command using a child process
        else{
            pid_t pid = fork();
            if(pid == 0){
                // Child process: execute the command
                execvp(args[0], args);
                // If execvp returns, there was an error
                printf("Error executing command\n");
                exit(0);
            }
            else{
                // Parent process: wait for the child to finish
                wait(NULL);
            }
        }
    }
    return 0;
}
