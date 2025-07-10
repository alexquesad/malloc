#ifndef MALLOC_H
# define MALLOC_H

# include <sys/mman.h>
# include <unistd.h>
# include <stddef.h>
# include <stdint.h>

// Zone types
# define TINY_ZONE      0
# define SMALL_ZONE     1
# define LARGE_ZONE     2

// Size limits
# define TINY_MAX       512
# define SMALL_MAX      4096

// Zone sizes (must contain at least 100 allocations)
# define TINY_ZONE_SIZE     (getpagesize() * 4)    // 16KB
# define SMALL_ZONE_SIZE    (getpagesize() * 32)   // 128KB

// Alignment
# define ALIGNMENT      16
# define ALIGN(size)    (((size) + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1))

// Headers
# define ZONE_HEADER_SIZE   sizeof(t_zone)
# define BLOCK_HEADER_SIZE  sizeof(t_block)

// Free block flag (stored in size LSB)
# define BLOCK_FREE     0x1
# define GET_SIZE(s)    ((s) & ~0x7)
# define IS_FREE(s)     ((s) & BLOCK_FREE)
# define SET_FREE(s)    ((s) | BLOCK_FREE)
# define CLEAR_FREE(s)  ((s) & ~BLOCK_FREE)

typedef struct s_block {
    size_t      size;       // Size with flags in lower bits
    size_t      prev_size;  // Size of previous block
} t_block;

typedef struct s_zone {
    struct s_zone   *next;
    struct s_zone   *prev;
    size_t          size;
    int             type;
    size_t          free_blocks;
    size_t          free_size;
} t_zone;

typedef struct s_malloc_data {
    t_zone          *tiny_zones;
    t_zone          *small_zones;
    t_zone          *large_zones;
} t_malloc_data;

// Global allocator data
extern t_malloc_data g_malloc_data;

// Public functions
void    free(void *ptr);
void    *malloc(size_t size);
void    *realloc(void *ptr, size_t size);
void    show_alloc_mem(void);

// Internal functions
t_zone  *create_zone(size_t size, int type);
void    *allocate_in_zone(t_zone *zone, size_t size);
t_block *find_free_block(t_zone *zone, size_t size);
void    split_block(t_block *block, size_t size);
void    coalesce_blocks(t_zone *zone, t_block *block);
int     zone_is_empty(t_zone *zone);
void    remove_zone(t_zone **list, t_zone *zone);
void    add_zone(t_zone **list, t_zone *zone);
t_zone  *find_zone_for_ptr(void *ptr);
void    *ft_memcpy(void *dst, const void *src, size_t n);
void    ft_bzero(void *s, size_t n);
void    ft_putstr(const char *s);
void    ft_putnbr(size_t n);
void    ft_putchar(char c);
void    print_hex(size_t n);

#endif