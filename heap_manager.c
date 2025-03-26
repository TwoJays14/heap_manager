#include <stdio.h>
#include <windows.h>
#include <stddef.h>
#include <minwindef.h>
#include "heap_manager.h"


#define ALIGNMENT_SIZE 8
#define MIN_BLOCK_SIZE 4
#define HEAP_SIZE (1024 * 1024)
#define ALIGN_SIZE(size) (size + (ALIGNMENT_SIZE - 1)) & ~(ALIGNMENT_SIZE - 1)
#define NON_CONTIGUOUS_FREE_BLOCK_COUNT 5


typedef struct MemoryBlock {
  size_t size;
  bool is_allocated;
  struct MemoryBlock *next;
  struct MemoryBlock *prev;
} MemoryBlock;

void* init_mem_pool() {
  // request virtual memory
  const LPVOID heap = VirtualAlloc(NULL, HEAP_SIZE, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

  MemoryBlock *mem_pool = heap;

  // null pointer error handling
  if (heap == NULL) {
    const DWORD error = GetLastError();
    fprintf(stderr, "Failed to allocate memory. Error Code: %lu", error);
    return NULL;
  }

  // initialise mem pool metadata
  mem_pool->size = HEAP_SIZE - sizeof(MemoryBlock);
  mem_pool->is_allocated = false;
  mem_pool->next = NULL;
  mem_pool->prev = NULL;


  return mem_pool;
}

bool is_block_allocated(MemoryBlock *heap) {
  return !heap->is_allocated;
}

void* find_free_block(MemoryBlock *heap, size_t size) {
  MemoryBlock *head = heap;

  while (head) {
    if (is_block_allocated(head) && head->size >= size)
      return head;
    head = head->next;
  }

  return NULL;
}

void* mem_alloc(MemoryBlock *heap, size_t const size) {
  size_t const mem_aligned_size = mem_align(size);
  MemoryBlock *free_block_to_be_allocated = find_free_block(heap, mem_aligned_size + sizeof(MemoryBlock));

  if (free_block_to_be_allocated == NULL) {
    return NULL;
  }

  MemoryBlock *allocated_block = free_block_to_be_allocated;

  // check if the free block is large enough to be allocated
  if (free_block_to_be_allocated->size >= mem_aligned_size + sizeof(MemoryBlock) + MIN_BLOCK_SIZE) {
    const size_t original_size = free_block_to_be_allocated->size;


    //Mark beginning portion as allocated.
    allocated_block->is_allocated = true;
    allocated_block->size = mem_aligned_size;


    // Calculate address of new free block after allocated block
    MemoryBlock *new_free_block = (MemoryBlock *) (
      (char *) allocated_block + sizeof(MemoryBlock) + mem_aligned_size);
    // TODO: validate block splitting conditions

    // set new block metadata
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

  return (void *) (char *) allocated_block + sizeof(MemoryBlock);
}

void mem_free(void *user_ptr) {
  if (user_ptr == NULL) {
    return;
  }

  // TODO:  verify that the new block’s starting address remains correctly aligned.
  MemoryBlock *current_block = (MemoryBlock *) ((char *) user_ptr - sizeof(MemoryBlock));

  current_block->is_allocated = false;

  // Coalesce with previous adjacent block
  mem_coalesce(&current_block);
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
  // rounds size of memory to be allocated to nearest 8 byte boundary
  return (size + (ALIGNMENT_SIZE - 1)) & ~(ALIGNMENT_SIZE - 1);
}

void* mem_realloc(MemoryBlock *heap, size_t new_size) {
  MemoryBlock *ptr = heap;

  if (new_size == 0 && ptr != NULL) {
    mem_free(ptr);
    return NULL;
  }

  if (ptr == NULL && new_size == 0) {
    return NULL;
  }

  if (ptr == NULL) {
    ptr = mem_alloc(ptr, new_size);
    return ptr;
  }

  MemoryBlock *current_block_address = (MemoryBlock *) ((char *) ptr - sizeof(MemoryBlock));

  if (!current_block_address->is_allocated) {
    return NULL;
  }

  size_t const new_block_size = mem_align(new_size);

  // if shrinking

  if (new_block_size <= current_block_address->size) {
    if (current_block_address->size - new_block_size >= sizeof(MemoryBlock) + MIN_BLOCK_SIZE) {
      MemoryBlock *new_free_block = (MemoryBlock *) ((char *) ptr + new_block_size);

      new_free_block->size = current_block_address->size - new_block_size - sizeof(MemoryBlock);
      new_free_block->is_allocated = false;
      new_free_block->next = current_block_address->next;
      new_free_block->prev = current_block_address;

      current_block_address->next = new_free_block;
      current_block_address->size = new_block_size;

      if (new_free_block->next != NULL) {
        if (!new_free_block->next->is_allocated) {
          mem_coalesce(&new_free_block->next);
        }
      }

      return ptr;
    }
  } else {
    current_block_address->size = new_block_size;
  }

  // if expanding
  if (current_block_address->next != NULL && !current_block_address->next->is_allocated) {
    if (current_block_address->size + sizeof(MemoryBlock) + current_block_address->next->size >= new_block_size) {
      size_t available_size = current_block_address->size + sizeof(MemoryBlock) + current_block_address->next->size;
      current_block_address->size = new_block_size;

      if (available_size - new_block_size >= sizeof(MemoryBlock) + MIN_BLOCK_SIZE) {
        split_block_after_expansion(current_block_address, new_block_size, available_size);
      }

      if (current_block_address->next->next != NULL) {
        current_block_address->next = current_block_address->next->next;
        current_block_address->next->prev = current_block_address;
      } else {
        current_block_address->next = NULL;
      }
      return ptr;
    }
  };

  // if block is already large enough
  if (current_block_address->size >= new_block_size) {
    return ptr;
  };

  // Relocate
  MemoryBlock *new_ptr = mem_alloc(heap, new_block_size);

  memcpy(new_ptr, ptr, min(current_block_address->size, new_block_size));

  mem_free(ptr);

  return new_ptr;
}

void split_block_after_expansion(MemoryBlock *current_block, size_t new_block_size, size_t available_size) {
  MemoryBlock *new_block = (MemoryBlock *) ((char *) current_block + sizeof(MemoryBlock) + new_block_size);

  // set block metadata
  new_block->size = available_size - (new_block_size + sizeof(MemoryBlock));
  new_block->is_allocated = false;
  new_block->next = current_block->next->next;
  new_block->prev = current_block;

  current_block->next = new_block;

  if (new_block->next != NULL) {
    new_block->next->prev = new_block;
  };

  if (new_block->next != NULL) {
    if (!new_block->next->is_allocated) {
      mem_coalesce(&new_block->next);
    };
  };
};

void mem_coalesce(MemoryBlock **current_block) {
  // Coalesce with previous adjacent block
  if ((*current_block)->prev && !(*current_block)->prev->is_allocated) {
    (*current_block)->prev->size += (*current_block)->size + sizeof(MemoryBlock);
    (*current_block)->prev->size = mem_align((*current_block)->prev->size);

    (*current_block)->prev->next = (*current_block)->next;

    if ((*current_block)->next) {
      (*current_block)->next->prev = (*current_block)->prev;
    }

    *current_block = (*current_block)->prev;
  }


  // Coalesce with next adjacent block
  if ((*current_block)->next && !(*current_block)->next->is_allocated) {
    (*current_block)->size += (*current_block)->next->size + sizeof(MemoryBlock);
    (*current_block)->size = mem_align((*current_block)->size);

    if ((*current_block)->next->next) {
      (*current_block)->next = (*current_block)->next->next;

      if ((*current_block)->next->prev) {
        (*current_block)->next->prev = *current_block;
      }
    }
  }
}

int free_mem_pool(MemoryBlock *heap) {
  if (!VirtualFree(heap, 0, MEM_RELEASE)) {
    DWORD const error = GetLastError();
    fprintf(stderr, "VirtualFree failed with error %lu\n", error);
    return 1;
  }

  return 0;
}

bool fragmentation_threshold_reached(MemoryBlock *heap) {
  if (heap == NULL) {
    return false;
  }

  int transition_count = 0;
  bool prev_block_is_allocated = heap->is_allocated;

  MemoryBlock *current = heap;

  while (current != NULL) {
    if (current != heap && prev_block_is_allocated && !current->is_allocated) {
      transition_count++;
    }

    prev_block_is_allocated = current->is_allocated;
    current = current->next;
  }

  int non_contiguous_free_blocks = transition_count;

  if (non_contiguous_free_blocks >= NON_CONTIGUOUS_FREE_BLOCK_COUNT) {
    return true;
  }

  return false;
}

void* mem_compact(MemoryBlock *heap) {

  // loop through heap

  // find transition pairs - where A=>F then F=>A

  // calculate space between transition pairs i.e. 2 free blocks between allocated blocks

  // do pointer arithmetic to get ptrs to relevant blocks

  // move allocated block so allocated blocks are adjacent

  // set blocks metadata

  // repeat until there is only one transition pair


  return heap;
}
