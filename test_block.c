#include "blockchain.c"
#include <stdio.h>
#include <string.h>


int main() {
	
    transaction txns[TRANS_LIST_SIZE];
	
    transaction txn1;
    txn1.timestamp = 10;
    strcpy(txn1.sender, "A");
    strcpy(txn1.recipient, "B");
    txn1.amount = 20;

    block* bp = malloc(sizeof(block));
    bp->index = 1;
    bp->timestamp = 0;
    bp->prev = NULL;
    bp->nonce = 0;
    block* bs = malloc(sizeof(block));
    bs->index = 0;
    bs->timestamp = 5;
    memset(bs->trans_list, 0, sizeof(bs->trans_list));
    bs->trans_list_length = TRANS_LIST_SIZE;
    bs->prev = bp;
    bs->trans_list[0] = txn1;
    bs->trans_list[1] = txn1;
    bs->trans_list[2] = txn1;
    bs->trans_list[3] = txn1;
    bs->trans_list_length = 4;

    char* hash = hash_transaction(bs->trans_list, bs->trans_list_length);
    char* sender = malloc(PUBLIC_ADDRESS_SIZE);
    for(int i = 0; i< 10;i++){
    	char* block_hash = hash_block(bs,sender,i);
    	free(block_hash);
    }
    free(hash);
    free(sender);
    free(bp);
    free(bs);

    return 0;
}




