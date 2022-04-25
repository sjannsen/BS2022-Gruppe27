//
// Created by User on 25.04.2022.
//

#ifndef BSPRAKTIKUM_KEYVALSTORE_H
#define BSPRAKTIKUM_KEYVALSTORE_H

#define BUFFERSIZE 1024

typedef struct Node
{
    char key[BUFFERSIZE];
    char value[BUFFERSIZE];
    struct Node* next;
} Node;

void insert_end(Node** root, char* assigned_key, char* assigned_value);
int remove_element(Node** root, char* delet_key);
void deallocate(Node** root);

#endif //BSPRAKTIKUM_KEYVALSTORE_H
