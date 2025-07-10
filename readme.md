# Dynamic Memory Allocator

A custom implementation of malloc, free, and realloc using mmap/munmap system calls. This library can be used as a drop-in replacement for the standard libc memory allocation functions.

## Table of Contents

- [Overview](#overview)
- [Architecture](#architecture)
- [Implementation Details](#implementation-details)
- [Building and Usage](#building-and-usage)
- [Testing](#testing)
- [Performance Considerations](#performance-considerations)
- [Technical Specifications](#technical-specifications)

## Overview

This project implements a dynamic memory allocator that manages memory through a three-tier zone system, minimizing system calls while providing efficient memory allocation and deallocation.

### Key Features

- **Zone-based allocation**: Pre-allocates memory zones to reduce mmap calls
- **Three-tier system**: TINY, SMALL, and LARGE allocations
- **Block coalescing**: Merges adjacent free blocks to reduce fragmentation
- **Block splitting**: Splits large blocks when smaller allocations are requested
- **Memory alignment**: All allocations are 16-byte aligned
- **Visual debugging**: `show_alloc_mem()` displays current memory state

## Architecture

### Zone Types

The allocator uses three types of zones based on allocation size:

1. **TINY Zone** (1-512 bytes)
   - Zone size: 16KB (4 pages)
   - Handles small allocations efficiently
   - Multiple allocations per zone

2. **SMALL Zone** (513-4096 bytes)
   - Zone size: 128KB (32 pages)
   - Handles medium-sized allocations
   - Multiple allocations per zone

3. **LARGE Zone** (4097+ bytes)
   - Each allocation gets its own mmap
   - No pre-allocation
   - Immediately unmapped when freed

### Memory Layout

```
Zone Structure:
┌─────────────────┐
│   Zone Header   │ (48 bytes)
├─────────────────┤
│  Block Header   │ (16 bytes)
├─────────────────┤
│   User Data     │
├─────────────────┤
│  Block Header   │
├─────────────────┤
│   User Data     │
└─────────────────┘

Zone Header:
- next: pointer to next zone
- prev: pointer to previous zone
- size: total zone size
- type: TINY/SMALL/LARGE
- free_blocks: count of free blocks
- free_size: total free space

Block Header:
- size: block size (LSB used as free flag)
- prev_size: size of previous block
```

## Implementation Details

### Memory Allocation Strategy

1. **Size Classification**
   ```c
   if (size <= 512) → TINY_ZONE
   else if (size <= 4096) → SMALL_ZONE
   else → LARGE_ZONE
   ```

2. **Allocation Process**
   - Align requested size to 16 bytes
   - Search existing zones for free space
   - If no space found, create new zone
   - Find first free block that fits (first-fit algorithm)
   - Split block if remaining space is significant
   - Mark block as allocated

3. **Free Block Management**
   - Blocks use LSB of size field as free flag
   - Free blocks are coalesced with adjacent free blocks
   - Zones are unmapped when completely empty (LARGE zones immediately)

### Key Algorithms

#### Block Splitting
When allocating from a larger free block:
```
If (block_size > requested_size + HEADER + MIN_BLOCK_SIZE):
    1. Allocate requested_size from beginning
    2. Create new free block with remaining space
    3. Update metadata for both blocks
```

#### Block Coalescing
When freeing a block:
```
1. Check if next block is free → merge
2. Check if previous block is free → merge
3. Update size to combined total
4. Reduce free block count accordingly
```

#### Zone Management
- Zones are kept in sorted linked lists by address
- Allows efficient zone lookup for pointer validation
- Enables proper ordering in show_alloc_mem output

### Memory Alignment

All allocations are aligned to 16 bytes for optimal performance:
```c
#define ALIGN(size) (((size) + 15) & ~15)
```

This ensures compatibility with SIMD instructions and improves cache performance.

## Building and Usage

### Compilation

```bash
# Build the library
make

# Clean object files
make clean

# Full cleanup
make fclean

# Rebuild
make re
```

The Makefile automatically:
- Detects host type (`uname -m`_`uname -s`)
- Creates library as `libft_malloc_$HOSTTYPE.so`
- Creates symbolic link `libft_malloc.so`

### Using the Library

#### Method 1: LD_PRELOAD (Linux)
```bash
export LD_PRELOAD=./libft_malloc.so
./your_program
```

#### Method 2: Direct Linking
```bash
gcc -o program program.c -L. -lft_malloc -Wl,-rpath,.
```

#### Method 3: In Code
```c
#include "malloc.h"

int main() {
    void *ptr = malloc(42);
    show_alloc_mem();  // Debug function
    free(ptr);
}
```

## Testing

### Basic Test Suite

```bash
make test
```

This runs comprehensive tests including:
- Basic allocations (various sizes)
- Free operations
- Realloc operations
- Edge cases (NULL, size 0)
- Memory write/read verification

### Manual Testing

```c
// Example test program
#include "malloc.h"
#include <string.h>

int main() {
    // Test different zone types
    char *tiny = malloc(42);      // TINY zone
    char *small = malloc(1024);   // SMALL zone  
    char *large = malloc(5000);   // LARGE zone
    
    // Verify writes work
    strcpy(tiny, "Hello");
    strcpy(small, "World");
    
    // Display memory layout
    show_alloc_mem();
    
    // Test free
    free(small);
    
    // Test realloc
    tiny = realloc(tiny, 100);
    
    // Clean up
    free(tiny);
    free(large);
    
    return 0;
}
```

## Performance Considerations

### Optimizations

1. **Pre-allocation**: Zones contain space for multiple allocations, reducing mmap calls
2. **Size classes**: Segregating by size reduces fragmentation
3. **Coalescing**: Automatic merging of free blocks maintains larger contiguous spaces
4. **Alignment**: 16-byte alignment improves CPU cache utilization

### Trade-offs

- **Memory overhead**: Zone headers and block headers consume space
- **Internal fragmentation**: Alignment and minimum block sizes waste some space
- **External fragmentation**: First-fit algorithm may not be optimal for all workloads

### Benchmarking

Compare with system malloc:
```bash
time env LD_PRELOAD=./libft_malloc.so ./benchmark_program
time ./benchmark_program  # System malloc
```

## Technical Specifications

### Constants

| Constant | Value | Description |
|----------|-------|-------------|
| TINY_MAX | 512 | Maximum size for TINY allocations |
| SMALL_MAX | 4096 | Maximum size for SMALL allocations |
| TINY_ZONE_SIZE | 16KB | Size of TINY zones |
| SMALL_ZONE_SIZE | 128KB | Size of SMALL zones |
| ALIGNMENT | 16 | Byte alignment for all allocations |

### Functions

#### Public API
- `void *malloc(size_t size)` - Allocate memory
- `void free(void *ptr)` - Deallocate memory
- `void *realloc(void *ptr, size_t size)` - Resize allocation
- `void show_alloc_mem(void)` - Display memory layout

#### Internal Functions
- Zone management: `create_zone`, `add_zone`, `remove_zone`
- Block management: `allocate_in_zone`, `split_block`, `coalesce_blocks`
- Utilities: `find_zone_for_ptr`, `zone_is_empty`

### Error Handling

- Returns NULL on allocation failure
- Silently ignores free(NULL)
- Validates pointers in free/realloc
- Never crashes on invalid operations

### Limitations

- Not thread-safe (bonus feature)
- No debugging features (bonus feature)
- First-fit allocation may not be optimal
- Fixed zone sizes may waste memory for certain workloads

## Debug Output Format

The `show_alloc_mem()` function displays:
```
TINY : 0x106ACC000
0x106ACC020 - 0x106ACC04A : 42 bytes
SMALL : 0x106ACE000
0x106ACE020 - 0x106ACE424 : 1028 bytes
LARGE : 0x106AD0000
0x106AD0020 - 0x106AD1388 : 5000 bytes
Total : 6070 bytes
```

Format: `start_address - end_address : size bytes`

## Future Improvements

1. **Thread safety**: Add mutex protection for concurrent access
2. **Better allocation strategy**: Implement best-fit or segregated free lists
3. **Debug features**: Add allocation tracking, leak detection
4. **Performance monitoring**: Add statistics collection
5. **Memory defragmentation**: Implement compaction for long-running programs

## License

This project is part of the 42 school curriculum. Feel free to use and modify for educational purposes.