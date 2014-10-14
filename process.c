#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "process.h"

void add_process(int pid, char *command, int paused, struct process **process_list){
    struct process *new_process = malloc(sizeof(struct process));
    new_process -> pid = pid;
    new_process -> command = strdup(command);
    new_process -> paused = paused;
    new_process -> next = *process_list;
    *process_list = new_process;
}

void free_process(struct process **process_list) {

}

void print_process(struct process **process_list) {

}
