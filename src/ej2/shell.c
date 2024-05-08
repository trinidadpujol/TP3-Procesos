#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

#define MAX_COMMANDS 200

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

                char *cmd_args[20];
                int arg_count = 0;
            
                get_args(commands[i], cmd_args, &arg_count);  
                execvp(cmd_args[0], cmd_args); // Execute command
                
                perror("Execvp error");
                exit(1);
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
