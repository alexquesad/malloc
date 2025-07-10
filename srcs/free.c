#include "malloc.h"

static void try_munmap_zone(t_zone *zone)
{
    t_zone **list;
    
    if (!zone_is_empty(zone))
        return;
    
    // Determine which list this zone belongs to
    if (zone->type == TINY_ZONE)
        list = &g_malloc_data.tiny_zones;
    else if (zone->type == SMALL_ZONE)
        list = &g_malloc_data.small_zones;
    else
        list = &g_malloc_data.large_zones;
    
    remove_zone(list, zone);
    munmap(zone, zone->size);
}

void free(void *ptr)
{
    t_zone *zone;
    t_block *block;
    
    if (!ptr)
        return;
    
    zone = find_zone_for_ptr(ptr);
    if (!zone)
        return;
    
    block = (t_block *)((char *)ptr - BLOCK_HEADER_SIZE);
    
    // Check if already free
    if (IS_FREE(block->size))
        return;
    
    // Mark block as free
    block->size = SET_FREE(block->size);
    zone->free_blocks++;
    zone->free_size += GET_SIZE(block->size);
    
    // Coalesce with adjacent free blocks
    coalesce_blocks(zone, block);
    
    // For large zones, unmap immediately
    if (zone->type == LARGE_ZONE)
        try_munmap_zone(zone);
}