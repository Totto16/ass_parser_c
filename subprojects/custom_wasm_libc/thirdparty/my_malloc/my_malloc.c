

#include <assert.h>
#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "./my_malloc.h"

#define INTERNAL_FUNCTION static

// the variables that start with __ can be visible globally, so they're  prefixed by __my_malloc_ so
// that it doesn't pollute the global scope additionally these are made static! (meaning no outside
// file can see them, on global variables this only makes them inivisible to other files )

typedef uint8_t status_t;

typedef uint8_t pseudoByte;

// FREE is 0, so each mmap (so initialized to 0) region has set every block to FREE
enum __my_malloc_alloc_status : uint8_t {
	FREE = 0,
	ALLOCED = 1,
};

typedef struct {
	void* nextBlock;
	void* previousBlock;
	status_t status;
} BlockInformation;

// every MemoryBlock starts with this
// [  BlockInformation | .....  ]

typedef struct {
	void* start;
	uint64_t size;
} GlobalMemoryBlockinformation;

typedef struct {
	GlobalMemoryBlockinformation global_block;
} GlobalObject;

static GlobalObject __my_malloc_globalObject = { .global_block = (GlobalMemoryBlockinformation){
	                                                 .size = 0, .start = NULL } };

#define PAGE_SIZE (1 << 16)

#define WASM_MEMORY_ID 0

/**
 * @brief INTERNAL FUNCTION: DO NOT USE
 *
 * @note Needs to be called with the mutex locked, in order to be thread safe!
 *
 */
INTERNAL_FUNCTION uint64_t size_of_double_pointer_block(BlockInformation* block) {
	if(block == NULL) {
		PANIC("INTERNAL: This is an allocator ERROR, this shouldn't occur: block is NULL");
	} else if(block->nextBlock == NULL) {
		const GlobalMemoryBlockinformation currentMemoryBlock =
		    __my_malloc_globalObject.global_block;

		return (((pseudoByte*)currentMemoryBlock.start + currentMemoryBlock.size) -
		        (pseudoByte*)block) -
		       sizeof(BlockInformation);
	} else {
		return ((pseudoByte*)block->nextBlock - (pseudoByte*)block) - sizeof(BlockInformation);
	}
}

/**
 * @brief INTERNAL FUNCTION; DO NOT USE
 *
 * @note Needs to be called with the mutex locked, in order to be thread safe!
 *
 */
INTERNAL_FUNCTION bool __my_malloc_block_fitsBetter(BlockInformation* toCompare,
                                                    BlockInformation* currentBlock, uint64_t size) {

	if(toCompare->status != FREE) {
		return false;
	}

	if(currentBlock->status != FREE) {
		return true;
	}

	const uint64_t blockSize = size_of_double_pointer_block(toCompare);

	// if a new block has to be "allocated" then there has to be space for that!
	if(toCompare->nextBlock == NULL) {

		if(blockSize == size) {
			return true;
		}

		if(blockSize < sizeof(BlockInformation) + size) {
			return false;
		}

		if(blockSize == sizeof(BlockInformation) + size) {
			return true;
		}
	}

	if(blockSize < size) {
		return false;
	}

	if(blockSize == size) {
		return true;
	}

	const uint64_t currentSize = size_of_double_pointer_block(currentBlock);

	if(currentSize > size + sizeof(BlockInformation)) {
		if(blockSize <= size + sizeof(BlockInformation)) {
			return false;
		}
	}
	return (blockSize - size) < (currentSize - size);
}

// TODO: align allocations to 8 bytes!

/**
 * @brief internal malloc, used by realloc and malloc, but doesn't lock mutexes, that is done by the
 * parent functions, DO NOT us outside of the internals of this file!
 */
