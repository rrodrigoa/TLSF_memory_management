#ifndef TLSF_HEADER

#define TLSF_HEADER

struct block{
  struct block* next;
	struct block* prev;
	unsigned int size;
	struct block* physical_next;
	struct block* physical_prev;
	char bitMask;
};

#define IS_FREE_BITMASK 0x1

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

bool TLSF_initMemory(unsigned int powerOfTwoBytes);
void TLSF_mapping_search(unsigned int *bytes, unsigned int *fl, unsigned int *sl);
void *TLSF_find_suitable_block(unsigned int *fl, unsigned int *sl);
void TLSF_mapping_insert(unsigned int bytes, unsigned int *fl, unsigned int *sl);
void TLSF_remove_head(unsigned int fl, unsigned int sl);
void TLSF_insert_block(struct block* block, unsigned int fl, unsigned int sl);
void* TLSF_split(struct block* block, unsigned int bytes);
void *TLSF_malloc(unsigned int size);
void TLSF_remove_block(struct block* block, unsigned int fl, unsigned int sl);
void TLSF_merge(struct block* blockPrev, struct block* block);
struct block* TLSF_merge_prev(struct block* block);
struct block* TLSF_merge_next(struct block* block);
void TLSF_free(void* address);


#endif
