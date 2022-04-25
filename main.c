#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>

#include "keyValStore.h"

#define PORT 5678
#define BUFFERSIZE 1024


int special_character(char* str);
int special_character_without_space(char* str);


//Test of the function strtokn() isntead of extracting

int main(int argc, char const *argv[])
{
    //Linked List
    Node* root = NULL;

    //Socketvariables
    char buffer[BUFFERSIZE] = "Hello from Server\n"; //Input/Output Buffer
    int server_socket;
    int client_socket;
    int n; //Status-check

    //Socketinitialisation
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1)
    {
        puts("Socket could not be created");
        exit(1);
    }
    puts("Socket created");

    //Configuration
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET; //Address Family Internet
    server_address.sin_port = htons(PORT); //Host to network short
    server_address.sin_addr.s_addr = INADDR_ANY;

    //Preparing Connection

    //Binding the address to the socket
    n = bind(server_socket, (struct sockaddr*)&server_address, sizeof(server_address));
    if (n == -1)
    {
        puts("Could not bind address to the socket");
        exit(2);
    }
    puts("Address binded to the socket");

    //Marks the socket as a passive socket, that will be used to accept incoming connection request
    n = listen(server_socket, 5);
    if (n == -1)
    {
        puts("Could not mark socket a passiv socket listening for incoming connection request");
        exit(3);
    }
    puts("Socket marked a passive socket listening for incoming connection request");


    Q: client_socket = accept(server_socket, NULL, NULL);
    if (client_socket == -1) {
        puts("Accepting incoming connection failed");
        goto Q;
    }
    puts("Connection accepted");

    //Communication
    send(client_socket, buffer, BUFFERSIZE, 0);


    int selected_command = 5;
    while (1)
    {

        n = recv(client_socket, &buffer, BUFFERSIZE-1, 0);
        if (n > 0)
        {
            buffer[n-2] = '\0';
            int i = 0;
            printf("Bufer[]: |%s|\n", buffer);
        }
        else
        {
            puts("Error ocurred while receiving data");
        }

        selected_command = 5;
        char output[BUFFERSIZE] = "";



        //Use of strtok instead of for loop
        char* command = strtok(buffer, " ");
        printf("Command[]: |%s|\n", command);

        //Command selection
        if(strcmp(command, "PUT") == 0) { //strmp directly (strcmp(command, get) == 0)
            selected_command = 0;
            puts("Command == PUT");
        }
        if (strcmp(command, "GET") == 0) {
            selected_command = 1;
            puts("Command == GET");
        }
        if (strcmp(command, "DEL") == 0) {
            selected_command = 2;
            puts("Command == DEL");
        }

        if (strcmp(command, "QUIT") == 0) {
            selected_command = 3;
            puts("Command == QUIT");
        }

        if (special_character_without_space(buffer) == -1) {//Invalid command e.g DEL/DELL/etc.
            selected_command = 4;
            puts("Buffer contains special characters");
        }

        if (strlen(command) == 0)
        {
            puts("Buffer is empty");
            selected_command = 5;
        }

        printf("Selected_command: %d\n", selected_command);


        //USE OF STRTOK INSTEAD OF EXTRACTING

        //Declaration for selected_command < 3
        int end_of_key = 4;

        char* key = strtok(NULL, " ");

        if (selected_command < 3)
        {

            printf("key: |%s|\n", key);

            if (special_character(key) == -1 )
            {
                printf("Key is invalid\n");
                strcpy(output, key);
                strcat(output, ":key_invalid\n");
                send(client_socket, output, sizeof(output), 0);

            }

        }



        switch (selected_command)
        {
            //PUT
            case 0: {

                char* value = strtok(NULL, "\n");
                printf("Value: |%s|\n", value);

                //MISSING: TESTING FOR SPECIAL CHARACTERS (VALUE)
                int p = special_character_without_space(value);
                if (n == -1)
                {
                    stpcpy(output, "Invalid value");
                    break;
                }
                //Needs to be tested


                insert_end(&root, key, value);
                puts("Added to list:");
                printf("%s, %s\n", root->key, root->value);

                puts("Checking the list");
                for (Node* curr = root; curr != NULL; curr = curr->next)
                {
                    printf("%s, %s\n", curr->key, curr->value);
                }
                puts("End of list");

                //Output_Message
                strcpy(output,"PUT:"); //strncat replace, adding max n bytes
                strcat(output, key);
                strcat(output, ":");
                strcat(output, value);
                strcat(output, "\n");
            }
                break;

                //GET
            case 1: {
                strcpy(output, "GET:");
                strcat(output, key);
                strcat(output, ":");
                int key_found = 0;
                puts("Printing the list before searching");
                for (Node* curr = root; curr != NULL; curr = curr->next)
                {
                    printf("%s, %s\n", curr->key, curr->value);
                }
                puts("End of list");

                for (Node* curr = root; curr != NULL ; curr = curr->next)
                {
                    puts("Searching for key");
                    printf("Curr->key |%s| searched key |%s|\n", curr->key, key);
                    if (strcmp((curr->key), key) == 0)
                    {
                        strcat(output, curr->value);
                        strcat(output, "\n");
                        key_found = 1;
                        break;
                    }
                }
                if (root == NULL)
                {
                    puts("list is empty");
                }


                if (key_found == 0)
                {
                    puts("Key not found");
                    strcat(output, "key_nonexistent\n");
                }
            }
                break;

                //DEL
            case 2: {
                strcpy(output, "DEL:");
                strcat(output, key);
                strcat(output, ":");
                int element_deleted = remove_element(&root, key);

                if (element_deleted == -1)
                    strcat(output, "key_nonexistent\n");
                else
                    strcat(output, "key_deleted\n");
            }
                break;

                //QUIT //Not necessary anymore due to the while loop
            case 3:

                break;

                //Buffer is empty
            case 5: {
                strncpy(output, "An error ocurred while receiving the data\n", BUFFERSIZE-1);
            }
                break;

                //Syntax error
            default:{
                strcpy(output, "command_nonexistent\n");
            }
                break;
        }

        //Not necessary anymore
        puts("Checking the list after Switch Case");
        for (Node* curr = root; curr != NULL; curr = curr->next)
        {
            printf("%s, %s\n", curr->key, curr->value);
        }

        send(client_socket, output, sizeof(output), 0);

    }//End of the while loop



    puts("Checking the list at the end");
    for (Node* curr = root; curr != NULL; curr = curr->next)
    {
        printf("%s, %s\n", curr->key, curr->value);
    }



    close(client_socket);
    close(server_socket);
    deallocate(&root);

    return 0;
}