INTERNAL_FUNCTION void* __internal__my_malloc(uint64_t size) {

	// calling my_malloc without initializing the allocator doesn't work, if that is the case,
	// likely the uninitialized mutex access before this will crash the program, but that is here
	// for safety measures! AND ALSO in the case of uninitialized allocator in the thread local case
	if(__my_malloc_globalObject.global_block.start == NULL) {
		PANIC("Calling malloc before initializing the allocator is prohibited!");
	}

	BlockInformation* bestFit = NULL;

	bestFit = (BlockInformation*)(((pseudoByte*)__my_malloc_globalObject.global_block.start));
	BlockInformation* nextFreeBlock = (BlockInformation*)bestFit->nextBlock;

	while(nextFreeBlock != NULL) {
		if(__my_malloc_block_fitsBetter(nextFreeBlock, bestFit, size)) {
			bestFit = nextFreeBlock;
			// shorthand evaluation, so if it fits perfectly don't look for a better one
			const uint64_t blockSize = size_of_double_pointer_block(bestFit);
			if(blockSize == size) {
				break;
			}
		}
		nextFreeBlock = nextFreeBlock->nextBlock;
	}

	const uint64_t blockSize = __my_malloc_globalObject.global_block.start == NULL
	                               ? 0
	                               : size_of_double_pointer_block(bestFit);

	// if the one that fit the best is not big enough, it means no block is big enough! If it's not
	// free, than there was no free block
	if(__my_malloc_globalObject.global_block.start == NULL || bestFit == NULL ||
	   bestFit->status != FREE || blockSize < size) {

		// grow the one memory block

		uint64_t preferredSize = PAGE_SIZE;

		if(preferredSize - sizeof(BlockInformation) < size) {
			preferredSize = size + -sizeof(BlockInformation);
		}

		// round up, /add PAGE_SIZE - 1 and then do a trunacting divide
		size_t pages_amount = (preferredSize + (PAGE_SIZE - 1)) / PAGE_SIZE;

		size_t old_pages = __builtin_wasm_memory_grow(WASM_MEMORY_ID, pages_amount);

		if(old_pages == ((size_t)-1)) {
			// don't fail, just return NULL ,indicating Out of memory
			return NULL;
		}

		if(old_pages * PAGE_SIZE != __my_malloc_globalObject.global_block.size) {
			PANIC("Old memory pages size was wrong, out of sync?");
		}

		size_t heap_size = __builtin_wasm_memory_size(WASM_MEMORY_ID) * PAGE_SIZE;

		if(heap_size < __my_malloc_globalObject.global_block.size) {
			PANIC("memory grow resulted in smaller memory, how did that happen?");
		}

		__my_malloc_globalObject.global_block.size = heap_size;

		// Now call internal malloc with the new block as hint, to use it, without duplicating
		// code and searching for it, if we already have it

		return __internal__my_malloc(size);
	};

	if(blockSize - size <= (sizeof(BlockInformation))) {
		// block size and size needed for allocation is the same, only need to set the status to
		// allocated

		// OR

		// block size and size needed for allocation is nearly the same, but can't allocate a new
		// block at the end, since it hasn't enough space for another BlockInformation, so only need
		// to set the status to allocated, but some size is wasted, it can create a gap of 1 or
		// more, that is fine, but gaps of 0 or less just "waste" that memory -- this handling
		// implicates, that no position of previous or next block may be calculated by using the
		// size!!

		bestFit->status = ALLOCED;

	} else {
		BlockInformation* newBlock =
		    (BlockInformation*)(((pseudoByte*)bestFit + sizeof(BlockInformation)) + size);

		// the new gap is at least 1 byte big, see above!

		newBlock->status = FREE;
		newBlock->nextBlock = bestFit->nextBlock; // can be NULL
		newBlock->previousBlock = bestFit;

		bestFit->status = ALLOCED;
		bestFit->nextBlock = newBlock;

		if(newBlock->nextBlock != NULL) {
			((BlockInformation*)newBlock->nextBlock)->previousBlock = newBlock;
		}
	}

	void* returnValue = (pseudoByte*)bestFit + sizeof(BlockInformation);

	return returnValue;
}

/**
 * @note MT-safe - with thread_local storage, this only accesses that, otherwise a mutex is
 * used, if this is called without initializing the underlying allocator beforehand, it is
 * undefined behaviour, however this function crashes the program in that case
 */
void* my_malloc(uint64_t size) {

	if(__my_malloc_globalObject.global_block.start == NULL) {
		PANIC("Calling malloc before initializing the allocator is prohibited!");
	}

	void* returnValue = __internal__my_malloc(size);

	return returnValue;
}

/**
 * @brief internal free, used by realloc and free, but doesn't lock mutexes, that is done by the
 * parent functions, DO NOT us outside of the internals of this file!
 */
