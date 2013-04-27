#include <stdio.h>
#include <cstring>
#include <functional>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <queue>
#include <algorithm>
#include <vector>
#include <stack>
#include <map>
#include <cmath>
#include <string>
#include <intrin.h>
using namespace std;

struct block{
  struct block* next;
	struct block* prev;
	unsigned int size;
	struct block* physical_next;
	struct block* physical_prev;
	bool isFree;
};

static bool TLSF_initialized = false;
// Initialize memory with 32MB
static const unsigned int TLSF_initializeSize = 25;
static const unsigned int TLSF_j = 3;
static const unsigned int TLSF_2_power_j = 8;
static const unsigned int TLSF_split_threshold = (1024*10);

static unsigned char *memory;
static unsigned int TLSF_SL_bitmap[32];
static unsigned int TLSF_FL_bitmap;
static struct block* addressMatrix[32][TLSF_2_power_j];

void TLSF_mapping_insert(unsigned int bytes, unsigned int *fl, unsigned int *sl);
void TLSF_insert_block(struct block* block, unsigned int fl, unsigned int sl);

// Initialize memory with size in bytes
bool TLSF_initMemory(unsigned int powerOfTwoBytes){
	TLSF_initialized = true;
	size_t sizeInBytes = (size_t)pow((long)2, (long)powerOfTwoBytes);
	if(sizeInBytes > 0){
		memory = (unsigned char*)malloc(sizeof(unsigned char)*sizeInBytes);
		
		//Test 
		/*
		TLSF_FL_bitmap = 65632;
		TLSF_SL_bitmap[5] |= (1 << 1);
		TLSF_SL_bitmap[6] |= (1 << 5);
		TLSF_SL_bitmap[16] |= (1 << 5);

		struct block *blockOn5 = (struct block*)malloc(sizeof(struct block));
		struct block *blockOn6 = (struct block*)malloc(sizeof(char)*800);
		struct block *blockOn16 = (struct block*)malloc(sizeof(struct block));

		blockOn5->next = NULL;
		blockOn5->size = 36;

		blockOn6->next = NULL;
		blockOn6->size = 504;

		blockOn16->next = NULL;
		blockOn16->size = 65536;

		addressMatrix[5][1] = blockOn5;
		addressMatrix[6][5] = blockOn6;
		addressMatrix[16][5] = blockOn16;
		*/

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
		firstBlock->isFree = true;

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
	head->isFree = false;
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
	block->isFree = true;

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
	newBlock->isFree = true;

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
	if(prevBlock != NULL && prevBlock->isFree == true){
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
	if(nextBlock != NULL && nextBlock->isFree == true){
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

struct Test2Struct{
	int intValue;
	float floatValue;
	char charStr[9];
	struct Test2Struct *next;
};

int main(){
	printf("Test 1\nWill allocate memory for 9 chars\n");

	unsigned int bytes = 9;

	char* myCharArray = (char*)TLSF_malloc(bytes);
	myCharArray[0] = 'D';
	myCharArray[1] = 'E';
	myCharArray[2] = 'A';
	myCharArray[3] = 'D';
	myCharArray[4] = 'B';
	myCharArray[5] = 'E';
	myCharArray[6] = 'E';
	myCharArray[7] = 'F';
	myCharArray[8] = '\0';

	printf("Allocated char value is [%s]\n", myCharArray);

	printf("Free allocated memory\n");
	TLSF_free(myCharArray);

	printf("\nTest 2\nWill allocate memory for 2 structure objs\n");
	struct Test2Struct* head = (struct Test2Struct*)TLSF_malloc(sizeof(struct Test2Struct));
	struct Test2Struct* next = (struct Test2Struct*)TLSF_malloc(sizeof(struct Test2Struct));

	head->charStr[0] = 'D';
	head->charStr[1] = 'E';
	head->charStr[2] = 'A';
	head->charStr[3] = 'D';
	head->charStr[4] = 'B';
	head->charStr[5] = 'E';
	head->charStr[6] = 'E';
	head->charStr[7] = 'F';
	head->charStr[8] = '\0';

	head->intValue = 8;
	head->floatValue = 5.3f;

	next->charStr[0] = 'B';
	next->charStr[1] = 'E';
	next->charStr[2] = 'E';
	next->charStr[3] = 'F';
	next->charStr[4] = 'D';
	next->charStr[5] = 'E';
	next->charStr[6] = 'A';
	next->charStr[7] = 'D';
	next->charStr[8] = '\0';

	next->intValue = 16;
	next->floatValue = 2.0f;

	head->next = next;
	next->next = NULL;
	struct Test2Struct *pt = head;

	while(pt != NULL){
		printf("charStr value is [%s]\n", pt->charStr);
		printf("intValue value is [%d]\n", pt->intValue);
		printf("floatVlaue value is [%f]\n", pt->floatValue);
		pt = pt->next;
		if(pt != NULL){
			printf("going to next struct\n");
		}
	}

	printf("free two structs\n");

	TLSF_free(head);
	TLSF_free(next);

	printf("DONE\n");
}
