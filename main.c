#include <stdio.h>
#include <windows.h>
#include <stddef.h>

#define ALIGNMENT_SIZE 8
#define MIN_BLOCK_SIZE 4
#define HEAP_SIZE (1024 * 1024)
#define ALIGN_SIZE(size) (size + (ALIGNMENT_SIZE - 1)) & ~(ALIGNMENT_SIZE - 1)


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

size_t mem_align(size_t size);

// TODO: implement memory compaction
void mem_compact(MemoryBlock *heap);

void *mem_realloc(MemoryBlock *heap, size_t new_size);


int main(void) {
  // Initialize a 1 MB heap pool.
  MemoryBlock *heap = init_mem_pool();
  if (!heap) {
    fprintf(stderr, "Heap initialization failed.\n");
    return 1;
  }

  printf("=== Initial Heap State ===\n");
  print_heap(heap);

  void *int_ptr = mem_alloc(heap, 2001);
  void *double_ptr = mem_alloc(heap, 43320);
  void *char_ptr = mem_alloc(heap, 123124);
  print_heap(heap);

  mem_free(double_ptr);
  printf("first free: ");
  print_heap(heap);

  mem_free(int_ptr);
  printf("second free: ");


  print_heap(heap);

  mem_free(char_ptr);
  printf("third free: ");
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
    if (is_mallocable(head) && head->size >= size)
      return head;
    head = head->next;
  }

  return NULL;
}

void *mem_alloc(MemoryBlock *heap, size_t const size) {
  size_t const mem_aligned_size = mem_align(size);
  MemoryBlock *free_block_to_be_allocated = find_free_block(heap, mem_aligned_size);
  if (free_block_to_be_allocated == NULL) {
    return NULL;
  }

  MemoryBlock *allocated_block = free_block_to_be_allocated;

  if (free_block_to_be_allocated->size >= mem_aligned_size + sizeof(MemoryBlock) + MIN_BLOCK_SIZE) {
    const size_t original_size = free_block_to_be_allocated->size;


    //Mark beginning portion as allocated.
    allocated_block->is_allocated = true;
    allocated_block->size = mem_aligned_size;


    // Calculate address of new free block after allocated block
    const auto new_free_block = (MemoryBlock *) ((uintptr_t) allocated_block + sizeof(MemoryBlock) + mem_aligned_size);
    // TODO: validate block splitting conditions
    new_free_block->size = original_size - mem_aligned_size - sizeof(MemoryBlock);
    new_free_block->is_allocated = false;

    // Update linked list
    new_free_block->prev = allocated_block;
    new_free_block->next = allocated_block->next;

    if (allocated_block->next) {
      allocated_block->next->prev = new_free_block;
    }

    allocated_block->next = new_free_block;
  } else {
    // No split -> allocate whole block
    allocated_block->is_allocated = true;
  }

  return (void *) (uintptr_t) allocated_block + sizeof(MemoryBlock);
}

void mem_free(void *user_ptr) {
  if (user_ptr == NULL) {
    return;
  }

  // TODO:  verify that the new block’s starting address remains correctly aligned.
  MemoryBlock *current_block = (MemoryBlock *) ((uintptr_t) user_ptr - sizeof(MemoryBlock));

  current_block->is_allocated = false;


  // Coalesce with previous adjacent block
  if (current_block->prev && !current_block->prev->is_allocated) {
    current_block->prev->size += current_block->size + sizeof(MemoryBlock);
    current_block->prev->size = mem_align(current_block->prev->size);

    current_block->prev->next = current_block->next;

    if (current_block->next) {
      current_block->next->prev = current_block->prev;
    }

    current_block = current_block->prev;
  }


  // Coalesce with next adjacent block
  if (current_block->next && !current_block->next->is_allocated) {
    current_block->size += current_block->next->size + sizeof(MemoryBlock);
    current_block->size = mem_align(current_block->size);

    if (current_block->next->next) {
      current_block->next = current_block->next->next;

      if (current_block->next->prev) {
        current_block->next->next->prev = current_block;
      }
    }
  }
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

size_t mem_align(size_t size) {
  return size + (ALIGNMENT_SIZE - 1) & ~(ALIGNMENT_SIZE - 1);
}

void *mem_realloc(MemoryBlock *heap, size_t new_size) {
  MemoryBlock *ptr = heap;

  if (ptr == NULL) {
    mem_alloc(heap, new_size);
  }

  if (new_size == 0 && ptr != NULL) {
    mem_free(ptr);
    return NULL;
  }

  if (ptr == NULL & new_size == 0) {
    return NULL;
  }

  MemoryBlock *current_block_address = (MemoryBlock *)((uintptr_t) ptr - sizeof(MemoryBlock));

  if (!current_block_address->is_allocated) {
    return NULL;
  }

  size_t new_block_size = mem_align(current_block_address->size);

  if (new_block_size <= current_block_address->size) {
    if (current_block_address->size - new_block_size >= sizeof(MemoryBlock) + MIN_BLOCK_SIZE) {
      MemoryBlock *new_free_block = (MemoryBlock *)((uintptr_t)ptr + new_block_size);

      new_free_block->size = current_block_address->size - new_block_size - sizeof(MemoryBlock) + MIN_BLOCK_SIZE;
      new_free_block->is_allocated = false;
      new_free_block->next = current_block_address->next;
      new_free_block->prev = current_block_address;

      current_block_address->next = new_free_block;
      current_block_address->size = new_block_size;


      return ptr;
    }
  } else {
    current_block_address->size = new_block_size;
  }

  return NULL;
}