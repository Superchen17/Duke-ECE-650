#include <stdbool.h>
#include <stdlib.h>

//-------------------- free list data structure --------------------//
struct _metadata {
  size_t dataSize;
  bool occupied;
  struct _metadata* prev;
  struct _metadata* next;
  struct _metadata* left;
  struct _metadata* right;
};
typedef struct _metadata Metadata;

struct _linkedList {
  Metadata* head;
  Metadata* tail;
};
typedef struct _linkedList LinkedList;

//-------------------- metadata methods --------------------//

void initMetadata(Metadata* block, size_t dataSize, bool occupied);
void printMetadata(Metadata* block);

//-------------------- linkedlist methods --------------------//

void printFree(LinkedList* list);
void printPhysical(LinkedList* list);
void appendFree(LinkedList* list, Metadata* block);
void appendPhysical(LinkedList* list, Metadata* block);
void insertFree(LinkedList* list, Metadata* block, Metadata* prevBlock);
void insertPhysical(LinkedList* list, Metadata* block, Metadata* leftBlock);
void deleteFree(LinkedList* list, Metadata* block);
void deletePhysical(LinkedList* list, Metadata* block);

//-------------------- malloc/free auxillary methods --------------------//

void* allocate(size_t dataSize);
Metadata* split(Metadata* block, size_t dataSize);
Metadata* coalesceLeft(Metadata* block);
Metadata* coalesceRight(Metadata* block);
Metadata* freeBlock(Metadata* block);

//-------------------- malloc/free API --------------------//

void* ff_malloc(size_t size);
void ff_free(void *ptr);
void *bf_malloc(size_t size);
void bf_free(void *ptr);

//-------------------- performance metrics --------------------//
unsigned long get_data_segment_size();
unsigned long get_data_segment_free_space_size();
