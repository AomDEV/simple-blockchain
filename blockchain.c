#include <stdio.h>
#include <stdlib.h>
#include <openssl/sha.h>
#include <time.h>
#include <string.h>
#include "blockchain.h"


void hash256(unsigned char* output, const char* input){

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

char hash_transaction(char* output , transaction* trans_list, unsigned int trans_list_length) {
    
    if (output == NULL || trans_list == NULL ) return 0;
    
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
        
        char hex_hash[HASH_HEX_SIZE] = {0};
        char buffer[3];
        
        for(int i =0; i < HASH_SIZE; i++) {
        	memset(buffer, 0, sizeof(buffer));
        	sprintf(buffer, "%02x", hashvalue[i]);
        	strcat(hex_hash, buffer);
        }
        
        printf("HEXHASH OF TRANSACTIONS: '%s'\n", hex_hash);
        
        strcpy(output, hex_hash);
        
        return 1;
        
}


void serializeNode(node* data, unsigned char* buffer) {
    memcpy(buffer, data, sizeof(node));
}
void deserializeNode(node* data, unsigned char* buffer) {
    memcpy(data, buffer, sizeof(node));
}
