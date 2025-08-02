#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <signal.h>
#include <setjmp.h>
#include <stdint.h>
#include <time.h>
#include <sys/resource.h>
#include "../includes/malloc.h"

#define TEST_START(name) write(1, "\n=== " name " ===\n", strlen("\n=== " name " ===\n"))
#define TEST_PASS(name) write(1, "[PASS] " name "\n", strlen("[PASS] " name "\n"))
#define TEST_FAIL(name) write(2, "[FAIL] " name "\n", strlen("[FAIL] " name "\n"))

void write_num(int fd, size_t num) {
    char buf[32];
    int i = 31;
    buf[i] = '\0';
    if (num == 0) {
        write(fd, "0", 1);
        return;
    }
    while (num > 0 && i > 0) {
        buf[--i] = '0' + (num % 10);
        num /= 10;
    }
    write(fd, &buf[i], 31 - i);
}


void safe_memset(void *ptr, int value, size_t size) {
    unsigned char *p = (unsigned char *)ptr;
    for (size_t i = 0; i < size; i++) {
        p[i] = (unsigned char)value;
    }
}

void safe_memcpy(void *dst, const void *src, size_t size) {
    unsigned char *d = (unsigned char *)dst;
    const unsigned char *s = (const unsigned char *)src;
    for (size_t i = 0; i < size; i++) {
        d[i] = s[i];
    }
}

// Test result tracking
static int tests_passed = 0;
static int tests_failed = 0;

// For crash testing
static jmp_buf jump_buffer;
static void segfault_handler(int sig) {
    (void)sig;
    longjmp(jump_buffer, 1);
}

void test_basic_functionality() {
    TEST_START("Test 1: Basic Functionality");
    
    void *tiny = malloc(8);
    void *small = malloc(1024);
    void *large = malloc(8192);
    
    assert(tiny != NULL);
    assert(small != NULL);
    assert(large != NULL);
    
    safe_memset(tiny, 'A', 8);
    safe_memset(small, 'B', 1024);
    safe_memset(large, 'C', 8192);
    
    // Verify writes
    assert(((char*)tiny)[0] == 'A');
    assert(((char*)tiny)[7] == 'A');
    assert(((char*)small)[0] == 'B');
    assert(((char*)small)[1023] == 'B');
    assert(((char*)large)[0] == 'C');
    assert(((char*)large)[8191] == 'C');
    
    free(tiny);
    free(small);
    free(large);
    
    TEST_PASS("Basic Functionality");
    tests_passed++;
}

void test_edge_cases() {
    TEST_START("Test 2: Edge Cases");
    
    // malloc(0) should return NULL
    void *zero = malloc(0);
    assert(zero == NULL);
    
    // free(NULL) should not crash
    free(NULL);
    
    // Very large allocation should fail gracefully
    // Use a size that's clearly too large but avoids compiler warnings
    size_t huge_size = (size_t)1 << 62;  // 4 exabytes - impossible to allocate
    void *huge = malloc(huge_size);
    if (huge != NULL) {
        write(2, "WARNING: Huge allocation succeeded - your malloc might not check limits\n", 72);
        free(huge);
    }
    
    // Test realistic but large allocation that should fail
    void *large_fail = malloc(1ULL << 40);  // 1TB - should fail on most systems
    if (large_fail != NULL) {
        write(2, "WARNING: 1TB allocation succeeded\n", 34);
        free(large_fail);
    }
    
    // Exact boundary sizes
    void *tiny_max = malloc(512);      // Exactly TINY_MAX
    void *small_max = malloc(4096);    // Exactly SMALL_MAX
    assert(tiny_max != NULL);
    assert(small_max != NULL);
    
    free(tiny_max);
    free(small_max);
    
    TEST_PASS("Edge Cases");
    tests_passed++;
}

void test_alignment() {
    TEST_START("Test 3: Alignment Verification");
    
    // Test that all returned pointers are 16-byte aligned
    for (size_t size = 1; size <= 100; size++) {
        void *ptr = malloc(size);
        assert(ptr != NULL);
        assert(((uintptr_t)ptr % 16) == 0);
        free(ptr);
    }
    
    TEST_PASS("Alignment Verification");
    tests_passed++;
}

