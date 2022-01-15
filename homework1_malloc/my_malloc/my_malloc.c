#include "my_malloc.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>

LinkedList listFreed = { .head=NULL, .tail=NULL };
LinkedList listPhysical = { .head=NULL, .tail=NULL };
size_t sizeAllocated = 0;
size_t sizeFreed = 0;
Metadata* lastBlock = NULL;

//-------------------- metadata methods --------------------//

void initMetadata(Metadata* block, size_t dataSize, bool occupied){
  block->dataSize = dataSize;
  block->occupied = occupied;
  block->prev = NULL;
  block->next = NULL;
  block->left = NULL;
  block->right = NULL;
}

void printMetadata(Metadata* block){
  printf(
    "curr: %p, size: %lu, prev: %p, next: %p, left: %p, right: %p, allocated: %d\n", 
    block, block->dataSize, block->prev, block->next, block->left, block->right, block->occupied
  );
}

//-------------------- linkedlist methods --------------------//

void printFree(LinkedList* list){
  printf("\nfree list \n");
  printf("metadata size: %lu\n", sizeof(Metadata));
  printf("head: %p\n", list->head);
  Metadata* curr = list->head;
  while(curr != NULL){
    printMetadata(curr);
    curr = curr->next;
  }
  printf("tail: %p\n", list->tail);
  printf("\n");
}

void printPhysical(LinkedList* list){
  printf("\nphysical list \n");
  printf("metadata size: %lu\n", sizeof(Metadata));
  printf("head: %p\n", list->head);
  Metadata* curr = list->head;
  while(curr != NULL){
    printMetadata(curr);
    curr = curr->right;
  }
  printf("tail: %p\n", list->tail);
  printf("\n");
}

void appendFree(LinkedList* list, Metadata* block){
  if(list->head == NULL && list->tail == NULL){
    list->head = block;
    list->tail = block;
  }
  else{
    list->tail->next = block;
    block->prev = list->tail;
    block->next = NULL;
    list->tail = block;
  }
  block->occupied = false;
}

void appendPhysical(LinkedList* list, Metadata* block){
  if(list->head == NULL && list->tail == NULL){
    list->head = block;
    list->tail = block;
  }
  else{
    list->tail->right = block;
    block->left = list->tail;
    block->right = NULL;
    list->tail = block;
  }
}

void insertFree(LinkedList* list, Metadata* block, Metadata* prevBlock){
  // if list empty
  if(list->head == NULL && list->tail == NULL){
    list->head = block;
    list->tail = block;
  }
  // if inserting before head
  else if(prevBlock == NULL){
    list->head->prev = block;
    block->next = list->head;
    block->prev = NULL;
    list->head = block;
  }
  // if inserting after tail
  else if(prevBlock == list->tail){
    list->tail->next = block;
    block->prev = list->tail;
    block->next = NULL;
    list->tail = block;
  }
  // if inserting in middle
  else{
    Metadata* nextBlock = prevBlock->next;
    prevBlock->next = block;
    block->prev = prevBlock;
    block->next = nextBlock;
    nextBlock->prev = block;
  }
}

void insertPhysical(LinkedList* list, Metadata* block, Metadata* leftBlock){
  // if list empty
  if(list->head == NULL && list->tail == NULL){
    list->head = block;
    list->tail = block;
  }
  // if inserting before head
  else if(leftBlock == NULL){
    list->head->left = block;
    block->right = list->head;
    block->left = NULL;
    list->head = block;
  }
  // if inserting after tail
  else if(leftBlock == list->tail){
    list->tail->right = block;
    block->left = list->tail;
    block->right = NULL;
    list->tail = block;
  }
  // if inserting in middle
  else{
    Metadata* rightBlock = leftBlock->right;
    leftBlock->right = block;
    block->left = leftBlock;
    block->right = rightBlock;
    rightBlock->left = block;
  }

}

void deleteFree(LinkedList* list, Metadata* block){
  if(block == list->head && block == list->tail){
    list->head = NULL;
    list->tail = NULL;
  }
  else if(block == list->head){
    list->head = list->head->next;
    list->head->prev = NULL;
  }
  else if(block == list->tail){
    list->tail = list->tail->prev;
    list->tail->next = NULL;
  }
  else{
    block->prev->next = block->next;
    block->next->prev = block->prev;
  }

  block->prev = NULL;
  block->next = NULL;
  block->occupied = true;
}

