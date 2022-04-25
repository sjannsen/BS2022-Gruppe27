//
// Created by User on 25.04.2022.
//
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "keyValStore.h"
#define BUFFERSIZE 1024


struct Node;



void insert_end(Node** root, char* assigned_key, char* assigned_value) {
    Node* new_Node = malloc(sizeof(Node)); //Initialisation of new Node
    if (new_Node == NULL) //Checking for NULL-pointer
    {
        exit(1);
    }
    strcpy(new_Node->key, assigned_key);
    strcpy(new_Node->value, assigned_value);
    new_Node->next = NULL;

    if (*root == NULL) //Check for empty list
    {
        *root = new_Node; //Node = root
        return;
    }

    Node* curr = *root; //Else finding the last element
    while (curr->next != NULL)
    {
        curr = curr->next;
    }
    curr->next = new_Node; //Setting new_Node as the last element
}

int remove_element(Node** root, char* delet_key) { //returns -1 if not element is deleted
    if (*root == NULL)
    {
        puts("Empty list");
        return -1;
    }

    if (strcmp((*root)->key, delet_key) == 0) //Special Case remove head-element
    {
        puts("Key is first element");
        Node* to_remove = *root;

        if ((*root)->next == NULL)
            *root = NULL;
        else
            *root = (*root)->next;

        free(to_remove);
        return 0;
    }

    for (Node* curr = *root; curr->next != NULL; curr = curr->next)
    {
        puts("Seaching for key");
        printf("Curr->key |%s| delet_key |%s|\n", curr->key, delet_key);
        if (strcmp(curr->next->key, delet_key) == 0)
        {
            Node* to_remove = curr->next;
            curr->next = curr->next->next;
            free(to_remove);
            return 0;
        }
    }
    return -1;
}

void deallocate(Node** root) {
    Node* curr = *root;
    while (curr != NULL)
    {
        Node* aux = curr;
        curr = curr->next;
        free(aux);
    }
    *root = NULL;
}


