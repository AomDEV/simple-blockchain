#include <openssl/evp.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <pthread.h>

#include "node.h"
#include "httpd.h"
#include "util.h"

block* chain;
int nodes[FD_SETSIZE];
int server_fd = 0;
struct sockaddr_in server_address;
int client_df = 0;
struct sockaddr_in client_address;
char address[PUBLIC_ADDRESS_SIZE];

// HTTP Routes
void route() {
    ROUTE_START();
    ROUTE_GET("/")
    {
        printf("HTTP/1.1 200 OK\r\n\r\n");
        printf("Hello! You are using %s", request_header("User-Agent"));
    }
    ROUTE_END();
}

// Start Node
int start(int port, int http_port, int node_port) {
    printf("\nNode started.\n");
    sleep(3);

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        printf("\nSocket failed.\n");
        return (EXIT_FAILURE);
    }
    
    // Node Connection
    if ((client_df = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\nSocket creation error.\n");
        return (EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    if(port <= 0) {
        server_address.sin_port = htons(PORT);
    } else {
        server_address.sin_port = htons(port);
    }

    // Printed the server socket addr and port
    printf("IP Address: %s\n", inet_ntoa(server_address.sin_addr));
    printf("Port: %d\n", (int)ntohs(server_address.sin_port));
    if(node_port > 0) printf("Node: %d\n", node_port);

    if(http_port <= 0) {
        http_port = 8080;
    }

    pthread_t wid;
    pthread_create(&wid, NULL, &serve_forever, &http_port);

    // Master Node Address Configuration
    if(node_port > 0) {
        client_address.sin_family = AF_INET;
        client_address.sin_addr.s_addr = INADDR_ANY; //INADDR_ANY always gives an IP of 0.0.0.0
        client_address.sin_port = htons(node_port);
        if (connect(client_df, (struct sockaddr *)&client_address, sizeof(client_address)) < 0)
        {
            printf("\nConnection Failed.\n");
            return (EXIT_FAILURE);
        }
        printf("\nConnected to node #%d.\n", client_df);

        node buffer;
        buffer.function = connected;
        strcpy(buffer.message, "Hello from client");

        send_to_server(client_df, buffer);
    }

    if (bind(server_fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        printf("bind failed");
        return (EXIT_FAILURE);
    }
    if (listen(server_fd, 5) < 0)
    {
        printf("listen");
        return (EXIT_FAILURE);
    }

    // Creating thread to keep receiving message in real time
    pthread_t tid;
    pthread_create(&tid, NULL, &receive_thread, &server_fd);

    pthread_t node;
    if(node_port > 0) {
        pthread_create(&node, NULL, &receive_thread, &client_df);
    }

    mining();

    if(node_port > 0) close(client_df);
    close(server_fd);
    return 0;
}

// Mining
void mining() {
    char test;
    scanf("%c", &test);
    char test2;
    scanf("%c", &test2);
}

// Calling receiving every 2 seconds
void *receive_thread(void *local_fd)
{
    int s_fd = *((int *)local_fd);
    while (1)
    {
        sleep(2);
        receiving(s_fd);
    }
}

// Receiving messages on our port
void receiving(int local_fd)
{
    struct sockaddr_in address;
    int valread;
    node buffer;
    int addrlen = sizeof(address);
    fd_set current_sockets, ready_sockets;

    //Initialize my current set
    FD_ZERO(&current_sockets);
    FD_SET(local_fd, &current_sockets);
    int k = 0;
    while (1)
    {
        k++;

        ready_sockets = current_sockets;

        if (select(FD_SETSIZE, &ready_sockets, NULL, NULL, NULL) < 0)
        {
            perror("Error");
            exit(EXIT_FAILURE);
        }

        for (int i = 0; i < FD_SETSIZE; i++)
        {
            if (!FD_ISSET(i, &ready_sockets)) continue;
            if (i == local_fd && local_fd == server_fd)
            {
                int client_socket;

                if ((client_socket = accept(local_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
                    perror("Accept");
                    exit(EXIT_FAILURE);
                }
                FD_SET(client_socket, &current_sockets);
                nodes[client_socket] = 1;
                printf("\nNode #%d connected.\n", client_socket);
            }
            else
            {
                unsigned char data[sizeof(node)];
                valread = recv(i, data, sizeof(node), 0);
                deserializeNode(&buffer, data);
                // valread = recv(i, (void*)&buffer, sizeof(buffer), 0);
                if(valread == 0 && local_fd == server_fd) {
                    printf("\nNode #%d disconnected.\n", i);
                    FD_CLR(i, &current_sockets);
                    nodes[i] = 0;
                    continue;
                }
                if(valread > 0) {
                    if(local_fd == server_fd) {
                        printf("\n[NODE#%d] Received: %u\n", i, buffer.function);
                        on_server_received(i, buffer);
                    }
                    if(local_fd == client_df) {
                        printf("\n[NODE] Received: %u\n", buffer.function);
                        on_client_received(i, buffer);
                    }
                }
                if(valread <= 0 && local_fd == client_df) {
                    printf("\nNode disconnected.\n");
                    exit(EXIT_FAILURE);
                }
            }
        }
        if (k == (FD_SETSIZE * 2)) break;
    }
}

//Free all outstanding memory
void graceful_shutdown(int dummy) {
    exit(0);
}

// Executor
int main(int argc, const char* argv[]) {
    //Ctrl-C Handler
    signal(SIGINT, graceful_shutdown);

    //Initialize Crypto
    OpenSSL_add_all_algorithms();
    OpenSSL_add_all_ciphers();

    //Begin mining
    char* port_args[2] = {"-p", "-port"};
    int port = get_int_args(argv, port_args, 2);

    char* node_args[1] = {"-node"};
    int node = get_int_args(argv, node_args, 1);

    char* http_args[1] = {"-http"};
    int http = get_int_args(argv, http_args, 1);

    char* owner_args[1] = {"-address"};
    const char* owner = get_raw_args(argv, owner_args, 1);
    if(owner != NULL) strcpy(address, owner);

    if(chain == NULL && node <= 0) {
        chain = create_genesis_block();
        printf("\nGenesis block #%d created\n", chain->index);
    }

    start(port, http, node);    

    return 0;
}

ssize_t send_to_server(int index, node data) {
    unsigned char buffer[sizeof(node)];
    serializeNode(&data, buffer);
    return sendto(index, buffer, sizeof(node), 0, (struct sockaddr *)&server_address, sizeof(server_address));
    // return sendto(index, &buffer, sizeof(buffer), 0, (struct sockaddr *)&server_address, sizeof(server_address));
}
ssize_t send_to_client(int index, node data) {
    unsigned char buffer[sizeof(node)];
    serializeNode(&data, buffer);
    return sendto(index, buffer, sizeof(node), 0, (struct sockaddr *)&server_address, sizeof(server_address));
    // return sendto(index, &buffer, sizeof(buffer), 0, (struct sockaddr *)&client_address, sizeof(client_address));
}

void broadcast(node buffer) {
    for (int i = 0; i < FD_SETSIZE; i++) {
        if(nodes[i] <= 0) continue;
        send_to_client(i, buffer);
    }
}

void on_client_received(int client_fd, node buffer) {
    node response;
    switch (buffer.function) {
        case connected:
            response.function = current_block;
            strcpy(response.message, "Current Block");
            break;
        case current_block:
            printf("\n[INFO] Current Block: #%d\n", buffer.block.index);
            return;
        case mined_block:
            if(buffer.block.index == chain->index && chain->prev != NULL) {
                printf("\n[INFO] Mined Block #%d by %s\n", buffer.block.index, buffer.sender);
                on_mined_block(buffer);
                return;
            }
            return;
        default:
            return;
    }
    send_to_server(client_fd, response);
    printf("\n[INFO] Sent %lu bytes\n", sizeof(response));
}

void on_server_received(int server_fd, node buffer) {
    node response;
    int is_broadcast = 0;
    switch (buffer.function) {
        case connected:
            response.function = connected;
            strcpy(response.message, "Hello from server");
            break;
        case disconnected:
            return;
        case current_block:
            response.function = current_block;
            response.block = *chain;
            response.block.prev = NULL;
            strcpy(response.message, "Current Block");
            break;
        case mined_block:
            is_broadcast = 1;
            on_mined_block(buffer);
            response.function = mined_block;
            strcpy(response.sender, buffer.sender);
            response.block = *(chain->prev);
            strcpy(response.message, "Mined Block");
            break;
        case new_transaction:
            is_broadcast = 1;
            if(chain->trans_list_length == 0 || chain->trans_list[chain->trans_list_length - 1].hash != buffer.txn.hash) {
               chain->trans_list[chain->trans_list_length] = buffer.txn; 
               chain->trans_list_length++;
            }
            response.function = new_transaction;
            response.txn = buffer.txn;
            strcpy(response.message, "New Transaction");
            break;
    }
    if(is_broadcast > 0) {
        broadcast(response);
    } else{
        send_to_client(server_fd, response);
    }
    printf("\n[INFO] Sent %lu bytes\n", sizeof(response));
}

void on_mined_block(node data) {
    if(challenge(chain, data.sender, data.block.nonce) >= 1) {
        chain = mine(chain, data.block.nonce, data.sender);
    };
}

void on_new_transaction(node data) {
    // do something
}