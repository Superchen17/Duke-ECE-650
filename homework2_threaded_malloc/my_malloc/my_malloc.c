#include "my_malloc.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include "assert.h"

Metadata* head = NULL;
Metadata* tail = NULL;
__thread Metadata* tlsHead = NULL;
__thread Metadata* tlsTail = NULL;

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

void printFree(Metadata* head, Metadata* tail){
  printf("\nfree list \n");
  printf("metadata size: %lu\n", sizeof(Metadata));
  printf("head: %p\n", head);
  Metadata* curr = head;
  while(curr != NULL){
    printMetadata(curr);
    curr = curr->next;
  }
  printf("tail: %p\n", tail);
  printf("\n");
}

void appendFree(Metadata* head, Metadata* tail, Metadata* block){
  if(head == NULL && tail == NULL){
    head = block;
    tail = block;
  }
  else{
    tail->next = block;
    block->prev = tail;
    block->next = NULL;
    tail = block;
  }
  block->occupied = false;
}

void insertFree(Metadata* head, Metadata* tail, Metadata* block, Metadata* prevBlock){
  if(head == NULL && tail == NULL){
    head = block;
    tail = block;
  }
  else if(prevBlock == NULL){
    head->prev = block;
    block->next = head;
    block->prev = NULL;
    head = block;
  }
  else if(prevBlock == tail){
    tail->next = block;
    block->prev = tail;
    block->next = NULL;
    tail = block;
  }
  else{
    Metadata* nextBlock = prevBlock->next;
    prevBlock->next = block;
    block->prev = prevBlock;
    block->next = nextBlock;
    nextBlock->prev = block;
  }
}

void deleteFree(Metadata* head, Metadata* tail, Metadata* block){
  if(block == head && block == tail){
    head = NULL;
    tail = NULL;
  }
  else if(block == head){
    head = head->next;
    head->prev = NULL;
  }
  else if(block == tail){
    tail = tail->prev;
    tail->next = NULL;
  }
  else{
    block->prev->next = block->next;
    block->next->prev = block->prev;
  }

  block->prev = NULL;
  block->next = NULL;
  block->occupied = true;
}

//-------------------- malloc/free auxillary methods --------------------//

void* sbrk_tls(size_t datasize){
  assert(pthread_mutex_lock(&lock) == 0);
  void* ptr = sbrk(datasize);
  assert(pthread_mutex_unlock(&lock) == 0);
  return ptr;
}

void* alloc(size_t dataSize, sbrkFp fp){
  size_t newTotalSize = dataSize + sizeof(Metadata);
  Metadata* newBlock = (*fp)(newTotalSize);

  if((void*)newBlock == (void*)(-1)){
    return NULL;
  }

  initMetadata(newBlock, dataSize, true);
  return (char*)newBlock + sizeof(Metadata);

}

Metadata* split(Metadata* head, Metadata* tail, Metadata* block, size_t dataSize){
  if(dataSize + sizeof(Metadata) > block->dataSize){
    deleteFree(head, tail, block);
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
    insertFree(head, tail, remainingBlock, block);
    deleteFree(head, tail, block);
  }
  return block;
}

void coalesceLeft(Metadata* head, Metadata* tail, Metadata* block){
  if(block->prev != NULL && (char*)block == (char*)block->prev + sizeof(Metadata) + block->prev->dataSize){
    Metadata* prevBlock = block->prev;
    prevBlock->dataSize += sizeof(Metadata) + block->dataSize;
    deleteFree(head, tail, block);
  }
}

void coalesceRight(Metadata* head, Metadata* tail, Metadata* block){
  if(block->next != NULL && (char*)block->next == (char*)block + sizeof(Metadata) + block->dataSize){
    Metadata* nextBlock = block->next;
    block->dataSize += sizeof(Metadata) + nextBlock->dataSize;
    deleteFree(head, tail, nextBlock);
  }
}

void freeBlock(Metadata* head, Metadata* tail, Metadata* block){
  block->occupied = false;

  if(head == NULL && tail == NULL){
    appendFree(head, tail, block);
  }
  else if((char*)block < (char*)head){
    insertFree(head, tail, block, NULL);
    coalesceRight(head, tail, block);
  }
  else if((char*)block > (char*)tail){
    appendFree(head, tail, block);
    coalesceLeft(head, tail, block);
  }
  else{
    Metadata* curr = head;
    while((char*)curr < (char*)block){
      curr = curr->next;
    }
    Metadata* prevBlock = curr->prev;
    insertFree(head, tail, block, prevBlock);
    coalesceRight(head, tail, block);
    coalesceLeft(head, tail, block);
  }
}

//-------------------- malloc/free API --------------------//

void* ff_malloc(Metadata* head, Metadata* tail, sbrkFp fp, size_t size){
  Metadata* curr = head;
  while(curr != NULL){
    if(curr->occupied == false && curr->dataSize >= size){
      return (char*)split(head, tail, curr, size) + sizeof(Metadata);
    }
    curr = curr->next;
  }
  return alloc(size, (*fp));
}

void ff_free(Metadata* head, Metadata* tail, void *ptr){
  if(ptr == NULL){
    return;
  }
  Metadata* blockFreed;
  blockFreed = (Metadata*)((char*)ptr - sizeof(Metadata));
  if(blockFreed->occupied == true){
    freeBlock(head, tail, blockFreed);
  }
}

void *bf_malloc(Metadata* head, Metadata* tail, sbrkFp fp,size_t size){
  Metadata* curr = head;
  Metadata* bestFit = NULL;
  size_t minSize = SIZE_MAX;

  while(curr != NULL){
    if(curr->occupied == false && curr->dataSize >= size){
      if(curr->dataSize == size){
        return (char*)split(head, tail, curr, size) + sizeof(Metadata);
      }
      if(curr->dataSize < minSize){
        bestFit = curr;
        minSize = curr->dataSize;
      }
    }
    curr = curr->next;
  }
  if(bestFit == NULL){
    return alloc(size, (*fp));
  }
  else{
    return (char*)split(head, tail, bestFit, size) + sizeof(Metadata);
  }
}

void bf_free(Metadata* head, Metadata* tail, void *ptr){
  ff_free(head, tail, ptr);
}

//-------------------- threaded malloc/free API --------------------//

void *ts_malloc_lock(size_t size){
  assert(pthread_mutex_lock(&lock) == 0);
  void* ptr = bf_malloc(head, tail, sbrk, size);
  assert(pthread_mutex_unlock(&lock) == 0);
  return ptr;
}

void ts_free_lock(void *ptr){
  assert(pthread_mutex_lock(&lock) == 0);
  bf_free(head, tail, ptr);
  assert(pthread_mutex_unlock(&lock) == 0);
}

void *ts_malloc_nolock(size_t size){
  void* ptr = bf_malloc(tlsHead, tlsTail, sbrk_tls, size);
  return ptr;
}

void ts_free_nolock(void *ptr){
  bf_free(tlsHead, tlsTail, ptr);
}
