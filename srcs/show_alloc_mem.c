#include "malloc.h"

static void print_zone_type(int type)
{
    if (type == TINY_ZONE)
        ft_putstr("TINY : ");
    else if (type == SMALL_ZONE)
        ft_putstr("SMALL : ");
    else
        ft_putstr("LARGE : ");
}

static size_t print_zone(t_zone *zone)
{
    t_block *block;
    char *zone_end;
    size_t total = 0;
    void *ptr;
    size_t block_size;
    
    print_zone_type(zone->type);
    print_hex((size_t)zone);
    ft_putstr("\n");
    
    zone_end = (char *)zone + zone->size;
    block = (t_block *)((char *)zone + ZONE_HEADER_SIZE);
    
    while ((char *)block < zone_end)
    {
        block_size = GET_SIZE(block->size);
        
        // Skip if we've reached the end of allocated blocks
        if (block_size == 0)
            break;
            
        // Safety check
        if ((char *)block + BLOCK_HEADER_SIZE + block_size > zone_end)
            break;
            
        if (!IS_FREE(block->size))
        {
            ptr = (char *)block + BLOCK_HEADER_SIZE;
            print_hex((size_t)ptr);
            ft_putstr(" - ");
            print_hex((size_t)ptr + block_size - 1);
            ft_putstr(" : ");
            ft_putnbr(block_size);
            ft_putstr(" bytes\n");
            total += block_size;
        }
        
        block = (t_block *)((char *)block + BLOCK_HEADER_SIZE + block_size);
    }
    
    return total;
}

void show_alloc_mem(void)
{
    t_zone *zone;
    size_t total = 0;
    
    // Print tiny zones
    zone = g_malloc_data.tiny_zones;
    while (zone)
    {
        total += print_zone(zone);
        zone = zone->next;
    }
    
    // Print small zones
    zone = g_malloc_data.small_zones;
    while (zone)
    {
        total += print_zone(zone);
        zone = zone->next;
    }
    
    // Print large zones
    zone = g_malloc_data.large_zones;
    while (zone)
    {
        total += print_zone(zone);
        zone = zone->next;
    }
    
    ft_putstr("Total : ");
    ft_putnbr(total);
    ft_putstr(" bytes\n");
}