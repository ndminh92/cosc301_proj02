/*
 * Dang Minh Nguyen
 * COSC301 Fall 2014 Project 02
 */


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

#include "list.h"
#include "process.h"

const char prompt[] = "> ";

char *check_path(struct node **, char *);

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
    while (commands_list[count] != NULL) {
        count++;
    }
    return count;
}

void ignore_comment(char *string) { // Replace first # with \0
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
    char *token = strtok(scopy,delimiter); // get first token
    char **tok_list = malloc((length + 1) * sizeof(char *));  
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
        printf("Token number %d: %s\n", i+1, tokens[i]);
        i++;
    }
}

// Concatenate a list of arguments into a single string
char *full_command(char **command) {
    int finallength = 1;
    int argn = strlistlen(command);
    for (int i=0; i<argn; i++) {
        finallength += strlen(command[i]) + 1;
    }
    char *result = malloc(sizeof(char) * finallength);
    result[0]='\0';
    for (int i=0; i<argn; i++) {
        strcat(result, command[i]);
        strcat(result, " ");
    }
    //printf("Full command is: %s\n",result);
    result[strlen(result)-1]='\0'; // get rid of the last space
    return result;
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
 * Try to execute a command as a shell built-in. 
 * Return 1 if command is built in and executed
 * Return 0 if command is built in but has syntax error  
 * Return -1 if command is not built in the shell
 */
int execute_built_in(char **argv, int *mode, int *exit_flag) {
    if (argv[0] == NULL) {          // No arguments, no execution
        return 0; 
    } else if (strcmp(argv[0],"exit") == 0) { // exit command
        if (argv[1] == NULL) { // The command is just 'exit'
            *exit_flag = 1;
            return 1;
        } else {  // some giberrish after 'exit'. print error
            printf("Command not recognized. Try 'exit' instead\n");
            return 0;    
        }                
    } else if (strcmp(argv[0], "mode") == 0) { // mode command
        if (argv[1] == NULL) { // print current mode
            if (*mode == 0) {
                printf("Shell is currently in sequential mode\n");
                return 1;
            } else if (*mode == 1) {
                printf("Shell is currently in parallel mode\n");
                return 1;
            } else {
                printf("Unknown mode. Something have broken\n");
                return 0;
            }
        } else if (strcmp(argv[1], "s") == 0 || 
                    strcmp(argv[1], "sequential") == 0) { // mode change
            *mode = 0;
            return 1;
        } else if (strcmp(argv[1], "p") == 0 || 
                    strcmp(argv[1], "parallel") == 0) { // mode change
            *mode = 1;
            return 1;
        } else {
            printf("Command not recognized. Try 'mode s' or 'mode p' instead\n");
            return 0;
        }
    }    
    return -1;
}

// Execute a non built-in command
int execute_nonshell(char **argv, struct node **path_list) {
    int status = 0;
    char *temp = check_path(path_list, argv[0]);
    
    pid_t pid = fork();

    if (pid == 0) {  // Child process
        if (execv(temp, argv) < 0) {
            fprintf(stderr, "execv failed: %s\n", strerror(errno));
            exit(-1);     // Quit if execv failed
        }
    }
    wait(&status);
    free(temp); 
    return 0;

}

/*
 * Execute a list of commands
 * Free the list before returning
 */
int execute_all(char ***commands_list, int *mode, struct node **path_list) {
    int exit_flag = 0;
    int counter = 0;
    int number_of_commands = how_many_commands(commands_list);
    printf("Number of commands is %d\n",number_of_commands);  
    if (*mode == 0) { // sequential execution    
        char **command = commands_list[counter];
        while (command != NULL) {
            if (execute_built_in(command, mode, &exit_flag) < 0) {
                execute_nonshell(command, path_list);            
            }            
            //execute_one(command, mode, &exit_flag, path_list);
            
  
            counter++;
            command = commands_list[counter];
        }
        free_commands_list(commands_list);
        
    } else if (*mode == 1) { // parallel execution
        char **command = commands_list[counter];
        pid_t pid_list[number_of_commands];
        pid_t pid = 0;
        int status = 0;
        int child_count = 0;
        while (command != NULL) { // Keep reading commands and forking
            // only fork if command is not built-in
            if (execute_built_in(command, mode, &exit_flag) < 0) { 
                child_count++;
                                     
                pid = fork();
                if (pid == 0) { // Child process, execute command
                    execute_nonshell(command, path_list);
                    char *command_literal = full_command(command); 
                    printf("Process %d (%s) is completed\n",getpid(),command_literal);
                    exit(0);
                } else { 
                    // Parent process, do nothing
                }    
                
            }
            pid_list[counter] = pid;
            counter++;                
            command = commands_list[counter];
        }  
        while (child_count > 0) { // wait for each child to complete
            wait(&status); 
            child_count--;
        }
        free_commands_list(commands_list);
    } 

    if (exit_flag == 1) { // exit command received somewhere in the list
        free_list(path_list);        
        exit(0);
    }
    return 0;
}

int execute_parallel(char ***commands_list, int *mode, struct node **path_list, int *exit_flag) {
    int counter = 0;
    //int number_of_commands = how_many_commands(commands_list);
    //printf("Number of commands is %d\n",number_of_commands);  
    char **command = commands_list[counter];
    pid_t pid = 0;
    int status = 0;
    int child_count = 0;
    while (command != NULL) { // Keep reading commands and forking
        // only fork if command is not built-in
        if (execute_built_in(command, mode, exit_flag) < 0) { 
            child_count++;
            pid = fork();
            if (pid == 0) { // Child process, execute command
                execute_nonshell(command, path_list);
                char *command_literal = full_command(command); 
                printf("Process %d (%s) is completed\n",getpid(),command_literal);
                exit(0);
            } else { 
                // Parent process, do nothing
            }       
        }
        counter++;                
        command = commands_list[counter];
    }  
    while (child_count > 0) { // wait for each child to complete
        wait(&status); 
        child_count--;
    }
    free_commands_list(commands_list);
    return 0;
} 

int execute_sequential(char ***commands_list, int *mode, struct node **path_list, int *exit_flag) {
    int counter = 0;
    //int number_of_commands = how_many_commands(commands_list);
    //printf("Number of commands is %d\n",number_of_commands);  
    char **command = commands_list[counter];
    while (command != NULL) {
        if (execute_built_in(command, mode, exit_flag) < 0) {
            execute_nonshell(command, path_list);            
        }            
        counter++;
        command = commands_list[counter];
    }
    free_commands_list(commands_list);
    return 0;
}        

char *** parse_commands(char * input) {
    int length = strlen(input);
    if (input[length-1] == '\n') { // Remove '\n' char from input
        input[length-1] = '\0';
    }    
    ignore_comment(input);
    
    char **unparsed_commands = tokenify(input,";"); // Break input into commands by ';' 
                                                    // Each command is still a string   
   
    // Now we break down each command into a list of arguments
    int number_of_commands = strlistlen(unparsed_commands);
    char ***commands_list = malloc(sizeof(char **) * (number_of_commands + 1));
    for (int i=0; i < number_of_commands; i++) {
        commands_list[i] = tokenify(unparsed_commands[i], " \n\t");
    }    
    commands_list[number_of_commands] = NULL;
    free_tokens(unparsed_commands);                   
    return commands_list;
}

// PART 2 FUNCTIONS

struct node **get_path() {
    struct node **path_list = malloc(sizeof(struct node *));
    *path_list = NULL; // initialization 
    FILE *datafile = NULL;
    datafile = fopen("shell-config", "r");  
    if (datafile == NULL) {
        printf("Unable to open shell-config. No PATH option available\n");
        return path_list; // Return a NULL path_list
    } else {
        char line_buffer[4096];
        while(fgets(line_buffer, 4096, datafile) != NULL) { // read until EOF
            int length = strlen(line_buffer);
            // Remove '\n' char from line_buffer
            if (line_buffer[length-1] == '\n') { 
                line_buffer[length-1] = '\0';
            }                
            ignore_comment(line_buffer);
            list_insert(line_buffer, path_list);
        }         
        printf("Done reading shell-config. Added the following:\n");
        print_list(path_list);
    }
    fclose(datafile);
    return path_list;
}

// Test if the input file name is found in one of the paths 
// If a match is found, immediately return the full path to the file
// If no match is found, return a copy the filename  in the heap
char *check_path(struct node **path_list, char *file) {
    int lengthf = strlen(file);    
    int counter = 0;    
    struct node *path = *path_list;
    struct stat buffer;
    int status = 0;
    while (path != NULL) {
        char *temp = malloc(sizeof(char) * (strlen(path->value) + lengthf + 2));
        temp[0] = '\0'; 
        strcat(temp, path->value);
        strcat(temp,"/");
        strcat(temp, file);
        status = stat(temp, &buffer);
        if (status == 0) { // success
            return temp;
        } else { 
            // Do nothing.        
        }
        free(temp);
        counter++;
        path = path -> next;
    }
    char *result = malloc(sizeof(char) * (lengthf + 1));
    strcpy(result, file);
    return result;
}
// END OF PART 2 FUNCTIONS

int main(int argc, char **argv) {
    int mode = 0; // 0 for sequential, 1 for parallel
    struct node **path_list = get_path();
    int exit_flag = 0;
    printf("%s", prompt);
    fflush(stdout);  
    char buffer[1024];

    while (fgets(buffer, 1024, stdin) != NULL) {
        char ***commands_list = parse_commands(buffer);        
        //execute_all(commands_list, &mode, path_list);       
        int number_of_commands = how_many_commands(commands_list);
        printf("Number of commands is %d\n",number_of_commands);  
        if (mode == 0) { // sequential mode
            execute_sequential(commands_list, &mode, path_list, &exit_flag);
        } else {  // parallel mode 
            execute_parallel(commands_list, &mode, path_list, &exit_flag);    
        }
        
        // Check for exit
        if (exit_flag == 1) {  
            break;        
        }
        // Print prompt for next command
        printf("%s", prompt);
        fflush(stdout);  
    }
    free_list(path_list);
    printf("Shell will now exit.\n");
   

    return 0;
}

