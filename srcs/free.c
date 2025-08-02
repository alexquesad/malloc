#include "malloc.h"

void free(void *ptr)
{
    t_zone *zone;
    t_block *block;
    
    if (!ptr)
        return;
    
    zone = find_zone_for_ptr(ptr);
    if (!zone)
        return;
    
    block = (t_block *)((char *)ptr - sizeof(t_block));
    
    // Check if already free
    if (IS_FREE(block->size))
        return;
    
    // Mark block as free
    block->size = SET_FREE(block->size);
    zone->free_blocks++;
    zone->free_size += GET_SIZE(block->size);
    
    // Coalesce with adjacent free blocks
    coalesce_blocks(zone, block);
    
    // For large zones, always unmap immediately
    if (zone->type == LARGE_ZONE) {
        remove_zone(&g_malloc_data.large_zones, zone);
        munmap(zone, zone->size);
        return;
    }
    
    // For TINY and SMALL zones, only unmap if empty AND not the only zone
    if (zone_is_empty(zone)) {
        t_zone **zone_list;
        t_zone *head;
        
        if (zone->type == TINY_ZONE) {
            zone_list = &g_malloc_data.tiny_zones;
            head = g_malloc_data.tiny_zones;
        } else {  // SMALL_ZONE
            zone_list = &g_malloc_data.small_zones;
            head = g_malloc_data.small_zones;
        }
        
        // Only unmap if it's not the last zone of its type
        // This prevents thrashing (constantly creating/destroying zones)
        if (head != zone || zone->next != NULL) {
            remove_zone(zone_list, zone);
            munmap(zone, zone->size);
        }
    }
}