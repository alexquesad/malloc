#include "malloc.h"

t_zone *create_zone(size_t size, int type)
{
    t_zone *zone;
    t_block *block;
    size_t usable_size;
    
    zone = mmap(NULL, size, PROT_READ | PROT_WRITE, 
                MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (zone == MAP_FAILED)
        return NULL;
    
    zone->next = NULL;
    zone->prev = NULL;
    zone->size = size;
    zone->type = type;
    zone->free_blocks = 1;
    
    // Calculate usable size (total size minus zone header)
    usable_size = size - ZONE_HEADER_SIZE - BLOCK_HEADER_SIZE;
    zone->free_size = usable_size;
    
    // Initialize first block
    block = (t_block *)((char *)zone + ZONE_HEADER_SIZE);
    block->size = SET_FREE(usable_size);
    block->prev_size = 0;
    
    return zone;
}

void add_zone(t_zone **list, t_zone *zone)
{
    t_zone *current;
    
    if (!*list || zone < *list)
    {
        zone->next = *list;
        if (*list)
            (*list)->prev = zone;
        *list = zone;
        return;
    }
    
    current = *list;
    while (current->next && current->next < zone)
        current = current->next;
    
    zone->next = current->next;
    zone->prev = current;
    if (current->next)
        current->next->prev = zone;
    current->next = zone;
}

void remove_zone(t_zone **list, t_zone *zone)
{
    if (zone->prev)
        zone->prev->next = zone->next;
    else
        *list = zone->next;
    
    if (zone->next)
        zone->next->prev = zone->prev;
}

int zone_is_empty(t_zone *zone)
{
    return zone->free_size == zone->size - ZONE_HEADER_SIZE - BLOCK_HEADER_SIZE;
}

t_zone *find_zone_for_ptr(void *ptr)
{
    t_zone *zone;
    
    zone = g_malloc_data.tiny_zones;
    while (zone)
    {
        if (ptr >= (void *)zone && ptr < (void *)((char *)zone + zone->size))
            return zone;
        zone = zone->next;
    }
    
    zone = g_malloc_data.small_zones;
    while (zone)
    {
        if (ptr >= (void *)zone && ptr < (void *)((char *)zone + zone->size))
            return zone;
        zone = zone->next;
    }
    
    zone = g_malloc_data.large_zones;
    while (zone)
    {
        if (ptr >= (void *)zone && ptr < (void *)((char *)zone + zone->size))
            return zone;
        zone = zone->next;
    }
    
    return NULL;
}