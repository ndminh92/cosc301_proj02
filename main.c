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


/*
 * Execute command based on arguments list given
 * If successful return 0
 * If error occurs return -1    
 */
int execute(char **argv) {
    if (argv[0] == NULL) { // No arguments, no execution
        return 0; 
    }
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


int main(int argc, char **argv) {
    char *command = NULL;

    printf("%s", prompt);
    fflush(stdout);  
    char buffer[1024];


    while (fgets(buffer, 1024, stdin) != NULL) {
    // process current command line in buffer
        replace_hash(buffer);
        char **commands = tokenify(buffer,";");
//        print_tokens(command);
//        free_tokens(command);
        int argn = 0;
        command = commands[argn];
        while (command != NULL) {
            printf("Printing command number %d \n",argn);
            char **arguments = tokenify(command, " \n\t");
            print_tokens(arguments);

            printf("Now executing commands.\n");
            execute(arguments);

            printf("Execution complete.\n");        
            argn++;
            command = commands[argn];
            free_tokens(arguments);
        }

        free_tokens(commands);

        // Print prompt for next command
        printf("%s", prompt);
        fflush(stdout);  
    }
    printf("Shell will now exit.\n");
   

    return 0;
}

