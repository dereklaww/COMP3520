#include <memory.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h> 

/*
 * 8 Mbytes minimum allocation size
 */
#define MIN_ALLOC_LOG2 3
#define MIN_ALLOC ((int)1 << MIN_ALLOC_LOG2)

/*
 * 2048 Mbytes maximum allocation size
 */
#define MAX_ALLOC_LOG2 11 
#define MAX_ALLOC ((int)1 << MAX_ALLOC_LOG2)

/*
 * Allocations are done in powers of 2 inclusive of min and max alloc. Each allocation size
 * has a container that stores the free blocks for that allocation size. Given a container index,
 * the size of the allocations in the container can be found with 1 << (MAX_ALLOCLOG2 - container_index)
 */ 
#define BLOCK_COUNT (MAX_ALLOC_LOG2 - MIN_ALLOC_LOG2 + 1) 

#define DEBUG_PRINT 0

/**
 * Each size allocation has an associated doubly linked-list which links all the free blocks
 * of that size. 
*/

typedef struct mab {
    int offset_address;
    int size;
    struct mab *next; // memory blocks with the same size
    struct mab *prev;
    struct mab *parent_block;
    struct mab *left_child_block;
    struct mab *right_child_block;
    bool allocated;
    bool remove;
    int level;
} Mab;

typedef Mab* MabPtr;

MabPtr init_mem_block (int size, int offset_address);
void print_tree();
MabPtr init_mem_system();
void search_remove(int level);
MabPtr mem_split(MabPtr mem_block, int mem_request);
MabPtr mem_alloc(MabPtr root_node, int mem_request);
MabPtr mem_merge (MabPtr mem_block);
MabPtr mem_free (MabPtr mem_block);
int norm_mem_request(int mem_request);