#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <poll.h>
#include <signal.h>

const char prompt[] = "> ";

int strlistlen(char **strlist) {
    int count = 0;
    char *temp = strlist[count];
    while (temp != NULL) {
        count++;
        temp = strlist[count];
    }
    return count;
}

int how_many_commands(char ***commands_list) {
    int count = 0;
    char **temp = NULL;
    while (temp != NULL) {
        count++;
        temp = commands_list[count];
    }
    return count;
}

void replace_hash(char *string) { // Replace first # with \0
    int length = strlen(string);
    for (int i = 0; i < length; i++) { 
        if (string[i] == '#') {
            string[i] = '\0';
            return;
        }
    }        
}

char** tokenify(const char *input, const char *delimiter) {
    int count = 0;
    char *scopy = strdup(input);

    int length = strlen(scopy);
    if (scopy[length-1] == '\n') { // Remove EOL char from input
        scopy[length-1] = '\0';
    }
    
    char *token = strtok(scopy,delimiter); // get first token
    char **tok_list = malloc(length * sizeof(char *));  
    while (token != NULL) {        
        tok_list[count] = strdup(token); 
        count += 1;
        token = strtok(NULL,delimiter); // get next token
    }
    tok_list[count] = NULL;      
    
    //optimize length of pointer array
    char **short_tok_list = malloc((count + 1) * sizeof(char *));
    for (int i = 0; i <= count; i++) { // Copy up to the NULL value
        short_tok_list[i] = tok_list[i];
    }    

    free(scopy);
    free(tok_list);
    return short_tok_list;
}

void print_tokens(char **tokens) {
    int i = 0;
    while (tokens[i] != NULL) {
        printf("Token %d: %s\n", i+1, tokens[i]);
        i++;
    }
}

void free_tokens(char **tokens) {
    int i = 0;
    while (tokens[i] != NULL) {
        free(tokens[i]); // free each string
        i++;
    }
    free(tokens); // then free the array
}

void free_commands_list(char ***commands_list) {
    int i = 0;
    while (commands_list[i] != NULL) {
        free_tokens(commands_list[i]);
        i++;
    }
    free(commands_list);
}

/*
 * Execute one command based on arguments list given
 * If successful return 0
 * If error occurs return -1    
 */
int execute_one(char **argv, int *mode, int *exit_flag) {
    if (argv[0] == NULL) { // No arguments, no execution
        return 0; 
    } else if (strcmp(argv[0],"exit") == 0) { // exit command
        *exit_flag = 1;
        return 0;
    } else if (strcmp(argv[0], "mode") == 0) { // mode command
        if (argv[1] == NULL) { // print current mode
            if (*mode == 0) {
                printf("Shell is currently in sequential mode.\n");
            } else if (*mode == 1) {
                printf("Shell is currently in parallel mode.\n");
            } else {
                printf("Unknown mode. Something have broken.\n");
                return -1;
            }
        } else if (strcmp(argv[1], "s") == 0 || strcmp(argv[1], "sequential") == 0) {
            *mode = 0;
        } else if (strcmp(argv[1], "p") == 0 || strcmp(argv[1], "parallel") == 0) {
            *mode = 1;
        } else {
            printf("Command not recognized. Try 'mode s' or 'mode p'.\n");
            return 0;
        }
    } else { // execute file
        int status = 0;
        pid_t pid = fork();
        if (pid == 0) {  // Child process
            if (execv(argv[0], argv) < 0) {
                fprintf(stderr, "execv failed: %s\n", strerror(errno));
                exit(0);     // Quit if execv failed
            }
        }
        wait(&status);
        return 0;
    }
    return 0;
}

int execute_all(char ***commands_list, int *mode) {
    int exit_flag = 0;
    int counter = 0;
    int number_of_commands = how_many_commands(commands_list);
    if (*mode == 0) { // sequential execution    
        char **command = commands_list[counter];
        while (command != NULL) {
            printf("Printing command number %d \n",counter);
            print_tokens(command);
            printf("Now executing commands.\n");
            execute_one(command, mode, &exit_flag);
            printf("Execution complete.\n");     
            counter++;
            command = commands_list[counter];
        }
        free_commands_list(commands_list);
        
        
    } else if (*mode == 1) { // parallel execution
        char **command = commands_list[counter];
        pid_t pid_list[number_of_commands];
        pid_t pid = 0;
        int status = 0;
        while (command != NULL) { // Keep reading commands and forking
            pid = fork(); 
            if (pid == 0) { // Child process, execute command
                execute_one(command, mode, &exit_flag);
                exit(exit_flag); // exit
            } else { // Parent process. Fork for next job
                pid_list[counter] = pid;
                counter++;                
                command = commands_list[counter];
            }    
        }  
        for (int i=0; i < number_of_commands; i++) { // wait for each child to complete
            waitpid(pid_list[i], &status,0); 
            printf("The exit value of the child is: %d\n",status);
            if (WEXITSTATUS(status) == 1) {
                exit_flag = 1;
            }
        }
        free_commands_list(commands_list);
        return 0;
    } 
    if (exit_flag == 1) { // exit command read at somepoint
        exit(0);
    }
    return 0;
}

char *** parse_commands(char * input) {
    replace_hash(input);
    char **unparsed_commands = tokenify(input,";"); // Break input into commands by ';'    
   
    // Now we break down each command into a list of arguments
    int number_of_commands = strlistlen(unparsed_commands);
    char ***commands_list = malloc(sizeof(char ***) * (number_of_commands + 1));
    for (int i=0; i < number_of_commands; i++) {
        commands_list[i] = tokenify(unparsed_commands[i], " \n\t");
    }    
    commands_list[number_of_commands] = NULL;
    free_tokens(unparsed_commands);                   
    return commands_list;
}

int main(int argc, char **argv) {
    int mode = 0; // 0 for sequential, 1 for parallel

    printf("%s", prompt);
    fflush(stdout);  
    char buffer[1024];


    while (fgets(buffer, 1024, stdin) != NULL) {
        char ***commands_list = parse_commands(buffer);        
        execute_all(commands_list, &mode);       

        // Print prompt for next command
        printf("%s", prompt);
        fflush(stdout);  
    }
    printf("Shell will now exit.\n");
   

    return 0;
}