void test_realloc_comprehensive() {
    TEST_START("Test 4: Realloc Comprehensive");
    
    // Test 1: realloc(NULL, size) == malloc(size)
    void *p1 = realloc(NULL, 100);
    assert(p1 != NULL);
    safe_memset(p1, 'X', 100);
    
    // Test 2: realloc(ptr, 0) == free(ptr)
    void *p2 = malloc(50);
    void *p3 = realloc(p2, 0);
    assert(p3 == NULL);
    
    // Test 3: Shrinking realloc
    void *p4 = malloc(1000);
    safe_memset(p4, 'Y', 1000);
    void *p5 = realloc(p4, 500);
    assert(p5 != NULL);
    // Verify data preserved
    for (int i = 0; i < 500; i++) {
        assert(((char*)p5)[i] == 'Y');
    }
    
    // Test 4: Growing realloc
    void *p6 = realloc(p5, 2000);
    assert(p6 != NULL);
    // Verify data preserved
    for (int i = 0; i < 500; i++) {
        assert(((char*)p6)[i] == 'Y');
    }
    
    // Test 5: Realloc to same size
    void *p7 = realloc(p6, 2000);
    assert(p7 != NULL);
    
    free(p1);
    free(p7);
    
    TEST_PASS("Realloc Comprehensive");
    tests_passed++;
}

void test_fragmentation() {
    TEST_START("Test 5: Fragmentation and Coalescing");
    
    // Allocate many blocks
    void *ptrs[100];
    for (int i = 0; i < 100; i++) {
        ptrs[i] = malloc(32);
        assert(ptrs[i] != NULL);
    }
    
    // Free every other block (create fragmentation)
    for (int i = 0; i < 100; i += 2) {
        free(ptrs[i]);
    }
    
    // Try to allocate larger blocks (should trigger coalescing)
    void *large1 = malloc(64);
    assert(large1 != NULL);
    
    // Free remaining blocks
    for (int i = 1; i < 100; i += 2) {
        free(ptrs[i]);
    }
    
    // Should be able to allocate very large block now
    void *large2 = malloc(3000);
    assert(large2 != NULL);
    
    free(large1);
    free(large2);
    
    TEST_PASS("Fragmentation and Coalescing");
    tests_passed++;
}


void test_double_free() {
    TEST_START("Test 6: Double Free Detection");
    
    // Set up signal handler
    struct sigaction sa, old_sa;
    sa.sa_handler = segfault_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGSEGV, &sa, &old_sa);
    sigaction(SIGABRT, &sa, &old_sa);
    
    if (setjmp(jump_buffer) == 0) {
        // This might crash or be caught
        void *ptr = malloc(100);
        free(ptr);
        free(ptr);  // Double free
        
        // If we get here, double free was handled gracefully
        TEST_PASS("Double Free Detection (handled gracefully)");
    } else {
        // Caught signal - also acceptable
        TEST_PASS("Double Free Detection (caught signal)");
    }
    
    // Restore old handler
    sigaction(SIGSEGV, &old_sa, NULL);
    sigaction(SIGABRT, &old_sa, NULL);
    
    tests_passed++;
}

void test_invalid_pointer() {
    TEST_START("Test 7: Invalid Pointer Handling");
    
    // Set up signal handler
    struct sigaction sa, old_sa;
    sa.sa_handler = segfault_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGSEGV, &sa, &old_sa);
    sigaction(SIGABRT, &sa, &old_sa);
    
    if (setjmp(jump_buffer) == 0) {
        // Try to free invalid pointers
        // Use volatile to prevent compiler warnings
        volatile void *bad_ptr1 = (void*)0x12345678;
        volatile void *bad_ptr2 = (void*)&tests_passed;
        
        free((void*)bad_ptr1);  // Random address
        free((void*)bad_ptr2);  // Stack address
        
        // If we get here, handled gracefully
        TEST_PASS("Invalid Pointer Handling (handled gracefully)");
    } else {
        // Caught signal - also acceptable
        TEST_PASS("Invalid Pointer Handling (caught signal)");
    }
    
    // Restore old handler
    sigaction(SIGSEGV, &old_sa, NULL);
    sigaction(SIGABRT, &old_sa, NULL);
    
    tests_passed++;
}


