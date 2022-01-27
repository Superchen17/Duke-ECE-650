#ifndef __MY_MALLOC_H__
#define __MY_MALLOC_H__

#include <stdbool.h>
#include <stdlib.h>
#include <pthread.h>

//-------------------- pthread mechanisms --------------------//
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

//-------------------- function pointers --------------------//

typedef void*(*sbrkFp)(intptr_t);

//-------------------- free list data structure --------------------//

struct _metadata {
  size_t dataSize;
  bool occupied;
  struct _metadata* prev;
  struct _metadata* next;
};
typedef struct _metadata Metadata;

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
void printFree(Metadata* head, Metadata* tail);

/**
 * @brief method to append a free list
 * 
 * @param head list head
 * @param tail list tail
 * @param block metadata pointer to be appended to the list
 */
void appendFree(Metadata* head, Metadata* tail, Metadata* block);

/**
 * @brief method to insert a metadata to a free list
 * 
 * @param head list head
 * @param tail list tail
 * @param block metadata pointer to be inserted to the list
 * @param prevBlock the previous element in the list behind which to be inserted, 
 *  NULL if inserting before the first element
 */
void insertFree(Metadata* head, Metadata* tail, Metadata* block, Metadata* prevBlock);

/**
 * @brief method to delete a metadata from a free list
 * 
 * @param head list head
 * @param tail list tail
 * @param block metadata pointer to be deleted from the list
 */
void deleteFree(Metadata* head, Metadata* tail, Metadata* block);

//-------------------- malloc/free auxillary methods --------------------//

/**
 * @brief tls version of sbrk
 * 
 * @param datasize size of requested data
 * @return void* pointer to allocated segment
 */
void* sbrk_tls(size_t datasize);

/**
 * @brief create a new segment in the heap
 * 
 * @param dataSize the size of the new segment
 * @param fp sbrk function pointer
 * @return void* the pointer to the newly created segment
 */
void* alloc(size_t dataSize, sbrkFp fp);

/**
 * @brief split a segment into two, 
 *  the first one with the requested size, removed from the free list and returned
 *  the second one to be inserted to the free list
 * 
 * @param head list head
 * @param tail list tail
 * @param dataSize the data size to be splitted off from the segment
 * @return Metadata* the pointer to the segment with requested data size
 */
Metadata* split(Metadata* head, Metadata* tail, Metadata* block, size_t dataSize);

/**
 * @brief merge a block with its left neighbor
 * 
 * @param head list head
 * @param tail list tail
 * @param block the block to be merged with its left neighbor
 * 
 * @note when the block is merged,
 *  it is removed from the free list,
 *  its left neighbor now contains its size
 */
void coalesceLeft(Metadata* head, Metadata* tail, Metadata* block);

/**
 * @brief merge a block with its right neighbor
 * 
 * @param head list head
 * @param tail list tail
 * @param block the block to be merged with its right neighbor
 * 
 * @note when the block is merged,
 *  its right neighbor is removed from the free list
 *  the block now contains its right neighbor's size
 */
void coalesceRight(Metadata* head, Metadata* tail, Metadata* block);

/**
 * @brief free a segment, insert it to the free list,
 *  and merge with its neighbors if possible
 * 
 * @param head list head
 * @param tail list tail
 * @param block the segment to be freed
 */
void freeBlock(Metadata* head, Metadata* tail, Metadata* block);

//-------------------- malloc/free API --------------------//

/**
 * @brief custom malloc API, using the first fit policy
 * not used in this homework
 * 
 * @param head list head
 * @param tail list tail
 * @param fp sbrk function pointer
 * @param size size to be allocated
 * @return void* pointer to the allocated space,
 *  NULL if no more space can be allocated
 */
void* ff_malloc(Metadata* head, Metadata* tail, sbrkFp fp, size_t size);

/**
 * @brief custom free API, using the first fit policy
 * 
 * @param head list head
 * @param tail list tail
 * @param ptr pointer to the space to be freed
 */
void ff_free(Metadata* head, Metadata* tail, void *ptr);

/**
 * @brief custom malloc API, using the best fit policy
 * 
 * @param head list head
 * @param tail list tail
 * @param fp sbrk function pointer
 * @param size size to be allocated
 * @return void* pointer to the allocated space,
 *  NULL if no more space can be allocated* 
 */
void *bf_malloc(Metadata* head, Metadata* tail, sbrkFp fp,size_t size);

/**
 * @brief custom free API, using the best fit policy
 * 
 * @param head list head
 * @param tail list tail
 * @param ptr pointer to the space to be freed
 */
void bf_free(Metadata* head, Metadata* tail, void *ptr);

//-------------------- threaded malloc/free API --------------------//

/**
 * @brief custom mutithreaded malloc API with locks
 * implements best fit allocation policy
 * 
 * @param size size to be allocated
 * @return void* pointer to allocated segment
 */
void *ts_malloc_lock(size_t size);

/**
 * @brief custom free API with locks
 * 
 * @param ptr pointer to segment to be freed
 */
void ts_free_lock(void *ptr);

/**
 * @brief custom mutithreaded malloc API without locks
 * implements best fit allocation policy
 * 
 * @param size size to be allocated
 * @return void* pointer to allocated segment
 */
void *ts_malloc_nolock(size_t size);

/**
 * @brief custom free API without licks
 * 
 * @param ptr pointer to segment to be freed
 */
void ts_free_nolock(void *ptr);

#endif
