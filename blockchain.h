#define PUBLIC_ADDRESS_SIZE 64
#define TRANS_LIST_SIZE 20
#define HASH_HEX_SIZE 65
#define MESSAGE_SIZE 1024
#define HASH_SIZE 32
#define BLOCK_STR_SIZE 30000
#define BLOCK_BUFFER_SIZE 5000
#define MAX_NONCE INT32_MAX
#define DIFFICULTY 2
#define BLOCK_REWARD 100

// Transaction structure
typedef struct transaction {
    unsigned int timestamp;
    char sender[PUBLIC_ADDRESS_SIZE];
    char recipient[PUBLIC_ADDRESS_SIZE];
    unsigned int amount;
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
    current_balance,
};
typedef struct node_dict {
    char key[PUBLIC_ADDRESS_SIZE];
    int value;
    size_t size;
} node_dict;
typedef struct node {
    char sender[PUBLIC_ADDRESS_SIZE];
    enum node_function function;
    char message[MESSAGE_SIZE];
    transaction txn;
    block current;
    block prev;
    node_dict balance;
} node;

block* create_genesis_block();
char* hash_block(block* block, char sender[PUBLIC_ADDRESS_SIZE], int nonce);
char* hash_transaction(transaction txn);
char* hash_transactions(transaction* trans_list, unsigned int trans_list_length);
int challenge(block* current, char sender[PUBLIC_ADDRESS_SIZE], int nonce);
transaction* add_transaction(char sender[PUBLIC_ADDRESS_SIZE], char recipient[PUBLIC_ADDRESS_SIZE], int amount, char signature);
block* mine(block* current, int nonce, char sender[PUBLIC_ADDRESS_SIZE]);
int balance(char address[PUBLIC_ADDRESS_SIZE]);
int get_current_block();
transaction* get_transactions();
void serializeNode(node* data, unsigned char* buffer);
void deserializeNode(node* data, unsigned char* buffer);
int get_difficulty(block prev);
char* get_empty_address();
