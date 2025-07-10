#include "malloc.h"

t_malloc_data g_malloc_data = {NULL, NULL, NULL};

static int get_zone_type(size_t size)
{
    if (size <= TINY_MAX)
        return TINY_ZONE;
    else if (size <= SMALL_MAX)
        return SMALL_ZONE;
    return LARGE_ZONE;
}

static t_zone **get_zone_list(int type)
{
    if (type == TINY_ZONE)
        return &g_malloc_data.tiny_zones;
    else if (type == SMALL_ZONE)
        return &g_malloc_data.small_zones;
    return &g_malloc_data.large_zones;
}

static size_t get_zone_size(int type, size_t size)
{
    if (type == TINY_ZONE)
        return TINY_ZONE_SIZE;
    else if (type == SMALL_ZONE)
        return SMALL_ZONE_SIZE;
    // For large allocations, allocate exact size + headers
    return ALIGN(size + ZONE_HEADER_SIZE + BLOCK_HEADER_SIZE);
}

void *malloc(size_t size)
{
    void *ptr;
    int type;
    t_zone **zone_list;
    t_zone *zone;
    
    if (size == 0)
        return NULL;
    
    // Align size
    size = ALIGN(size);
    
    type = get_zone_type(size);
    zone_list = get_zone_list(type);
    
    // Try to find space in existing zones
    zone = *zone_list;
    while (zone && type != LARGE_ZONE)
    {
        ptr = allocate_in_zone(zone, size);
        if (ptr)
            return ptr;
        zone = zone->next;
    }
    
    // Create new zone
    zone = create_zone(get_zone_size(type, size), type);
    if (!zone)
        return NULL;
    
    add_zone(zone_list, zone);
    ptr = allocate_in_zone(zone, size);
    
    return ptr;
}