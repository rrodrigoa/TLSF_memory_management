#include "TLSF.h"
#include <stdlib.h>
#include <stdio.h>
#include <intrin.h>
#include <cmath>

// Initialize memory with size in bytes
bool TLSF_initMemory(unsigned int powerOfTwoBytes){
  TLSF_initialized = true;
	size_t sizeInBytes = (size_t)pow((long)2, (long)powerOfTwoBytes);
	if(sizeInBytes > 0){
		memory = (unsigned char*)malloc(sizeof(unsigned char)*sizeInBytes);

		// Initialization
		unsigned int fl;
		unsigned int sl;
		struct block* firstBlock = (struct block*)memory;

		firstBlock->next = NULL;
		firstBlock->prev = NULL;
		firstBlock->size = sizeInBytes - sizeof(struct block);
		TLSF_mapping_insert(firstBlock->size, &fl, &sl);
		firstBlock->physical_next = NULL;
		firstBlock->physical_prev = NULL;
		firstBlock->bitMask = IS_FREE_BITMASK;

		TLSF_insert_block(firstBlock, fl, sl);

		if(memory == NULL){
			TLSF_initialized = false;
		}
	}else{
		TLSF_initialized = false;
	}
	return TLSF_initialized;
}

// Find the location fl, sl in the array
void TLSF_mapping_search(unsigned int *bytes, unsigned int *fl, unsigned int *sl){
	unsigned long indexLeft;
	_BitScanReverse(&indexLeft, (unsigned long)(*bytes));

	(*bytes) = (*bytes) + (1 << (indexLeft - TLSF_j)) - 1;
	_BitScanReverse(&indexLeft, (unsigned long)(*bytes));
	(*fl) = (unsigned int)indexLeft;
	(*sl) = ((*bytes) >> ((*fl) - TLSF_j)) - TLSF_2_power_j;
}

void *TLSF_find_suitable_block(unsigned int *fl, unsigned int *sl){
	unsigned int bitmap_temp = TLSF_SL_bitmap[(*fl)] & (~0 << (*sl));
	unsigned int non_empty_sl;
	unsigned int non_empty_fl;
	unsigned int index;
	unsigned long longNumber;
	if(bitmap_temp != 0){
		_BitScanForward(&longNumber, (unsigned long)bitmap_temp);
		non_empty_sl = (unsigned int)longNumber;
		non_empty_fl = (*fl);
	}else{
		bitmap_temp = TLSF_FL_bitmap & (~0 << ((*fl) + 1));
		_BitScanForward(&longNumber, (unsigned long)bitmap_temp);
		non_empty_fl = (unsigned int)longNumber;
		_BitScanForward(&longNumber, (unsigned long)TLSF_SL_bitmap[non_empty_fl]);
		non_empty_sl = (unsigned int)longNumber;
	}
	void* address = addressMatrix[non_empty_fl][non_empty_sl];
	(*fl) = non_empty_fl;
	(*sl) = non_empty_sl;

	return address;
}

void TLSF_mapping_insert(unsigned int bytes, unsigned int *fl, unsigned int *sl){
	unsigned long longNumber;
	_BitScanReverse(&longNumber, (unsigned long)bytes);
	(*fl) = (unsigned int)longNumber;
	(*sl) = (bytes >> ((*fl) - TLSF_j)) - TLSF_2_power_j;
}

void TLSF_remove_head(unsigned int fl, unsigned int sl){
	struct block* head = addressMatrix[fl][sl];
	struct block* headNext = head->next;
	if(head->next != NULL){
		head->next->prev = head->prev;
	}
	if(head->prev != NULL){
		head->prev->next = head->next;
	}
	head->next = NULL;
	head->prev = NULL;
	head->bitMask &= ~(IS_FREE_BITMASK);
	addressMatrix[fl][sl] = headNext;

	if(headNext == NULL){
		bool isLast = true;
		TLSF_SL_bitmap[fl] &= ~(1 << sl);
		for(int indexSL = 0; indexSL < TLSF_2_power_j; indexSL++){
			if(addressMatrix[fl][indexSL] != NULL){
				isLast = false;
				break;
			}
		}
		if(isLast){
			TLSF_FL_bitmap &= ~(1 << fl);
		}
	}
}

