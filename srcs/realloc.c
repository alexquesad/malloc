#include "malloc.h"

void *realloc(void *ptr, size_t size)
{
    void *new_ptr;
    t_zone *zone;
    t_block *block;
    size_t old_size;
    
    if (!ptr)
        return malloc(size);
    
    if (size == 0)
    {
        free(ptr);
        return NULL;
    }
    
    zone = find_zone_for_ptr(ptr);
    if (!zone)
        return NULL;
    
    block = (t_block *)((char *)ptr - BLOCK_HEADER_SIZE);
    old_size = GET_SIZE(block->size);
    
    // If new size fits in current block, return same pointer
    if (ALIGN(size) <= old_size)
        return ptr;
    
    // Allocate new block
    new_ptr = malloc(size);
    if (!new_ptr)
        return NULL;
    
    // Copy old data
    ft_memcpy(new_ptr, ptr, old_size < size ? old_size : size);
    
    // Free old block
    free(ptr);
    
    return new_ptr;
}