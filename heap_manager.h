//
// Created by Jermaine on 04/03/2025.
//
#ifndef HEAP_MANAGER_H
#define HEAP_MANAGER_H

#include <stdio.h>

typedef struct MemoryBlock MemoryBlock;

void *init_mem_pool();

bool is_block_allocated(MemoryBlock *heap);

void *find_free_block(MemoryBlock *heap, size_t size);

void *mem_alloc(MemoryBlock *heap, size_t size);

void mem_free(void *user_ptr);

void print_heap(MemoryBlock *heap);

size_t mem_align(size_t size);

// TODO: implement memory compaction
void *mem_compact(MemoryBlock *heap);

void *mem_realloc(MemoryBlock *heap, size_t new_size);

void mem_coalesce(MemoryBlock **current_block);

void split_block_after_expansion(MemoryBlock *block, size_t new_block_size, size_t available_size);

int free_mem_pool(MemoryBlock *heap);

bool is_compaction_ready(MemoryBlock *heap);

bool fragmentation_threshold_reached(MemoryBlock *heap);

#endif //HEAP_MANAGER_H