void TLSF_insert_block(struct block* block, unsigned int fl, unsigned int sl){
	block->next = addressMatrix[fl][sl];
	addressMatrix[fl][sl] = block;
	if(block->next != NULL){
		block->next->prev = block;
	}
	block->bitMask |= IS_FREE_BITMASK;

	TLSF_SL_bitmap[fl] |= 1 << sl;
	TLSF_FL_bitmap |= 1 << fl;
}

void* TLSF_split(struct block* block, unsigned int bytes){
	char* pointerToNewAddress = (char*)block;
	pointerToNewAddress += sizeof(struct block) + bytes;

	struct block *newBlock = (struct block *)pointerToNewAddress;

	newBlock->next = NULL;
	newBlock->prev = NULL;
	newBlock->size = block->size - sizeof(struct block) - bytes;
	newBlock->physical_next = block->physical_next;
	newBlock->physical_prev = block;
	block->physical_next = newBlock;
	newBlock->bitMask = IS_FREE_BITMASK;

	block->size = bytes;

	return (void*)newBlock;
}

void *TLSF_malloc(unsigned int size){
	if(TLSF_initialized == false){
		TLSF_initMemory(TLSF_initializeSize);
	}
	unsigned int fl;
	unsigned int sl;
	unsigned int bytes = size;
	TLSF_mapping_search(&bytes, &fl, &sl);
	void* free_block_add = TLSF_find_suitable_block(&fl, &sl);
	struct block *free_block = (struct block*)free_block_add;
	if(free_block_add == NULL){
		return NULL;
	}else{
		TLSF_remove_head(fl, sl);
		if(free_block->size - bytes > TLSF_split_threshold){
			struct block* remaining_block;
			remaining_block = (struct block*)TLSF_split(free_block, bytes);
			TLSF_mapping_insert(remaining_block->size, &fl, &sl);
			TLSF_insert_block(remaining_block, fl, sl);
		}
		char * pointer = (char*)free_block_add;
		pointer += sizeof(struct block);
		free_block_add = (void*)pointer;
		return free_block_add;
	}
}

void TLSF_remove_block(struct block* block, unsigned int fl, unsigned int sl){
	struct block* blockNext = block->next;
	struct block* blockPrev = block->prev;
	if(addressMatrix[fl][sl] == block){
		addressMatrix[fl][sl] = blockNext;
	}
	if(blockNext != NULL){
		blockNext->prev = blockPrev;
	}
	if(blockPrev != NULL){
		blockPrev->next = blockNext;
	}
}

void TLSF_merge(struct block* blockPrev, struct block* block){
	blockPrev->size = blockPrev->size + sizeof(struct block) + block->size;
	blockPrev->physical_next = block->physical_next;
}

struct block* TLSF_merge_prev(struct block* block){
	struct block* prevBlock = block->physical_prev;
	if(prevBlock != NULL && (prevBlock->bitMask & IS_FREE_BITMASK)){
		unsigned int fl;
		unsigned int sl;
		TLSF_mapping_insert(prevBlock->size, &fl, &sl);
		TLSF_remove_block(prevBlock, fl, sl);
		TLSF_merge(prevBlock, block);
	}else{
		prevBlock = block;
	}
	return prevBlock;
}

struct block* TLSF_merge_next(struct block* block){
	struct block* nextBlock = block->physical_next;
	if(nextBlock != NULL && (nextBlock->bitMask & IS_FREE_BITMASK )){
		unsigned int fl;
		unsigned int sl;
		TLSF_mapping_insert(nextBlock->size, &fl, &sl);
		TLSF_remove_block(nextBlock, fl, sl);
		TLSF_merge(block, nextBlock);
	}
	return block;
}

void TLSF_free(void* address){
	char* pointer = (char*)address;
	pointer -= sizeof(struct block);
	struct block* block = (struct block*)pointer;
	struct block* merged_block = TLSF_merge_prev(block);
	merged_block = TLSF_merge_next(merged_block);
	unsigned int fl;
	unsigned int sl;
	TLSF_mapping_insert(merged_block->size, &fl, &sl);
	TLSF_insert_block(merged_block, fl, sl);
}
