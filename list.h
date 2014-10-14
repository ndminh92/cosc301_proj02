#ifndef __LIST_H__
#define __LIST_H__

/* your list data structure declarations */
struct node {
    char *value;
    struct node *next;
};

/* your function declarations associated with the list */
void list_insert(char *path, struct node **head);
void free_list(struct node **head);
void print_list(struct node **head) ;

#endif // __LIST_H__
