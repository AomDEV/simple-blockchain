#include <openssl/evp.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>

#include "node.h"
#include "httpd.h"
#include "util.h"
#include "dict.h"

block* chain = NULL;
int nodes[FD_SETSIZE];
int server_fd = 0;
struct sockaddr_in server_address;
int client_df = 0;
struct sockaddr_in client_address;
char address[PUBLIC_ADDRESS_SIZE];
int max_thread = 5;
dict* balances;
struct miner nonce;
int socket_ready = 0;
int found_nonce = 0;
// index:0 - HTTP
// index:1 - Server
// index:2 - Client
pthread_t processors[4];
pthread_mutex_t lock;
pthread_cond_t cond;

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
    // Initialize synchronization variables
    pthread_mutex_init(&lock, NULL);
    pthread_cond_init(&cond, NULL);

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

    // Serve HTTP
    pthread_create(&processors[0], NULL, &serve_forever, &http_port);

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
    if(pthread_create(&processors[1], NULL, &receive_thread, &server_fd) != 0) {
        fprintf(stderr, "\nFailed to create server thread\n");
        return (EXIT_FAILURE);
    }

    if(node_port > 0) {
        if(pthread_create(&processors[2], NULL, &receive_thread, &client_df)) {
            fprintf(stderr, "\nFailed to create client thread\n");
            return (EXIT_FAILURE);
        }
    }

    if(pthread_create(&processors[3], NULL, &mining, NULL) != 0) {
        fprintf(stderr, "\nFailed to create mining thread\n");
        return (EXIT_FAILURE);
    };

    // Wait for all threads to finish (except the abandoned socket threads)
    for (int i = 0; i < sizeof(processors); i++) {
        if (pthread_join(processors[i], NULL) != 0) {
            fprintf(stderr, "\nFailed to join socket thread #%d\n", i);
            return 1;
        }
    }

    // Cleanup synchronization variables
    pthread_mutex_destroy(&lock);
    pthread_cond_destroy(&cond);

    if(node_port > 0) close(client_df);
    close(server_fd);
    return 0;
}

// Mining
void* mining() {
    // Wait for the socket threads to be ready
    pthread_mutex_lock(&lock);
    while (socket_ready < 1 || chain == NULL) pthread_cond_wait(&cond, &lock);
    pthread_mutex_unlock(&lock);

    // Perform the loop operations
    printf("\nStart Mining (%d thread)\n", max_thread);
    sleep(1);
    while (1) {
        if(max_thread <= 0) {
            sleep(1);
            continue;
        }

        printf("\n=== Mining for Block #%d ===\n", chain->index);
        found_nonce = 0;
        pthread_t threads[max_thread];
        bound boundaries[max_thread];
        
        nonce.index = chain->index;
        nonce.current = *chain;

        for(int t = 0; t < max_thread; t++) {
            const unsigned int per_thread = (MAX_NONCE / max_thread) - 1;
            const unsigned int min = per_thread * t;
            const unsigned int max = min + per_thread;    
            
            boundaries[t].index = t;
            boundaries[t].bottom = min;
            boundaries[t].top = max;

            pthread_create(&threads[t], NULL, bruteforce, &boundaries[t]);
        }
        void* found;
        for (int t = 0; t < max_thread; t++) {
            if(pthread_join(threads[t], &found) != 0) {
                fprintf(stderr, "\nFailed to join socket thread %d\n", t);
                pthread_exit(NULL);
                break;
            }
        }
        sleep(1);
    }
    printf("Mining thread finished\n");
    pthread_exit(NULL);
}

void* bruteforce(void* boundary) {
    bound* data = (bound*)boundary;
    printf("\nThread #%d is now executing (%d-%d)\n", data->index, data->bottom, data->top);
    for (int i = data->bottom; i < data->top; i++) {
        if(found_nonce > 0) break;
        int result = challenge(&nonce.current, address, i);
        if(result > 0) {
            nonce.nonce = i;
            printf("\nThread #%d found nonce (%d)\n", data->index, i);
            
            // convert pointer to non-pointer
            block* mined = mine(chain, i, address);
            node buffer;
            buffer.function = mined_block;
            strcpy(buffer.sender, address);
            buffer.current = *mined;
            buffer.current.nonce = i;
            buffer.current.prev = NULL;
            buffer.prev = *chain;
            buffer.prev.prev = NULL;

            on_mined_block(buffer.prev, buffer.current, buffer.sender);
            printf("\n=== Block #%d mined ===\n", chain->index);

            broadcast(buffer);
            
            pthread_mutex_lock(&lock);
            found_nonce = 1;
            pthread_mutex_unlock(&lock);

            pthread_exit((void*)1);
            break;
        }
    }
    printf("\nThread #%d terminated\n", data->index);
    pthread_exit((void*)0);
}