INTERNAL_FUNCTION void __internal__my_free(void* ptr) {

	// calling my_free without initializing the allocator doesn't work, if that is the case,
	// likely the uninitialized mutex access before this will crash the program, but that is here
	// for safety measures! AND ALSO in the case of uninitialized allocator in the thread local case
	if(__my_malloc_globalObject.global_block.start == NULL) {
		PANIC("Calling free before initializing the allocator is prohibited!");
	}

	BlockInformation* currentBlock =
	    (BlockInformation*)((pseudoByte*)ptr - sizeof(BlockInformation));

	if(__my_malloc_globalObject.global_block.start == NULL) {
		// no block is not free, since we have no block anymore xD
		PANIC("ERROR: You tried to free a already freed Block");
	}

	if(currentBlock->status == FREE) {
		PANIC("ERROR: You tried to free a already freed Block");
	}

	currentBlock->status = FREE;

	BlockInformation* nextBlock = (BlockInformation*)currentBlock->nextBlock;
	BlockInformation* previousBlock = (BlockInformation*)currentBlock->previousBlock;

	// merge with previous free block, if in 5the same memory block!
	if(previousBlock != NULL && previousBlock->status == FREE) {

		// MERGE three free blocks into one: layout Previous | Current | Next => New Free one
		if(nextBlock != NULL && nextBlock->status == FREE) {
			previousBlock->nextBlock = nextBlock->nextBlock; // Can be NULL

			if(nextBlock->nextBlock != NULL) {
				((BlockInformation*)nextBlock->nextBlock)->previousBlock = previousBlock;
			}
			// merge previous free block with current one
		} else {

			previousBlock->nextBlock = nextBlock; // can be NULL
			if(nextBlock != NULL) {
				nextBlock->previousBlock = previousBlock;
			}
		}

		// merge next free block with current one, if in the same memory block
	} else if(nextBlock != NULL && nextBlock->status == FREE) {
		currentBlock->nextBlock = nextBlock->nextBlock; // can be NULL

		if(nextBlock->nextBlock != NULL) {
			((BlockInformation*)nextBlock->nextBlock)->previousBlock = currentBlock;
		}
	}
}

/**
 * @brief frees a pointer, a NULL pointer is ignored and a safe noop,
 * if the pointer is not allocated with my_malloc, this call is undefined behaviour. It likely will
 * crash or create a blockInformation structure, that will crash in later stages, since it tries to
 * interpret some random garbage memory as block-structure, so be aware of that!
 * DOUBLE Frees crash the program, so remember to always set freed pointer sto NULL :)
 *
 * @note MT-safe, using the mutex, or the thread local storage, the same principles as in my_malloc
 * apply, so calling this with an uninitialized allocator is undefined behaviour and crashes the
 * program
 *
 */
void my_free(void* ptr) {

	// so that if you pass a wrong argument just nothing happens!
	if(ptr == NULL) {
		return;
	}

	__internal__my_free(ptr);
}

/**
 * @brief If ptr is NULL, this behaves as my_malloc
 * If size == 0 it behaves as my_free and returns NULL
 *
 * Otherwise it reallocates the memory, it may have a different address than before, but the return
 * value may also be the same as ptr. If the new size is greater than the previous size, the whole
 * content of the previous ptr is preserves and copied to the new ptr, if needed, the rest of the
 * data is undefined. If the new size is smaller, the data beyond that is potentially overwritten,
 * at least it's not accessible anymore, the returned ptr can be the same, but doesn't have to be
 * the same, since realloc may chose a better suited block for it, if that'S the case, the data up
 * to the new size is the same as the old one
 */
