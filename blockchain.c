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

int challenge(block* prev, char sender[PUBLIC_ADDRESS_SIZE], int nonce) {
    return 0;
}
block* mine(block* prev, int nonce, char sender[PUBLIC_ADDRESS_SIZE]) {
    return NULL;
}

char* hash_transaction(transaction* trans_list, unsigned int trans_list_length){

	char all_transactions[BLOCK_STR_SIZE + 3000] = {0};
    
    	for(int i=0; i < trans_list_length; i++){
    
    		char this_transaction[BLOCK_BUFFER_SIZE];
    		sprintf(this_transaction, "%d %s %s %d ",
    			trans_list[i].timestamp,
    			trans_list[i].sender,
    			trans_list[i].recipient,
    			trans_list[i].amount
    		);
        	strcat(all_transactions, this_transaction);
    	}
        
        unsigned char hashvalue[HASH_SIZE];
        hash256(hashvalue,all_transactions);
        
        char* hex_hash = malloc(HASH_HEX_SIZE + 1);
	char* buffer = malloc(3);
        
        for (int i = 0; i < HASH_SIZE; i++) {
        	memset(buffer, 0, 3);
        	sprintf(buffer, "%02x", hashvalue[i]);
        	strcat(hex_hash, buffer);
    	}
    	printf("HASH OF Transaction: '%s'\n", hex_hash + 5);
    	free(buffer);
    	return hex_hash;

}

char* hash_block(block* block, char sender[PUBLIC_ADDRESS_SIZE], int nonce) {
    char all_blocks[BLOCK_STR_SIZE + 3000] = {0};
    char* transaction_hash = hash_transaction(block->trans_list, block->trans_list_length);
    char this_block[BLOCK_BUFFER_SIZE];
    sprintf(this_block, "%u %u %s %u %u %s", block->index, block->timestamp, transaction_hash, block->trans_list_length, nonce, block->prev->current_hash);
    strcat(all_blocks, this_block);
    unsigned char hashvalue[HASH_SIZE];
    hash256(hashvalue, all_blocks);
    char* hex_hash = malloc(HASH_HEX_SIZE + 1);
    char* buffer = malloc(3);
    for (int i = 0; i < HASH_SIZE; i++) {
        memset(buffer, 0, 3);
        sprintf(buffer, "%02x", hashvalue[i]);
        strcat(hex_hash, buffer);
    }
    printf("HASH OF BLOCKS: '%s'\n", hex_hash + 5);
    free(buffer);
    return hex_hash;
}


	
void serializeNode(node* data, unsigned char* buffer) {
    memcpy(buffer, data, sizeof(node));
}
void deserializeNode(node* data, unsigned char* buffer) {
    memcpy(data, buffer, sizeof(node));
}



