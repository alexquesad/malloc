#include "malloc.h"

void *allocate_in_zone(t_zone *zone, size_t size)
{
    t_block *block;
    char *zone_end;
    size_t block_size;
    size_t actual_size;
    
    zone_end = (char *)zone + zone->size;
    block = (t_block *)((char *)zone + ZONE_HEADER_SIZE);
    
    while ((char *)block < zone_end)
    {
        block_size = GET_SIZE(block->size);
        
        // Safety check
        if (block_size == 0 || (char *)block + BLOCK_HEADER_SIZE + block_size > zone_end)
            break;
            
        if (IS_FREE(block->size) && block_size >= size)
        {
            // Remember the actual size we'll use
            actual_size = size;
            
            // Split the block if necessary
            split_block(block, size);
            
            // Check if split happened
            if (block_size > size + BLOCK_HEADER_SIZE + ALIGNMENT)
            {
                // Split happened, use requested size
                block->size = size;  // No FREE bit, just size
            }
            else
            {
                // No split, use entire block
                block->size = block_size;  // No FREE bit, just size
                actual_size = block_size;
            }
            
            zone->free_blocks--;
            zone->free_size -= actual_size;
            return (char *)block + BLOCK_HEADER_SIZE;
        }
        
        // Move to next block
        block = (t_block *)((char *)block + BLOCK_HEADER_SIZE + block_size);
    }
    
    return NULL;
}

void split_block(t_block *block, size_t size)
{
    t_block *new_block;
    size_t block_size;
    size_t remaining;
    
    block_size = GET_SIZE(block->size);
    
    // Check if we have enough space to split
    if (block_size <= size + BLOCK_HEADER_SIZE + ALIGNMENT)
        return;
        
    remaining = block_size - size - BLOCK_HEADER_SIZE;
    
    // Create new free block
    new_block = (t_block *)((char *)block + BLOCK_HEADER_SIZE + size);
    new_block->size = SET_FREE(remaining);
    new_block->prev_size = size;
    
    // Update next block's prev_size if it exists
    t_block *next = (t_block *)((char *)new_block + BLOCK_HEADER_SIZE + remaining);
    if ((char *)next < (char *)block + BLOCK_HEADER_SIZE + block_size)
        next->prev_size = remaining;
}

void coalesce_blocks(t_zone *zone, t_block *block)
{
    t_block *next;
    t_block *prev;
    char *zone_start;
    size_t total_size;
    
    zone_start = (char *)zone + ZONE_HEADER_SIZE;
    total_size = GET_SIZE(block->size);
    
    // Try to coalesce with next block
    next = (t_block *)((char *)block + BLOCK_HEADER_SIZE + GET_SIZE(block->size));
    if ((char *)next < (char *)zone + zone->size && IS_FREE(next->size))
    {
        total_size += BLOCK_HEADER_SIZE + GET_SIZE(next->size);
        zone->free_blocks--;
    }
    
    // Try to coalesce with previous block
    if (block->prev_size > 0 && (char *)block > zone_start)
    {
        prev = (t_block *)((char *)block - BLOCK_HEADER_SIZE - block->prev_size);
        if (IS_FREE(prev->size))
        {
            total_size += BLOCK_HEADER_SIZE + GET_SIZE(prev->size);
            block = prev;
            zone->free_blocks--;
        }
    }
    
    // Update the coalesced block
    block->size = SET_FREE(total_size);
    
    // Update next block's prev_size
    next = (t_block *)((char *)block + BLOCK_HEADER_SIZE + total_size);
    if ((char *)next < (char *)zone + zone->size)
        next->prev_size = total_size;
}