void* my_realloc(void* ptr, uint64_t size) {

	// if ptr == NULL, it is the same as my_malloc(size);
	if(ptr == NULL) {
		return my_malloc(size);
	}

	// if size == 0, it is the same as my_free(size);
	if(size == 0) {
		my_free(ptr);
		return NULL;
	}

	// calling my_malloc without initializing the allocator doesn't work, if that is the case,
	// likely the uninitialized mutex access before this will crash the program, but that is here
	// for safety measures! AND ALSO in the case of uninitialized allocator in the thread local case
	if(__my_malloc_globalObject.global_block.start == NULL) {
		PANIC("Calling realloc before initializing the allocator is prohibited!");
	}

	BlockInformation* currentBlock =
	    (BlockInformation*)((pseudoByte*)ptr - sizeof(BlockInformation));

	if(__my_malloc_globalObject.global_block.start == NULL) {
		// no block is not free, since we have no block anymore xD
		PANIC("ERROR: You tried to realloc a freed Block");
	}

	if(currentBlock->status == FREE) {
		PANIC("ERROR: You tried to realloc a freed Block");
	}

	// ATTENTION: this size isn't always the correct size, of the previous alloc! since some amount
	// of dread space can be at the end, it can be between 0 and sizeof(BlockInformation) bytes,
	// since there's no room for a new block in there. So every calculation here has to pay
	// attention to that

	// It is fine, to copy the undefined memory, since it's  at the end, where the new memory would
	// be undefined nevertheless
	const uint64_t blockSize = size_of_double_pointer_block(currentBlock);

	// CASE 1: the new size is smaller or the same (it may be also the same, if the blockSize is
	// slightly bigger, since there might be end padding!)
	if(size <= blockSize) {

		// CASE 1.1: no new block can be placed after the new size, so just returning the old size
		// and doing some valgrind house keeping
		if(blockSize - size <= sizeof(BlockInformation)) {

			// just return the old pointer
			return ptr;

		} else {

			// CASE 1.2: make a new block, that is free, and is at the end of size

			// If applicable we search a better block, that is better suited for that cause, that
			// is computational intensive, but it may create less holes in the end also pay
			// attention to padding, so it has to be at least sizeof(BlockInformation) smaller, to
			// allocate a new block!

			// CASE 1.2.1: the new areas is significantly smaller than the last one, so using free +
			// malloc to get a better spot for the significantly smaller size, use 50% as threshold,
			// so that if it's 50% smaller, use malloc to get a new block
			// small NOTE: since we don't free this block before issuing a malloc, this block might
			// be suited better, but we have to use another xD, it might even return NULL, so it'S
			// out of memory xD
			if(size * 2 < blockSize) {

				void* newRegion = __internal__my_malloc(size);

				// out of memory, so just use the current nevertheless xD
				// ATTENTION: code duplication, since no good pattern emerges, to reuse code via
				// call or control flow :(
				if(newRegion == NULL) {

					BlockInformation* newBlock =
					    (BlockInformation*)(((pseudoByte*)currentBlock + sizeof(BlockInformation)) +
					                        size);
					newBlock->nextBlock = currentBlock->nextBlock; // may be NULL
					newBlock->previousBlock = currentBlock;
					newBlock->status = FREE;

					currentBlock->nextBlock = newBlock;
					if(newBlock->nextBlock != NULL) {
						((BlockInformation*)newBlock->nextBlock)->previousBlock = newBlock;
					}

					// just return the old pointer
					return ptr;
				}

				// copy the subset of data into the new region
				void* dest = memcpy(newRegion, ptr, size);

				if(dest != newRegion) {
					PANIC("Error during memcpy, dest pointer is not the same as the given dest "
					      "pointer");
				}

				// free the previous section
				__internal__my_free(ptr);

				// return the new region
				return newRegion;

			} else {

				// CASE 1.2.2: just divide the block and use the current One

				BlockInformation* newBlock =
				    (BlockInformation*)(((pseudoByte*)currentBlock + sizeof(BlockInformation)) +
				                        size);

				newBlock->nextBlock = currentBlock->nextBlock; // may be NULL
				newBlock->previousBlock = currentBlock;
				newBlock->status = FREE;

				currentBlock->nextBlock = newBlock;
				if(newBlock->nextBlock != NULL) {
					((BlockInformation*)newBlock->nextBlock)->previousBlock = newBlock;
				}

				// just return the old pointer
				return ptr;
			}
		}

	} else {
		// CASE 2: the size is bigger

		// it is enough to look forward one block, since there is per guarantee no block that is
		// also free, after a free block

		BlockInformation* nextBlock = (BlockInformation*)currentBlock->nextBlock; // may be NULL

		// Case 2.1: the current block with the next block can fit the new size!
		if(nextBlock != NULL && nextBlock->status == FREE) {
			const uint64_t nextBlockSize = size_of_double_pointer_block(nextBlock);

			const uint64_t totalPotentialSize =
			    nextBlockSize + sizeof(BlockInformation) + blockSize;

			if(totalPotentialSize >= size) {

				// figure out, if theres space for another block inside the new larger area!

				// CASE 2.1.1: no new block can be placed inside the new larger area, just deleting
				// the old in the middle (nextBlock)
				if(totalPotentialSize - size <= (sizeof(BlockInformation))) {

					currentBlock->nextBlock = nextBlock->nextBlock; // can be NULL
					if(nextBlock->nextBlock != NULL) {
						((BlockInformation*)nextBlock->nextBlock)->previousBlock = currentBlock;
					}

					// just return the old pointer, it has now space for the size
					return ptr;

				} else {

					// CASE 2.1.2: delete the current one and create a new one at the end, that is
					// free

					BlockInformation* newBlock =
					    (BlockInformation*)(((pseudoByte*)currentBlock + sizeof(BlockInformation)) +
					                        size);

					newBlock->previousBlock = currentBlock;
					newBlock->nextBlock = nextBlock->nextBlock; // can be NULL
					newBlock->status = FREE;

					currentBlock->nextBlock = newBlock;

					if(nextBlock->nextBlock != NULL) {
						((BlockInformation*)nextBlock->nextBlock)->previousBlock = newBlock;
					}

					// just return the old pointer, it has now space for the size
					return ptr;
				}
			}
		}

		// CASE 2.2 we need to issue a new malloc and copy the data over
		void* newRegion = __internal__my_malloc(size);

		if(newRegion == NULL) {

			return NULL;
		}

		// copy the data into the new region, the blockSize is not 100%% accurate, but as said
		// above, the rest is undefined memory, as the rest of the newRegion region
		void* dest = memcpy(newRegion, ptr, blockSize);

		if(dest != newRegion) {
			PANIC("Error during memcpy, dest pointer is not the same as the given dest pointer");
		}

		// free the previous section
		__internal__my_free(ptr);

		// return the new region
		return newRegion;
	}
}

