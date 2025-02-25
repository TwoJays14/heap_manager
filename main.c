#include <stdio.h>
#include <windows.h>
#include <stddef.h>

#define HEAP_SIZE (1024 * 1024)

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

void mem_free(void *user_ptr);

void print_heap(MemoryBlock *heap);


int main(void) {
  // Initialize a 1 MB heap pool.
  MemoryBlock *heap = init_mem_pool();
  if (!heap) {
    fprintf(stderr, "Heap initialization failed.\n");
    return 1;
  }

  printf("=== Initial Heap State ===\n");
  print_heap(heap);

  void *int_ptr = mem_alloc(heap, 2000);
  printf("\n=== After Allocating an int ===\n");
  print_heap(heap);

  void *double_ptr = mem_alloc(heap, 43320);
  printf("\n=== After Allocating a double ===\n");
  print_heap(heap);

  void *char_ptr = mem_alloc(heap, 123124);
  printf("\n=== After Allocating char[50] ===\n");
  print_heap(heap);

  mem_free(double_ptr);
  printf("\n=== After Freeing the double block ===\n");
  print_heap(heap);

  mem_free(int_ptr);
  printf("\n=== After Freeing the int block ===\n");
  print_heap(heap);

  mem_free(char_ptr);
  printf("\n=== After Freeing the char block (Complete Coalescing) ===\n");
  print_heap(heap);

  // Release the entire memory pool.
  if (!VirtualFree(heap, 0, MEM_RELEASE)) {
    DWORD const error = GetLastError();
    fprintf(stderr, "VirtualFree failed with error %lu\n", error);
    return 1;
  }

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
  return !heap->is_allocated;
}

void *find_free_block(MemoryBlock *heap, size_t size) {
  MemoryBlock *head = heap;

  while (head) {
    if (!head->is_allocated && head->size >= size)
      return head;
    head = head->next;
  }

  return NULL;
}

void *mem_alloc(MemoryBlock *heap, size_t size) {
  MemoryBlock *free_block = find_free_block(heap, size);
  if (!free_block) {
    return NULL;
  }

  if (free_block->size >= size + sizeof(MemoryBlock) + 1) {
    const size_t original_size = free_block->size;

    //Mark beginning portion as allocated.
    free_block->is_allocated = true;
    free_block->size = size;


    // Calculate address of new free block after allocated block
    const auto new_free_block = (MemoryBlock *) ((char *) free_block + sizeof(MemoryBlock) + size);
    new_free_block->size = original_size - size - sizeof(MemoryBlock);
    new_free_block->is_allocated = false;

    // Update linked list
    new_free_block->prev = free_block;
    new_free_block->next = free_block->next;

    if (free_block->next) {
      free_block->next->prev = new_free_block;
    }

    free_block->next = new_free_block;

  } else {
    // No split -> allocate whole block
    free_block->is_allocated = true;
  }

  return (char *) free_block + sizeof(MemoryBlock);
}

void mem_free(void *user_ptr) {
  if (user_ptr == NULL) {
    return;
  }

  MemoryBlock *current_block = (MemoryBlock *) ((char *) user_ptr - sizeof(MemoryBlock));

  current_block->is_allocated = false;


  // Coalesce with previous adjacent block

  if (current_block->prev && !current_block->prev->is_allocated) {
    current_block->prev->size += current_block->size + sizeof(MemoryBlock);

    current_block->prev->next = current_block->next;

    if (current_block->next) {
      current_block->next->prev = current_block->prev;
    }

    current_block = current_block->prev;
  }


  // Coalesce with next adjacent block
  if (current_block->next && !current_block->next->is_allocated) {
    current_block->size += current_block->next->size + sizeof(MemoryBlock);

    if (current_block->next->next) {
      current_block->next = current_block->next->next;

      if (current_block->next->prev) {
        current_block->next->next->prev = current_block;
      }
    }
  }

  return;
}

void print_heap(MemoryBlock *heap) {
  MemoryBlock *current = heap;
  int index = 0;
  while (current) {
    printf("Block %d: Addr: %p | Size: %zu | %s\n",
           index,
           (void *) current,
           current->size,
           current->is_allocated ? "Allocated" : "Free");
    current = current->next;
    index++;
  }
}
