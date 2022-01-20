#include <stdbool.h>
#include <stdlib.h>

//-------------------- free list data structure --------------------//

struct _metadata {
  size_t dataSize;
  bool occupied;
  struct _metadata* prev;
  struct _metadata* next;
};
typedef struct _metadata Metadata;

struct _linkedList {
  Metadata* head;
  Metadata* tail;
};
typedef struct _linkedList LinkedList;

//-------------------- metadata methods --------------------//

/**
 * @brief initiate a metadata pointer with provided values
 * 
 * @param block the metadata pointer to be initialized
 * @param dataSize the size of avaiable space
 * @param occupied true if allocated, false otherwise
 */
void initMetadata(Metadata* block, size_t dataSize, bool occupied);

/**
 * @brief a testing method to display a metadata
 * 
 * @param block the metadata pointer to be displayed
 */
void printMetadata(Metadata* block);

//-------------------- linkedlist methods --------------------//

/**
 * @brief testing method to display a linked list
 * 
 * @param list the linked list to be displayed
 */
void printFree(LinkedList* list);

/**
 * @brief method to append a free list
 * 
 * @param list the linked list to be appended
 * @param block metadata pointer to be appended to the list
 */
void appendFree(LinkedList* list, Metadata* block);

/**
 * @brief method to insert a metadata to a free list
 * 
 * @param list the linked list to be inserted
 * @param block metadata pointer to be inserted to the list
 * @param prevBlock the previous element in the list behind which to be inserted, 
 *  NULL if inserting before the first element
 */
void insertFree(LinkedList* list, Metadata* block, Metadata* prevBlock);

/**
 * @brief method to delete a metadata from a free list
 * 
 * @param list the linked list
 * @param block metadata pointer to be deleted from the list
 */
void deleteFree(LinkedList* list, Metadata* block);

//-------------------- malloc/free auxillary methods --------------------//

/**
 * @brief create a new segment in the heap
 * 
 * @param dataSize the size of the new segment
 * @return void* the pointer to the newly created segment
 */
void* allocate(size_t dataSize);

/**
 * @brief split a segment into two, 
 *  the first one with the requested size, removed from the free list and returned
 *  the second one to be inserted to the free list
 * 
 * @param block the pointer to the segment to be splitted
 * @param dataSize the data size to be splitted off from the segment
 * @return Metadata* the pointer to the segment with requested data size
 */
Metadata* split(Metadata* block, size_t dataSize);

/**
 * @brief merge a block with its left neighbor
 * 
 * @param block the block to be merged with its left neighbor
 * 
 * @note when the block is merged,
 *  it is removed from the free list,
 *  its left neighbor now contains its size
 */
void coalesceLeft(Metadata* block);

/**
 * @brief merge a block with its right neighbor
 * 
 * @param block the block to be merged with its right neighbor
 * 
 * @note when the block is merged,
 *  its right neighbor is removed from the free list
 *  the block now contains its right neighbor's size
 */
void coalesceRight(Metadata* block);

/**
 * @brief free a segment, insert it to the free list,
 *  and merge with its neighbors if possible
 * 
 * @param block the segment to be freed
 */
void freeBlock(Metadata* block);

//-------------------- malloc/free API --------------------//

/**
 * @brief custom malloc API, using the first fit policy
 * 
 * @param size size to be allocated
 * @return void* pointer to the allocated space,
 *  NULL if no more space can be allocated
 */
void* ff_malloc(size_t size);

/**
 * @brief custom free API, using the first fit policy
 * 
 * @param ptr pointer to the space to be freed
 */
void ff_free(void *ptr);

/**
 * @brief custom malloc API, using the best fit policy
 * 
 * @param size size to be allocated
 * @return void* pointer to the allocated space,
 *  NULL if no more space can be allocated* 
 */
void *bf_malloc(size_t size);

/**
 * @brief custom free API, using the best fit policy
 * 
 * @param ptr pointer to the space to be freed
 */
void bf_free(void *ptr);

//-------------------- performance metrics --------------------//

/**
 * @brief Get the data segment size
 * 
 * @return unsigned long 
 */
unsigned long get_data_segment_size();

/**
 * @brief Get the freed data segment size
 * 
 * @return unsigned long 
 */
unsigned long get_data_segment_free_space_size();