// Calling receiving every 2 seconds
void *receive_thread(void *local_fd)
{
    // Signal that the socket thread is ready
    pthread_mutex_lock(&lock);
    socket_ready++;
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&lock);

    int s_fd = *((int *)local_fd);
    printf("\n[NODE#%d] Ready\n", s_fd);

    pthread_detach(pthread_self());

    while (1)
    {
        sleep(2);
        receiving(s_fd);
    }
    printf("Node thread finished\n");
    pthread_exit(NULL);
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
    if(owner != NULL) strncpy(address, owner, PUBLIC_ADDRESS_SIZE);

    char* thread_args[2] = {"-t", "-thread"};
    int thread = get_int_args(argv, thread_args, 2);
    max_thread = thread;

    balances = dict_create();
    if(chain == NULL && node <= 0) {
        pthread_mutex_lock(&lock);
        chain = create_genesis_block();
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&lock);

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
            response.function = current_balance;
            strcpy(response.message, "Current Balance");
            on_current_block(buffer.current, buffer.prev);
            break;
        case mined_block:
            if(chain == NULL) return;
            if(buffer.current.index == chain->index) {
                printf("\n[INFO] Mined Block #%d by %s\n", buffer.current.index, buffer.sender);
                on_mined_block(buffer.prev, buffer.current, buffer.sender);
                return;
            }
            return;
        case current_balance:
            dict_insert(balances, buffer.balance.key, &buffer.balance.value, buffer.balance.size);
            return;
        case new_transaction:
            if(buffer.sender != buffer.txn.sender) {
                printf("\n[INFO] Invalid transaction (%s)\n", buffer.txn.hash);
                return;
            }
            printf("\n[INFO] Transaction Hash %s (%s to %s)\n", buffer.txn.hash, buffer.txn.sender, buffer.txn.recipient);

            transaction txns[TRANS_LIST_SIZE];
            txns[0] = buffer.txn;
            on_new_transaction(*chain, txns, 1);
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
            response.current = *chain;
            response.current.prev = NULL;
            if(chain->prev != NULL) {
                response.prev = *(chain->prev);
                response.prev.prev = NULL;
            }
            strcpy(response.message, "Current Block");
            break;
        case current_balance:
            response.function = current_balance;
            char** keys = dict_get_all_keys(balances);
            if (keys != NULL) {
                for (int i = 0; keys[i] != NULL; i++) {
                    node_dict nodedict;
                    printf("\ni: %d\n",i);
                    printf("\nKey: %s\n", keys[i]);
                    strncpy(nodedict.key, keys[i], PUBLIC_ADDRESS_SIZE);
                    int value = *(int*)dict_access(balances, keys[i]);
                    nodedict.value = value;
                    nodedict.size = sizeof(nodedict.value);
                    response.balance = nodedict;
                    send_to_client(server_fd, response);
                }
                for (int i = 0; keys[i] != NULL; i++) {
                    free(keys[i]);
                }
                free(keys);
            }
            return;
        case mined_block:
            is_broadcast = 1;
            on_mined_block(buffer.prev, buffer.current, buffer.sender);
            response.function = mined_block;
            strcpy(response.sender, buffer.sender);
            response.current = *(chain->prev);
            strcpy(response.message, "Mined Block");
            break;
        case new_transaction:
            is_broadcast = 1;

            transaction txns[TRANS_LIST_SIZE];
            txns[0] = buffer.txn;
            on_new_transaction(*chain, txns, 1);

            response.function = new_transaction;
            response.txn = buffer.txn;
            response.current = buffer.current;
            strcpy(response.sender, buffer.txn.sender);
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

void on_current_block(block current, block prev) {
    pthread_mutex_lock(&lock);
    if(current.timestamp <= 0) {
        chain = create_genesis_block();
        balances = dict_create();
    } else {
        chain = &current;
        chain->prev = &prev;
    }
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&lock);

    printf("\n[INFO] Current Block: #%d\n", chain->index);
}

void on_mined_block(block prev, block current, char sender[PUBLIC_ADDRESS_SIZE]) {
    if(chain->prev == NULL) {
        chain->prev = &prev;
    }
    if(challenge(chain, sender, current.nonce) >= 1) {
        pthread_mutex_lock(&lock);
        chain = mine(chain, current.nonce, sender);
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&lock);

        on_new_transaction(*chain, chain->trans_list, chain->trans_list_length);
    };
    printf("\nBalance of %s: %d\n", sender, *((int*)dict_access(balances, sender)));
}

void on_new_transaction(block current, transaction txns[TRANS_LIST_SIZE], unsigned int trans_list_length) {
    if(chain->index != current.index) {
        printf("\n[INFO] Invalid transaction (%s)\n", txns[0].hash);
        return;
    };
    // do something
    for (int i = 0; i < trans_list_length; i++) {
        if(chain->trans_list_length >= TRANS_LIST_SIZE) {
            printf("\n[INFO] Transaction is full\n");
            return;
        };
        int current_index = chain->trans_list_length;
        chain->trans_list[current_index] = txns[i];
        chain->trans_list_length++;

        // do account balance update
        void* sender_funds = dict_access(balances, txns[i].sender);
        int sender_balance = 0;
        if(sender_funds != NULL) sender_balance = *((int*)sender_funds);


        void* recipient_funds = dict_access(balances, txns[i].recipient);
        int recipient_balance = 0;
        if(recipient_funds != NULL) recipient_balance = *((int*)recipient_funds);

        int is_mint = strcmp(txns[i].sender, get_empty_address()) == 0 ? 1 : 0;

        // if sender amount is less than transaction amount and not zero address (mint)
        if(sender_balance < txns[i].amount && is_mint <= 0) {
            printf("\n[INFO] %s Insufficient funds (%s)\n", txns[i].sender, txns[i].hash);
            return;
        }
        // if not mint
        if(is_mint <= 0) {
            sender_balance -= txns[i].amount;
        };
        recipient_balance += txns[i].amount;

        dict_insert(balances, txns[i].sender, &sender_balance, sizeof(sender_funds));
        dict_insert(balances, txns[i].recipient, &recipient_balance, sizeof(recipient_funds));

        // hash transaction
        transaction* tx = &txns[i];
        strncpy(chain->trans_list[current_index].hash, hash_transaction(tx), HASH_HEX_SIZE);
    }
}