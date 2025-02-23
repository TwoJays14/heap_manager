// //
// // Created by Jermaine on 17/02/2025.
// //Understand VirtualAlloc and VirtualFree:
// Learn how to reserve and commit a large block of memory using VirtualAlloc. Understand that VirtualFree will be used later to release the entire memory pool when it is no longer needed.
//
// Design Your Memory Block Structure:
// Define a structure that represents a block of memory. This structure should include the size of the block, a flag indicating whether it is free or allocated, and pointers to the next and previous blocks. This header will be stored at the beginning of each block in your memory pool.
//
// Initialize the Memory Pool:
// Create an initialization routine that takes the pointer to the reserved memory and its total size. In this routine, set up a single large free block that spans nearly the entire pool (subtracting the space needed for the header).
//
// Implement a Free List Using a Doubly Linked List:
// Use a doubly linked list to track free memory blocks. Each node in the list should reference a free memory block (using your memory block structure). Develop helper routines to add nodes to the free list, remove nodes from it, and update nodes when blocks are split or coalesced.
//
// Implement a Free Block Search Function:
// Create a function that traverses the free list to find a block that is free and has enough size to satisfy a memory allocation request. This function should return a pointer to a suitable memory block or indicate that no such block exists.
//
// Develop the Allocation Function:
// Write an allocation routine that first uses your free block search function. If a sufficiently large free block is found, decide whether it should be used as is or split into two parts. In the case of splitting, allocate the required amount and then create a new free block for the remaining memory, updating the free list accordingly. Finally, mark the allocated block as not free and return a pointer to the usable memory (just after the block header).
//
// Develop the Freeing Function:
// Implement a free routine that takes a pointer to allocated memory, converts it back to its corresponding block header, and marks it as free. Then, check if adjacent blocks are free and merge (coalesce) them to reduce fragmentation. Finally, update the free list to include the newly freed (and possibly merged) block.
//
// Keep the Free List Updated:
// Ensure that every time a block is allocated or freed (or split/merged), the free list is updated accordingly. This includes removing a block from the free list when it’s allocated, and adding it (or its parts) back when it’s freed or merged.
//
// Implement Debugging and Logging Functions:
// Write functions that traverse and print the state of your memory pool and free list. These debugging functions help you observe the behavior of your allocator and verify that the free list and block headers are being updated correctly.
//
// Ensure Proper Memory Alignment:
// Decide on an appropriate alignment boundary (typically based on the size of a pointer or the platform’s maximum alignment requirement) and adjust the allocation sizes to ensure that returned pointers are properly aligned. This step may involve rounding up the requested size.
//
// Test Your Heap Manager Thoroughly:
// Create a test routine that initializes the memory pool, performs various allocations and frees, and uses your debugging functions to log the state after each operation. Testing different allocation sizes and orders helps ensure your splitting and coalescing logic is correct.
//
// Release the Entire Memory Pool:
// When the program is finished with the heap manager, call VirtualFree on the entire memory pool to release the reserved memory back to the operating system.
