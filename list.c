#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "list.h"

/* your list function definitions */

// Insert new path into list at the head
void list_insert(char *path, struct node **head) { 
    struct node *new_node = malloc(sizeof(struct node));
    char *copy = malloc(sizeof(char) * (strlen(path) + 1));
    strcpy(copy,path);
    new_node -> value = copy;
    new_node -> next = *head;
    //printf("New node valueis: %s\n",new_node->value);
    *head = new_node;
    //printf("New head value is: %s\n",(*head)->value);
}

// Free memory used by the list
void free_list(struct node **head) {
    struct node *next = NULL;
    struct node *temp = *head;
    while (temp != NULL) {
        next = temp -> next;
        free(temp->value);
        free(temp);
        temp = next;
    }
    free(head);
}


void print_list(struct node **head) {
    printf("** List Contents Begin **\n");
    struct node *temp = *head;
    while (temp != NULL) {
        printf("%s\n", temp -> value);
        temp = temp -> next;
    }
    printf("** List Contents End **\n");
}
