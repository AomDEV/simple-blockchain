#define PUBLIC_ADDRESS_SIZE 500
#define TRANS_LIST_SIZE 20
#define HASH_HEX_SIZE 65
#define MESSAGE_SIZE 1024

// Transaction structure
typedef struct transaction {
    int timestamp;
    char sender[PUBLIC_ADDRESS_SIZE];
    char recipient[PUBLIC_ADDRESS_SIZE];
    int amount;
    char hash[HASH_HEX_SIZE];
} transaction;

// Block Structure
typedef struct block {
    unsigned int index;
    unsigned int timestamp;
    transaction trans_list[TRANS_LIST_SIZE];
    unsigned int trans_list_length;
    char current_hash[HASH_HEX_SIZE];
    unsigned int nonce;
    struct block* prev;
} block;

// Node Structure
enum node_function {
    connected,
    disconnected,
    current_block,
    mined_block,
    new_transaction,
};
typedef struct node {
    char sender[PUBLIC_ADDRESS_SIZE];
    enum node_function function;
    char message[MESSAGE_SIZE];
    transaction txn;
    block current;
    block prev;
} node;

block* create_genesis_block();
char* hash_block(block* block);
char* hash_transaction(transaction txn[TRANS_LIST_SIZE]);
int challenge(block* prev, char sender[PUBLIC_ADDRESS_SIZE], int nonce);
block* create_new_block(block* prev, int nonce);
transaction* add_transaction(char sender[PUBLIC_ADDRESS_SIZE], char recipient[PUBLIC_ADDRESS_SIZE], int amount, char signature);
block* mine(block* prev, int nonce, char sender[PUBLIC_ADDRESS_SIZE]);
int balance(char address[PUBLIC_ADDRESS_SIZE]);
int get_current_block();
transaction* get_transactions();
void serializeNode(node* data, unsigned char* buffer);
void deserializeNode(node* data, unsigned char* buffer);