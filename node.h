#include "blockchain.h"

#define PORT 6310
#define MAX_NODE 10

typedef struct miner{
    int index;
    int nonce;
    block current;
} miner;

typedef struct bound{
    unsigned int index;
    unsigned int top;
    unsigned int bottom;
} bound;

int start(int port, int http_port, int node_port);
void mining();
void* bruteforce(void* boundary);
void receiving(int local_fd);
void *receive_thread(void *local_fd);
void broadcast(node buffer);
ssize_t send_to_server(int index, node buffer);
ssize_t send_to_client(int index, node buffer);
void on_client_received(int client_fd, node buffer);
void on_server_received(int server_fd, node buffer);
void on_mined_block(node data);
void on_new_transaction(node data);