/**
 * @note NOT MT-safe. this function HAS TO BE called exactly once at the start of every program,
 * that uses this. If using thread_local storage, you have to call it once per thread. After that
 * every call to free and malloc is thread safe in both cases. If this fails, the program crashes.
 * No error is returned
 *
 * By default the allocator doesn't allocate a memory block, it creates a block in the first called
 * malloc. But you can force the creation of, one, if you wish so, but free may remove the last one,
 * no guarantee there. This whole thing would largely benefit programs, that don't use any dynamic
 * memory, but use this malloc, so no mmap call will be issued and no memory is required, if they
 * don't opt in into it
 *
 */

extern void* __heap_base;

void my_allocator_init(void) {
	__my_malloc_globalObject.global_block =
	    (GlobalMemoryBlockinformation){ .size = 0, .start = NULL };

	GlobalMemoryBlockinformation global_block = { .start = __heap_base, .size = 0 };

	size_t heap_size = __builtin_wasm_memory_size(WASM_MEMORY_ID) * PAGE_SIZE;

	global_block.size = heap_size;

	// initialize the first block
	BlockInformation* firstBlock = (BlockInformation*)((pseudoByte*)global_block.start);

	firstBlock->nextBlock = NULL;
	firstBlock->previousBlock = NULL;
	firstBlock->status = FREE;
}

#include "./statistics.h"

#include <stdio.h>

AllocatorStatistics allocator_get_statistics(void) {

	if(__my_malloc_globalObject.global_block.start == NULL) {
		PANIC("allocator not initialized!");
	}

	AllocatorStatistics statistics = {
		.free = 0,
		.used = 0,
		.metadata = 0,
		.total = __my_malloc_globalObject.global_block.size,
	};

	BlockInformation* currentBlock =
	    (BlockInformation*)(((pseudoByte*)__my_malloc_globalObject.global_block.start));

	uint64_t total_available_for_user = 0;

	while(currentBlock != NULL) {

		uint64_t size = size_of_double_pointer_block(currentBlock);

		if(currentBlock->status == FREE) {
			statistics.free += size;
		} else {
			statistics.used += size;
		}

		total_available_for_user += size;

		currentBlock = currentBlock->nextBlock;
	}

	fprintf(stderr, "here total: %lu # %lu", __my_malloc_globalObject.global_block.size,
	        statistics.total);

	statistics.metadata = statistics.total - total_available_for_user;

	return statistics;
}
