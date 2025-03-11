#include "heap_manager.h"

int main(void) {
  // Initialize a 1 MB heap pool.
  MemoryBlock *heap = init_mem_pool();
  if (heap == NULL) {
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

  fragmentation_threshold_reached(heap);

  mem_free(int_ptr);
  printf("second free: ");


  print_heap(heap);

  mem_free(char_ptr);
  printf("third free: ");
  print_heap(heap);

  // Release the entire memory pool.
  free_mem_pool(heap);
}