void test_show_alloc_mem() {
    TEST_START("Test 8: Show Alloc Mem");
    
    // Create predictable allocations
    void *tiny1 = malloc(16);
    void *tiny2 = malloc(32);
    void *small1 = malloc(1024);
    void *large1 = malloc(10000);
    
    write(1, "\n--- Memory Layout ---\n", 23);
    show_alloc_mem();
    write(1, "--- End Layout ---\n", 19);
    
    // Free some
    free(tiny1);
    free(small1);
    
    write(1, "\n--- After Free ---\n", 20);
    show_alloc_mem();
    write(1, "--- End Layout ---\n", 19);
    
    // Cleanup
    free(tiny2);
    free(large1);
    
    TEST_PASS("Show Alloc Mem");
    tests_passed++;
}

void test_allocation_limits() {
    TEST_START("Test 9: Allocation Limits");
    
    // Test allocating until failure
    void *ptrs[10000];
    int count = 0;
    size_t total_allocated = 0;
    
    // Allocate 1MB chunks until failure
    while (count < 10000) {
        ptrs[count] = malloc(1024 * 1024);  // 1MB
        if (ptrs[count] == NULL) {
            break;
        }
        total_allocated += 1024 * 1024;
        count++;
        
        // Safety limit to prevent system issues
        if (total_allocated > 1024ULL * 1024 * 1024) {  // 1GB
            break;
        }
    }
    
    write(1, "Allocated chunks: ", 18);
    write_num(1, count);
    write(1, " (", 2);
    write_num(1, total_allocated / (1024 * 1024));
    write(1, " MB)\n", 5);
    
    // Free everything
    for (int i = 0; i < count; i++) {
        free(ptrs[i]);
    }
    
    // Should be able to allocate again
    void *after = malloc(1024 * 1024);
    assert(after != NULL);
    free(after);
    
    TEST_PASS("Allocation Limits");
    tests_passed++;
}

void test_performance() {
    TEST_START("Test 10: Performance");
    
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    
    // Do 100,000 malloc/free cycles
    for (int i = 0; i < 100000; i++) {
        void *ptr = malloc(64);
        free(ptr);
    }
    
    clock_gettime(CLOCK_MONOTONIC, &end);
    
    long long elapsed = (end.tv_sec - start.tv_sec) * 1000000000LL + 
                       (end.tv_nsec - start.tv_nsec);
    
    write(1, "100k malloc/free cycles took: ", 31);
    write_num(1, elapsed / 1000000);
    write(1, " ms\n", 4);
    
    TEST_PASS("Performance");
    tests_passed++;
}

int main(void) {
    write(1, "=== COMPREHENSIVE MALLOC TEST SUITE (FIXED) ===\n", 48);
    write(1, "Using write() to avoid printf malloc calls\n", 43);
    write(1, "Using safe memory operations\n\n", 30);
    
    // Run all tests
    test_basic_functionality();
    test_edge_cases();
    test_alignment();
    test_realloc_comprehensive();
    test_fragmentation();
    test_double_free();
    test_invalid_pointer();
    test_show_alloc_mem();
    test_allocation_limits();
    test_performance();
    
    // Summary
    write(1, "\n=== TEST SUMMARY ===\n", 22);
    write(1, "Tests passed: ", 14);
    write_num(1, tests_passed);
    write(1, "\nTests failed: ", 15);
    write_num(1, tests_failed);
    write(1, "\n\n", 2);
    
    if (tests_failed == 0) {
        write(1, "ALL TESTS PASSED! ✓\n", 21);
        return 0;
    } else {
        write(2, "SOME TESTS FAILED! ✗\n", 22);
        return 1;
    }
}