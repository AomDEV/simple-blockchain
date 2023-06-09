#include <stdio.h>
#include <stdlib.h>
#include <openssl/sha.h>
#include <time.h>
#include <string.h>
#include "blockchain.h"


void hash256(unsigned char* output, const char* input) {

    size_t length = strlen(input);
    unsigned char md[32];
    SHA256((const unsigned char*)input, length, md);
    memcpy(output,md, 32);

    return;
}


block* create_genesis_block() {
    block* new_block = malloc(sizeof(block));

    new_block->index = 0;
    new_block->timestamp = time(NULL);
    new_block->prev = NULL;
    new_block->nonce = 0;
    memset(new_block->current_hash, 0, HASH_HEX_SIZE);
    memset(new_block->trans_list, 0, sizeof(new_block->trans_list));
    new_block->trans_list_length = 0;
    return new_block;
}
int challenge(block* current, char sender[PUBLIC_ADDRESS_SIZE], int nonce) {
    // if(nonce == 1503238541+100) return 1; // debug
    if(current == NULL) return 0;
    char* block_hash = hash_block(current, sender, nonce);
    if(block_hash == NULL) return 0;
    char* head = malloc(DIFFICULTY);
    strncpy(head, block_hash , DIFFICULTY);

    // Build answer with DIFFICULTY 0's
    char* answer = malloc(DIFFICULTY);
    strncpy(answer, get_filled_char(DIFFICULTY), DIFFICULTY);

    // compare head and answer
    if(strcmp(head, answer) == 0) return 1;
    return 0;
}
block* mine(block* current, int nonce, char sender[PUBLIC_ADDRESS_SIZE]) {
    if(current == NULL) {
        printf("\nError: current block is NULL\n");
        return NULL;
    }
    block* new_block = malloc(sizeof(block));
    new_block->index = current->index + 1;
    new_block->timestamp = time(NULL);
    memset(new_block->trans_list, 0, sizeof(new_block->trans_list));
    memset(new_block->current_hash, 0, HASH_HEX_SIZE);
    new_block->trans_list_length = 0;
    new_block->nonce = 0;
    new_block->prev = current;
    strncpy(new_block->prev->current_hash, hash_block(current, sender, nonce), HASH_HEX_SIZE);
    new_block->prev->nonce = nonce;

    // add first transaction to block
    char zero_addr[PUBLIC_ADDRESS_SIZE];
    char* empty_char = get_empty_char(PUBLIC_ADDRESS_SIZE - 1);
    strncpy(zero_addr, empty_char, PUBLIC_ADDRESS_SIZE - 1);
    zero_addr[PUBLIC_ADDRESS_SIZE - 1] = '\0';

    struct transaction txn;
    txn.timestamp = time(NULL);
    strcpy(txn.sender, zero_addr);
    strcpy(txn.recipient, sender);
    txn.amount = 100;

    new_block->trans_list[0] = txn;
    new_block->trans_list_length = 1;

    return new_block;
}

char* hash_transaction(transaction txn) {
    char current_txn[BLOCK_BUFFER_SIZE];
    sprintf(current_txn, "%d %s %s %d",
        txn.timestamp,
        txn.sender,
        txn.recipient,
        txn.amount
    );
    unsigned char hashed[HASH_SIZE];
    hash256(hashed, current_txn);

    char* hex_hash = malloc(HASH_HEX_SIZE + 1);
	char* buffer = malloc(3);
        
    for (int i = 0; i < HASH_SIZE; i++) {
        memset(buffer, 0, 3);
        sprintf(buffer, "%02x", hashed[i]);
        strcat(hex_hash, buffer);
    }
    free(buffer);
    
    return hex_hash;
}

char* hash_transactions(transaction* trans_list, unsigned int trans_list_length) {
	char txns[BLOCK_STR_SIZE + 3000] = {0};
    for(int i=0; i < trans_list_length; i++) strcat(txns, hash_transaction(trans_list[i]));
        
    unsigned char hashed[HASH_SIZE];
    hash256(hashed, txns);
    
    char* hex_hash = malloc(HASH_HEX_SIZE + 1);
	char* buffer = malloc(3);
        
    for (int i = 0; i < HASH_SIZE; i++) {
        memset(buffer, 0, 3);
        sprintf(buffer, "%02x", hashed[i]);
        strcat(hex_hash, buffer);
    }
    free(buffer);
    return hex_hash;

}

char* hash_block(block* block, char sender[PUBLIC_ADDRESS_SIZE], int nonce) {
    char* transactions_hash = hash_transactions(block->trans_list, block->trans_list_length);
    char* block_hash = block->prev == NULL ? get_empty_hash() : block->prev->current_hash;
    char current_block[BLOCK_BUFFER_SIZE];
    sprintf(current_block, "%u %u %s %u %d %s",
        block->index,
        block->timestamp,
        transactions_hash,
        block->trans_list_length,
        nonce,
        block_hash
    );
    unsigned char hashed[HASH_SIZE];
    hash256(hashed, current_block);
    char* hex_hash = malloc(HASH_HEX_SIZE + 1);
    char* buffer = malloc(3);
    for (int i = 0; i < HASH_SIZE; i++) {
        memset(buffer, 0, 3);
        sprintf(buffer, "%02x", hashed[i]);
        strcat(hex_hash, buffer);
    }
    free(buffer);
    return hex_hash;
}

void serializeNode(node* data, unsigned char* buffer) {
    memcpy(buffer, data, sizeof(node));
}

void deserializeNode(node* data, unsigned char* buffer) {
    memcpy(data, buffer, sizeof(node));
}

char* get_filled_char(size_t size) {
    size_t new_size = size + 1;
    char* addr = (char*)malloc(new_size * sizeof(char));
    for (int i = 0; i < new_size - 1; i++) addr[i] = '0';

    // Null-terminate the string
    addr[new_size - 1] = '\0';
    return addr;
}
char* get_empty_char(size_t size) {
    char* filled = get_filled_char(size + 1);
    filled[1] = 'x';
    return filled;
}
char* get_empty_hash() {
    return get_empty_char(HASH_HEX_SIZE);
}
char* get_empty_address() {
    return get_empty_char(PUBLIC_ADDRESS_SIZE);
}
