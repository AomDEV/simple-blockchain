#include "blockchain.h"

#define PORT 6310
#define MAX_NODE 10

int start(int port, int http_port, int node_port);
void mining();
void receiving(int local_fd);
void *receive_thread(void *local_fd);
void broadcast(node buffer);
ssize_t send_to_server(int index, node buffer);
ssize_t send_to_client(int index, node buffer);
void on_client_received(int client_fd, node buffer);
void on_server_received(int server_fd, node buffer);