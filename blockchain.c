#include <stdio.h>
#include <stdlib.h>
#include <openssl/sha.h>
#include <time.h>
#include <string.h>
#include "blockchain.h"

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
    char* block_hash = hash_block(current);
    if(block_hash == NULL) return 0;
    char* head = malloc(DIFFICULTY);
    memcpy(&head, block_hash , DIFFICULTY);

    // Build answer with DIFFICULTY 0's
    char* answer = malloc(DIFFICULTY);
    for (int i = 0; i < DIFFICULTY; i++) answer[i] = '0';

    if(head == answer) return 1;
    return 0;
}
block* mine(block* current, int nonce, char sender[PUBLIC_ADDRESS_SIZE]) {
    block* new_block = malloc(sizeof(block));
    new_block->index = current->index + 1;
    new_block->timestamp = time(NULL);
    memset(new_block->trans_list, 0, sizeof(new_block->trans_list));
    memset(new_block->current_hash, 0, HASH_HEX_SIZE);
    new_block->trans_list_length = 0;
    new_block->nonce = 0;
    new_block->prev = current;
    strcpy(new_block->prev->current_hash, hash_block(current));
    new_block->prev->nonce = nonce;

    // add first transaction to block
    transaction txn;
    txn.timestamp = time(NULL);
    strncpy(txn.sender, get_empty_address(), PUBLIC_ADDRESS_SIZE);
    strncpy(txn.recipient, sender, PUBLIC_ADDRESS_SIZE);
    txn.amount = 100;

    new_block->trans_list[0] = txn;
    new_block->trans_list_length++;

    return new_block;
}
char* hash_transaction(transaction txn[TRANS_LIST_SIZE]) {
    return "ABC";
}
char* hash_block(block* block) {
    return "ABC";
}
void serializeNode(node* data, unsigned char* buffer) {
    memcpy(buffer, data, sizeof(node));
}
void deserializeNode(node* data, unsigned char* buffer) {
    memcpy(data, buffer, sizeof(node));
}
char* get_empty_address() {
    char* addr = (char*)malloc(PUBLIC_ADDRESS_SIZE * sizeof(char));
    for (int i = 0; i < PUBLIC_ADDRESS_SIZE - 1; i++) {
        if (i == 1) {
            addr[i] = 'x';
        } else {
            addr[i] = '0';
        }
    }

    // Null-terminate the string
    addr[PUBLIC_ADDRESS_SIZE - 1] = '\0';
    return addr;
}