//PROBLEM DES BUFFER OVERFLOW -> '\0' einfügen
//MISSING: FUNCTION TO CHECK IF KEY IS ALREADY USED

//Mayby special_character(char* str, int space 0/1) instead of using two seperate functions
int special_character(char* str){
    int i, flag;
    flag = 0;
    i = 0;
    printf("Special_character() for: %s\n", str);
    while (str[i] != '\0') {
        //checking each character of the string for special character.
        if(str[i] == '!' || str[i] == '@' || str[i] == '#' || str[i] == '$'
           || str[i] == '%' || str[i] == '^' || str[i] == '&' || str[i] == '*'
           || str[i] == '(' || str[i] == ')' || str[i] == '-' || str[i] == '{'
           || str[i] == '}' || str[i] == '[' || str[i] == ']' || str[i] == ':'
           || str[i] == ';' || str[i] == '"' || str[i] == '\'' || str[i] == '<'
           || str[i] == '>' || str[i] == '.' || str[i] == '/' || str[i] == '?'
           || str[i] == '~' || str[i] == '`' || str[i] == ' '){
            printf("String is not allowed\n");
            flag = -1;
            break;
        }
        i++;
    }
    //if there is no special charcter
    if (flag == 0){
        printf("String is accepted\n");
    }
    return flag;
}

int special_character_without_space(char* str){
    int i, flag;
    flag = 0;
    i = 0;
    while (str[i] != '\0'){
        //checking each character of the string for special character.
        if(str[i] == '!' || str[i] == '@' || str[i] == '#' || str[i] == '$'
           || str[i] == '%' || str[i] == '^' || str[i] == '&' || str[i] == '*'
           || str[i] == '(' || str[i] == ')' || str[i] == '-' || str[i] == '{'
           || str[i] == '}' || str[i] == '[' || str[i] == ']' || str[i] == ':'
           || str[i] == ';' || str[i] == '"' || str[i] == '\'' || str[i] == '<'
           || str[i] == '>' || str[i] == '.' || str[i] == '/' || str[i] == '?'
           || str[i] == '~' || str[i] == '`'){
            printf("String is not allowed\n");
            flag = -1;
            break;
        }
        i++;
    }
    //if there is no special charcter
    if (flag == 0){
        printf("String is accepted\n");
    }
    return flag;
}


