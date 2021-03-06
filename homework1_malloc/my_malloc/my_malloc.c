#include "my_malloc.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>

LinkedList listFreed = { .head=NULL, .tail=NULL };
size_t sizeAllocated = 0;
size_t sizeFreed = 0;

//-------------------- metadata methods --------------------//

void initMetadata(Metadata* block, size_t dataSize, bool occupied){
  block->dataSize = dataSize;
  block->occupied = occupied;
  block->prev = NULL;
  block->next = NULL;
}

void printMetadata(Metadata* block){
  printf(
    "curr: %p, size: %lu, prev: %p, next: %p, allocated: %d\n", 
    block, block->dataSize, block->prev, block->next, block->occupied
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

void insertFree(LinkedList* list, Metadata* block, Metadata* prevBlock){
  if(list->head == NULL && list->tail == NULL){
    list->head = block;
    list->tail = block;
  }
  else if(prevBlock == NULL){
    list->head->prev = block;
    block->next = list->head;
    block->prev = NULL;
    list->head = block;
  }
  else if(prevBlock == list->tail){
    list->tail->next = block;
    block->prev = list->tail;
    block->next = NULL;
    list->tail = block;
  }
  else{
    Metadata* nextBlock = prevBlock->next;
    prevBlock->next = block;
    block->prev = prevBlock;
    block->next = nextBlock;
    nextBlock->prev = block;
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

void* allocate(size_t dataSize){
  size_t newTotalSize = dataSize + sizeof(Metadata);
  Metadata* newBlock = sbrk(newTotalSize);

  if((void*)newBlock == (void*)(-1)){
    return NULL;
  }

  initMetadata(newBlock, dataSize, true);
  sizeAllocated += newTotalSize;
  return (char*)newBlock + sizeof(Metadata);

}

Metadata* split(Metadata* block, size_t dataSize){
  if(dataSize + sizeof(Metadata) > block->dataSize){
    sizeFreed -= (sizeof(Metadata) + block->dataSize);
    deleteFree(&listFreed, block);
  }
  else{
    size_t dataSizeBeforeSplit = block->dataSize;
    block->dataSize = dataSize;
    Metadata* remainingBlock = (Metadata*)(
      (char*)block + sizeof(Metadata) + dataSize
    );
    size_t remainingSize = dataSizeBeforeSplit - dataSize - sizeof(Metadata);
    initMetadata(remainingBlock, remainingSize, false);

    block->occupied = true;
    insertFree(&listFreed, remainingBlock, block);
    deleteFree(&listFreed, block);

    sizeFreed -= (sizeof(Metadata) + dataSize);
  }
  return block;
}

void coalesceLeft(Metadata* block){
  if(block->prev != NULL && (char*)block == (char*)block->prev + sizeof(Metadata) + block->prev->dataSize){
    Metadata* prevBlock = block->prev;
    prevBlock->dataSize += sizeof(Metadata) + block->dataSize;
    deleteFree(&listFreed, block);
  }
}

void coalesceRight(Metadata* block){
  if(block->next != NULL && (char*)block->next == (char*)block + sizeof(Metadata) + block->dataSize){
    Metadata* nextBlock = block->next;
    block->dataSize += sizeof(Metadata) + nextBlock->dataSize;
    deleteFree(&listFreed, nextBlock);
  }
}

void freeBlock(Metadata* block){
  sizeFreed += block->dataSize + sizeof(Metadata);

  block->occupied = false;

  if(listFreed.head == NULL && listFreed.tail == NULL){
    appendFree(&listFreed, block);
  }
  else if((char*)block < (char*)listFreed.head){
    insertFree(&listFreed, block, NULL);
    coalesceRight(block);
  }
  else if((char*)block > (char*)listFreed.tail){
    appendFree(&listFreed, block);
    coalesceLeft(block);
  }
  else{
    Metadata* curr = listFreed.head;
    while((char*)curr < (char*)block){
      curr = curr->next;
    }
    Metadata* prevBlock = curr->prev;
    insertFree(&listFreed, block, prevBlock);
    coalesceRight(block);
    coalesceLeft(block);
  }
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
  return allocate(size);
}

void ff_free(void *ptr){
  if(ptr == NULL){
    return;
  }
  Metadata* blockFreed;
  blockFreed = (Metadata*)((char*)ptr - sizeof(Metadata));
  if(blockFreed->occupied == true){
    freeBlock(blockFreed);
  }
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
    return allocate(size);
  }
  else{
    return (char*)split(bestFit, size) + sizeof(Metadata);
  }
}

void bf_free(void *ptr){
  ff_free(ptr);
}

//-------------------- performance metrics --------------------//

unsigned long get_data_segment_size(){
  return sizeAllocated;
}

unsigned long get_data_segment_free_space_size(){
  return sizeFreed;
}
