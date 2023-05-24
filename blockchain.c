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
int challenge(block* prev, char sender[PUBLIC_ADDRESS_SIZE], int nonce) {
    return 0;
}
block* mine(block* prev, int nonce, char sender[PUBLIC_ADDRESS_SIZE]) {
    return NULL;
}
char* hash_block(block* block) {
    return NULL;
}
void serializeNode(node* data, unsigned char* buffer) {
    memcpy(buffer, data, sizeof(node));
}
void deserializeNode(node* data, unsigned char* buffer) {
    memcpy(data, buffer, sizeof(node));
}