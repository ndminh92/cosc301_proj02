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
    struct process *temp = *process_list;
    while (temp != NULL) {
        free(temp -> command);
        struct process *next = temp -> next;
        free(temp);
        temp = next;
    }
    free(process_list);
}

void print_process(struct process **process_list) {
    
}

int in_list(int search_pid, struct process **head) {
    struct process *temp = *head;
    while (temp != NULL) {
        if (search_pid == temp -> pid) {
            return 1;
        }        
        temp = temp -> next;
    }
    return 0;    
}

struct process *get_process_info(int search_pid, struct process **head) {
    struct process *temp = *head;
    while (temp != NULL) {
        if (search_pid == temp -> pid) {
            return temp;
        }        
        temp = temp -> next;
    }
    return NULL;    
}

// Remove a process from the list based on pid
// Return 1 on success, 0 on fail
int remove_process(int pid, struct process **head) {
    struct process *temp = *head;
    if (temp == NULL) {
        return 0;    
    } else if (pid == (temp -> pid)) { // Found match first in the list, just change head pointer
        *head = temp -> next;
        free(temp -> command);
        free(temp);
        return 1;
    } else {
        struct process *prev = temp;
        temp = prev -> next;    
        while (temp != NULL) {
            if (pid == temp -> pid) {
                prev -> next = temp -> next; 
                free(temp -> command);
                free(temp);
                return 1;
            }
            prev = temp;
            temp = temp -> next;
        }
    }
    return 0;
}        
