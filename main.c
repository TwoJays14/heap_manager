#include <stdio.h>
#include <windows.h>

typedef struct MemoryBlock {
  size_t size;
  bool is_allocated;
  struct MemoryBlock *next;
  struct MemoryBlock *prev;
} MemoryBlock;

void *init_mem_pool();
bool is_mallocable(MemoryBlock *heap);
void *find_free_block(MemoryBlock *heap, size_t size);
void *mem_alloc(MemoryBlock *heap, size_t size);
void *mem_free(void *ptr_to_allocated_mem_block);

int main(void) {
  MemoryBlock *heap = init_mem_pool();
  void *new_mem = mem_alloc(heap, sizeof(int));

  printf("Hello World");
  return 0;
}

void *init_mem_pool() {
  constexpr size_t heap_size = 1024 * 1024;

  const LPVOID heap = VirtualAlloc(NULL, heap_size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

  MemoryBlock *mem_pool = heap;

  if (heap == NULL) {
    const DWORD error = GetLastError();
    fprintf(stderr, "Failed to allocate memory. Error Code: %lu", error);
    return NULL;
  }

  mem_pool->size = heap_size - sizeof(MemoryBlock);
  mem_pool->is_allocated = false;
  mem_pool->next = NULL;
  mem_pool->prev = NULL;


  return mem_pool;
}

bool is_mallocable(MemoryBlock *heap) {
  if (heap->is_allocated == false)
    return true;

  return false;
}

void *find_free_block(MemoryBlock *heap, size_t size) {
  MemoryBlock *head = heap;

  while (head) {
    if(!head->is_allocated && head->size >= size)
      return head;
    head = head->next;
  }

  return NULL;
}

void *mem_alloc(MemoryBlock *heap, size_t size) {

  MemoryBlock *free_block = find_free_block(heap, size);
  if(!free_block) {
    return NULL;
  }

  if(free_block->size >= size + sizeof(MemoryBlock)) {

    size_t original_size = free_block->size;

    //Reduce size of current block - leaves memory space after current block
    free_block->size -= size + sizeof(MemoryBlock);

    // create allocated block at beginning of free_block
    MemoryBlock *allocated_block = free_block;
    allocated_block->is_allocated = true;
    allocated_block->size = size;

    // Calculate address of new free block after allocated block
    MemoryBlock *new_free_block = (MemoryBlock *)((char *)allocated_block + sizeof(MemoryBlock) + size);
    new_free_block->is_allocated;
    new_free_block->size = original_size - size + sizeof(MemoryBlock);

    // Update linked list
    new_free_block->next = allocated_block->next;
    new_free_block->prev = allocated_block;
    if(allocated_block->next) {
      allocated_block->next->prev = new_free_block;
    }
    allocated_block->next = new_free_block;

    //Return ptr to usable memory
    return (char *)allocated_block + sizeof(MemoryBlock);
  }

  // No split -> allocate whole block
  free_block->is_allocated = true;
  return (char *)free_block + sizeof(MemoryBlock);
}

void *mem_free(void *ptr_to_allocated_mem_block) {

  return NULL;
}
