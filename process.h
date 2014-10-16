#ifndef __PROCESS_H__
#define __PROCESS_H__

/* data structure declarations */
struct process{
    int pid;
    char *command; // command that generated this process
    int paused; // 0 if unpaused, 1 if paused
    struct process *next;
};

/* your function declarations associated with the list */
void add_process(int pid, char *command, int paused, struct process **process_list);
void free_process(struct process **head);
void print_process(struct process **head);
int in_list(int search_pid, struct process **head);
struct process *get_process_info(int search_pid, struct process **head);
int remove_process(int pid, struct process **head);
#endif // __PROCESS_H__
