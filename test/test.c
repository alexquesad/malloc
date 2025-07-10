// test/test.c
#include <stdio.h>
#include <string.h>
#include "../includes/malloc.h"

int main()
{
    printf("=== Test 1: Basic allocations ===\n");
    char *str1 = malloc(42);
    char *str2 = malloc(1024);
    char *str3 = malloc(5000);
    
    // Test that we can write to the allocated memory
    if (str1) strcpy(str1, "Hello");
    if (str2) strcpy(str2, "World");
    if (str3) strcpy(str3, "Testing malloc implementation");
    
    show_alloc_mem();
    
    printf("\n=== Test 2: Free and realloc ===\n");
    free(str2);
    
    char *str4 = realloc(str1, 84);
    if (str4) strcpy(str4, "Reallocated string");
    
    show_alloc_mem();
    
    printf("\n=== Test 3: Edge cases ===\n");
    void *null_malloc = malloc(0);
    printf("malloc(0) returned: %p\n", null_malloc);
    
    void *null_realloc = realloc(NULL, 100);
    printf("realloc(NULL, 100) returned: %p\n", null_realloc);
    
    free(NULL);  // Should not crash
    printf("free(NULL) completed without crash\n");
    
    // Clean up
    free(str3);
    free(str4);
    free(null_realloc);
    
    printf("\n=== Test completed successfully ===\n");
    return 0;
}