void deletePhysical(LinkedList* list, Metadata* block){
  if(block == list->head && block == list->tail){
    list->head = NULL;
    list->tail = NULL;
  }
  else if(block == list->head){
    list->head = list->head->right;
    list->head->left = NULL;
  }
  else if(block == list->tail){
    list->tail = list->tail->left;
    list->tail->right = NULL;
  }
  else{
    block->left->right = block->right;
    block->right->left = block->left;
  }
}

//-------------------- malloc/free auxillary methods --------------------//

Metadata* allocate(size_t dataSize){
  size_t newTotalSize = dataSize + sizeof(Metadata);
  Metadata* newBlock = sbrk(newTotalSize);
  initMetadata(newBlock, dataSize, true);
  appendPhysical(&listPhysical, newBlock);

  sizeAllocated += newTotalSize;
  return newBlock;
}

Metadata* split(Metadata* block, size_t dataSize){
  if(dataSize + sizeof(Metadata) > block->dataSize){
    sizeFreed -= (sizeof(Metadata) + block->dataSize);
    deleteFree(&listFreed, block);
  }
  else{
    size_t dataSizeBeforeSplit = block->dataSize;
    block->dataSize = dataSize;
    Metadata* nextBlock = block->next;
    Metadata* remainingBlock = (Metadata*)(
      (char*)block + sizeof(Metadata) + dataSize
    );
    size_t remainingSize = dataSizeBeforeSplit - dataSize - sizeof(Metadata);
    initMetadata(remainingBlock, remainingSize, false);

    block->occupied = true;
    deleteFree(&listFreed, block);
    appendFree(&listFreed, remainingBlock);
    insertPhysical(&listPhysical, remainingBlock, block);

    sizeFreed -= (sizeof(Metadata) + dataSize);
  }
  return block;
}

Metadata* freeBlock(Metadata* block){

  sizeFreed += block->dataSize + sizeof(Metadata);

  bool signal = false;
  if(block->right != NULL && block->right->occupied == false){
    coalesceRight(block);
    signal = true;
  }
  if(block->left != NULL && block->left->occupied == false){
    coalesceLeft(block);
    signal = true;
  }

  if(signal == false){
    appendFree(&listFreed, block);
  }

  block->occupied = false;

}

Metadata* coalesceLeft(Metadata* block){
  Metadata* leftBlock = block->left;
  leftBlock->dataSize += sizeof(Metadata) + block->dataSize;

  if(block->occupied == false){
    deleteFree(&listFreed, block);
  }
  deletePhysical(&listPhysical, block);

  return leftBlock;
}

Metadata* coalesceRight(Metadata* block){
  Metadata* rightBlock = block->right;
  block->dataSize += sizeof(Metadata) + block->right->dataSize;
  appendFree(&listFreed, block);
  deleteFree(&listFreed, block->right);
  deletePhysical(&listPhysical, block->right);

  return block;
}

//-------------------- malloc/free API --------------------//

void* ff_malloc(size_t size){
  Metadata* curr = listFreed.head;
  while(curr != NULL){
    if(curr->occupied == false && curr->dataSize >= size){
      return (char*)split(curr, size) + sizeof(Metadata);
    }
    curr = curr->next;
  }
  return (char*)allocate(size) + sizeof(Metadata);
}

void ff_free(void *ptr){
  Metadata* blockFreed;
  blockFreed = (Metadata*)((char*)ptr - sizeof(Metadata));
  freeBlock(blockFreed);
}

void *bf_malloc(size_t size){
  Metadata* curr = listFreed.head;
  Metadata* bestFit = NULL;
  size_t minSize = SIZE_MAX;

  while(curr != NULL){
    if(curr->occupied == false && curr->dataSize >= size){
      if(curr->dataSize == size){
        return (char*)split(curr, size) + sizeof(Metadata);
      }
      if(curr->dataSize < minSize){
        bestFit = curr;
        minSize = curr->dataSize;
      }
    }
    curr = curr->next;
  }
  if(bestFit == NULL){
    return (char*)allocate(size) + sizeof(Metadata);
  }
  else{
    return (char*)split(bestFit, size) + sizeof(Metadata);
  }
}

void bf_free(void *ptr){
  ff_free(ptr);
}

unsigned long get_data_segment_size() {
  return sizeAllocated;
}

unsigned long get_data_segment_free_space_size() {
  return sizeFreed;
}
