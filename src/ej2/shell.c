#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

#define MAX_COMMANDS 200
#define MAX_ARGS 20


void execute_command(char *command) {

    char *cmd_args[MAX_ARGS];
    int arg_count = 0;
    char *token;

    token = strtok(command, " ");
    while (token != NULL && arg_count < MAX_ARGS - 1) {
        // Remove quotes from token
        char *quote_pos;
        for (int i = 0; i < 2; i++) { 
            while ((quote_pos = strchr(token, '\"')) != NULL) {
                memmove(quote_pos, quote_pos + 1, strlen(quote_pos)); 
            }
        }
        cmd_args[arg_count++] = token;
        token = strtok(NULL, " ");
    }
    cmd_args[arg_count] = NULL;
    execvp(cmd_args[0], cmd_args);   // execute command
    // if execvp returns, there was an error
    perror("Execvp Error");
    exit(1);
}


int main() {

    char command[256];
    char *commands[MAX_COMMANDS];
    int command_count = 0;
    int pipes[MAX_COMMANDS][2];

    while (1) {

        printf("Shell> ");
        fgets(command, sizeof(command), stdin);
        command[strcspn(command, "\n")] = '\0';

        char *token = strtok(command, "|");
        while (token != NULL) {
            commands[command_count++] = token;
            token = strtok(NULL, "|");
        }

        // Create pipes
        for (int i = 0; i < command_count - 1; i++) {
            if (pipe(pipes[i]) == -1) { 
                perror("Pipe error");
                exit(1);
            }
        }

        // Create children
        for (int i = 0; i < command_count; i++) {
            int pid = fork(); 

            if (pid == -1){
                perror("Fork error");
                exit(1);
            
            } else if (pid == 0){
                if (i > 0) { //not the first command
                    dup2(pipes[i - 1][0], STDIN_FILENO); 
                }

                if (i < command_count - 1) { //not the last command
                    dup2(pipes[i][1], STDOUT_FILENO); 
                }

                for (int j = 0; j < command_count - 1; j++) { //close all pipes
                    close(pipes[j][0]);
                    close(pipes[j][1]);
                }
                execute_command(commands[i]);  //gets args and executes command
            }
    
        }

        // Close all pipes
        for (int i = 0; i < command_count - 1; i++) { 
            close(pipes[i][0]); 
            close(pipes[i][1]); 
        }

        // Wait for all children
        for (int i = 0; i < command_count; i++) { 
            wait(NULL);
        }

        command_count = 0;
    }
    return 0;
}
