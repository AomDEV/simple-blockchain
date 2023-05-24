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
int challenge(block current, char sender[PUBLIC_ADDRESS_SIZE], int nonce) {
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
block* mine(block current, int nonce, char sender[PUBLIC_ADDRESS_SIZE]) {
    block* new_block = malloc(sizeof(block));
    new_block->index = current.index + 1;
    new_block->timestamp = time(NULL);
    memset(new_block->trans_list, 0, sizeof(new_block->trans_list));
    memset(new_block->current_hash, 0, HASH_HEX_SIZE);
    new_block->trans_list_length = 0;
    new_block->nonce = 0;
    new_block->prev = &current;
    strcpy(new_block->prev->current_hash, hash_block(current));
    new_block->prev->nonce = nonce;

    // add first transaction to block
    transaction txn;
    txn.timestamp = time(NULL);
    strcpy(txn.sender, get_empty_address());
    strcpy(txn.recipient, sender);
    txn.amount = 100;

    new_block->trans_list[0] = txn;
    new_block->trans_list_length++;

    return new_block;
}
char* hash_transaction(transaction txn[TRANS_LIST_SIZE]) {
    return NULL;
}
char* hash_block(block block) {
    return NULL;
}
void serializeNode(node* data, unsigned char* buffer) {
    memcpy(buffer, data, sizeof(node));
}
void deserializeNode(node* data, unsigned char* buffer) {
    memcpy(data, buffer, sizeof(node));
}
char* get_empty_address() {
    char* address = malloc(PUBLIC_ADDRESS_SIZE);
    for (int i = 0; i < PUBLIC_ADDRESS_SIZE; i++) address[i] = '0';
    address[1] = 'x';
    return address;
}