#include <stdlib.h>
#include <openssl/sha.h>
#include "blockchain.h"

block* create_genesis_block() {
    block* new_block = malloc(sizeof(block));
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