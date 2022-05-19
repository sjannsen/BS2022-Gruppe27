
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros
#include "keyValStore.h"


#define PORT 5679
#define BUFFERSIZE 1024


int special_character(char *str);


int main(int argc , char *argv[])
{
    int opt = 1;
    int master_socket, addrlen, new_socket, client_socket[30], max_clients = 30, activity, i , byte_read, server_file_descriptor, n, running;
    int max_server_file_descriptor;
    struct sockaddr_in address;
    char buffer[BUFFERSIZE];
    char output[BUFFERSIZE];
    Node *root = NULL;


    //set of socket descriptors
    fd_set read_file_descriptors;

    //welcome message
    char* message = "Hello from Server \r\n";

    for (i = 0; i < max_clients; i++)  //initialise all client_socket[] to 0 so not checked
        client_socket[i] = 0;

    if ((master_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0) { 	//create a master socket
        printf("[-]Error while creating socket\n");
        return -1;
    }

    if (setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0 ) { //set master socket to allow multiple connections, this is just a good habit, it will work without this
        printf("[-]Error while setting socket options\n");
        return -2;
    }

    address.sin_family = AF_INET; //type of socket created
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(master_socket, (struct sockaddr *)&address, sizeof(address))<0) { //bind the socket to localhost port 8888
        printf("[-]Error while binding socket to port %d\n", PORT);
        return -3;
    }
    printf("[+]Socket binded to port %d\n", PORT);

    if (listen(master_socket, 3) < 0) { //try to specify maximum of 3 pending connections for the master socket
        printf("[-]Error while listening\n");
        return -4;
    }
    printf("[+]Listening...\n");

    addrlen = sizeof(address);
    printf("[+]Waiting for connections ...\n");

    running = 1;

    while (running == 1) {
        FD_ZERO(&read_file_descriptors); //clear the socket set

        FD_SET(master_socket, &read_file_descriptors); //add master socket to set
        max_server_file_descriptor = master_socket;

        for ( i = 0 ; i < max_clients ; i++) { //add child sockets to FD_SET
            server_file_descriptor = client_socket[i];

            if(server_file_descriptor > 0)  //if valid socket descriptor then add to read list
                FD_SET(server_file_descriptor , &read_file_descriptors);

            if(server_file_descriptor > max_server_file_descriptor)  //highest file descriptor number, need it for the select function
                max_server_file_descriptor = server_file_descriptor;
        }

        activity = select( max_server_file_descriptor + 1, &read_file_descriptors, NULL, NULL, NULL); //wait for an activity on one of the sockets, timeout is NULL, so wait indefinitely
        if ((activity == -1) && (errno!=EINTR))
            printf("[-]Error while select");


        if (FD_ISSET(master_socket, &read_file_descriptors)) { //If something happened on the master socket, then its an incoming connection
            if ((new_socket = accept(master_socket, (struct sockaddr *)&address, (socklen_t*)&addrlen)) == -1) {
                printf("[-]Error while accepting new connection\n");
                return -5;
            }
            printf("[+]New connection, socket_file_descriptor is %d, ip is: %s, port: %d\n", new_socket, inet_ntoa(address.sin_addr), ntohs(address.sin_port)); //inform user of socket number - used in send and receive commands

            if (send(new_socket, message, strlen(message), 0) == -1)  //send new connection greeting message
                printf("[-]Error while sending welcome message to client\n");
            printf("[+]Welcome message sent successfully\n");

            for (i = 0; i < max_clients; i++) { //add new socket to array of sockets
                if (client_socket[i] == 0) { //if position is empty
                    client_socket[i] = new_socket;
                    printf("[+]Adding to list of sockets as %d\n" , i);
                    break;
                }
            }
        }

        for (i = 0; i < max_clients; i++) { //else its some IO operation on some other socket
            server_file_descriptor = client_socket[i];

            if (FD_ISSET(server_file_descriptor, &read_file_descriptors)) {
                if ((byte_read = read(server_file_descriptor, buffer, 1024)) == 0) { //Check if it was for closing, and also read the incoming message
                    Q:		getpeername(server_file_descriptor, (struct sockaddr*)&address, (socklen_t*)&addrlen); //Somebody disconnected, get his details and print
                    printf("[+]Client disconnected, ip %s, port %d\n", inet_ntoa(address.sin_addr), ntohs(address.sin_port));

                    close(server_file_descriptor); //Close the socket and mark as 0 in list for reuse
                    client_socket[i] = 0;
                }

                else {
                    buffer[byte_read-2] = '\0'; //set the string terminating NULL byte on the end of the data read
                    printf("[+]Buffer read: [%s]\n", buffer);

                    char* command = strtok(buffer, " "); //Cutting the buffer at the first space
                    printf("[+]Command: [%s]\n", command);

                    if (strcmp(command, "PUT") == 0) {
                        char* key = strtok(NULL, " "); //Cutting the buffer at the second space
                        char* value = strtok(NULL, "\0"); //Cutting buffer at the end

                        printf("[+]Key: [%s]\n", key);
                        printf("[+]Value: [%s]\n", value);

                        strcpy(output, "PUT:");
                        strcat(output, key);
                        strcat(output, ":");
                        strcat(output, value);

                        if (special_character(key) == -1)
                            strcat(output, ":key_invalid\n");

                        else if (special_character(value) == -1)
                            strcat(output, ":value_invalid\n");

                        else {
                            strcat(output, "\n");
                            insert_end(&root, key, value);
                            printf("[+]Added to list: ");
                            printf("%s, %s\n", root->key, root->value);
                            printf("[+]Checking the list\n");
                            for (Node *curr = root; curr != NULL; curr = curr->next)
                                printf("\t\t%s, %s\n", curr->key, curr->value);
                            printf("[+]End of list\n");
                        }
                    }

                    else if (strcmp(command, "GET") == 0) {
                        char* key = strtok(NULL, "\0");
                        printf("[+]Key: [%s]\n", key);

                        strcpy(output, "GET:");
                        strcat(output, key);
                        strcat(output, ":");

                        int key_found = 0;
                        printf("[+]Printing the list before searching\n");
                        for (Node *curr = root; curr != NULL; curr = curr->next)
                            printf("%s, %s\n", curr->key, curr->value);
                        printf("[+]End of list\n");

                        for (Node *curr = root; curr != NULL; curr = curr->next) {
                            printf("[+]Searching for key\n");
                            printf("Curr->key [%s] searched key [%s]\n", curr->key, key);
                            if (strcmp((curr->key), key) == 0) {
                                strcat(output, curr->value);
                                strcat(output, "\n");
                                key_found = 1;
                                break;
                            }
                        }
                        if (root == NULL)
                            printf("[-]list is empty\n");

                        if (key_found == 0) {
                            printf("[+]Key not found\n");
                            strcat(output, "key_nonexistent\n");
                        }
                    }

                    else if (strcmp(command, "DEL") == 0) {
                        char* key = strtok(NULL, "\0");
                        printf("[+]Key: [%s]\n", key);

                        strcpy(output, "DEL:");
                        strcat(output, key);
                        strcat(output, ":");

                        //int element_deleted = remove_element(&root, key);
                        if (remove_element(&root, key) == -1)
                            strcat(output, "key_nonexistent\n");
                        else
                            strcat(output, "key_deleted\n");
                    }

                    else if (strcmp(command, "QUIT") == 0)
                        goto Q;

                    else if (strcmp(command, "END") == 0) {
                        printf("[+]Client terminated server\n");
                        running = 0;
                        break;
                    }

                    else {
                        printf("[-]Command not existent\n");
                        strcpy(output, command);
                        strcat(output, ":command_nonexistent\n");
                    }

                    if (send(server_file_descriptor, output, strlen(output), 0 ) == -1) {
                        printf("[-]Error while sending data to client\n");
                        return -5;
                    }

                    memset(buffer, '\0', sizeof(buffer));
                    memset(output, '\0', sizeof(output));
                }
            }
        }
    }


    printf("[+]Checking the list at the end\n");
    for (Node *curr = root; curr != NULL; curr = curr->next)
        printf("\t\t%s, %s\n", curr->key, curr->value);

    close(server_file_descriptor);
    printf("[+]Client socket closed\n");

    deallocate(&root);
    printf("[+]List deallocated\n");

    return 0;
}


int special_character(char *str) {
    int i = 0;
    while (str[i] != '\0') {
        if (str[i] == '!' || str[i] == '@' || str[i] == '#' || str[i] == '$' || str[i] == '%' ||
            str[i] == '^' || str[i] == '&' || str[i] == '*' || str[i] == '(' || str[i] == ')' ||
            str[i] == '-' || str[i] == '{' || str[i] == '}' || str[i] == '[' || str[i] == ']' ||
            str[i] == ':' || str[i] == ';' || str[i] == '"' || str[i] == '\'' || str[i] == '<'||
            str[i] == '>' || str[i] == '.' || str[i] == '/' || str[i] == '?' || str[i] == '~' ||
            str[i] == '`' || str[i] == ' ')
        {
            return -1;
        }
        i++;
    }
    return 0;
}
