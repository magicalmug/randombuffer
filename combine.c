#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define ITERATIONS 100
#define BUCKET_COUNT 1024

typedef struct Node {
    uint64_t value;
    struct Node* next;
} Node;

// --- TEST 1: RANDOM SWAP ---
void do_swap_test(uint64_t* buffer, size_t n) {
    size_t idxA = rand() % n;
    size_t idxB = rand() % n;
    uint64_t temp = buffer[idxA];
    buffer[idxA] = buffer[idxB];
    buffer[idxB] = temp;
}

// --- TEST 2: BUCKET SORT ---
void do_bucket_sort(uint64_t* buffer, size_t n) {
    Node* buckets[BUCKET_COUNT] = {NULL};
    uint64_t max_val = 0;
    for(size_t i=0; i<n; i++) if(buffer[i] > max_val) max_val = buffer[i];
    for (size_t i = 0; i < n; i++) {
        int b_idx = (buffer[i] * BUCKET_COUNT) / (max_val + 1);
        Node* new_node = malloc(sizeof(Node));
        new_node->value = buffer[i];
        new_node->next = buckets[b_idx];
        buckets[b_idx] = new_node;
    }
    size_t idx = 0;
    for (int i = 0; i < BUCKET_COUNT; i++) {
        Node* curr = buckets[i];
        while (curr) {
            buffer[idx++] = curr->value;
            Node* temp = curr;
            curr = curr->next;
            free(temp);
        }
    }
}

// --- TEST 3: CHUNK ---
void do_chunk_test(uint64_t* buffer, size_t n) {
    size_t idx = rand() % n;
    buffer[idx] = ((uint64_t)rand() << 32) | (uint64_t)rand();
}

// --- TEST 4: POINTER CHASING (New) ---
// Logic: Forces the CPU to wait for the memory result before it can find the next address.
void do_pointer_chase(uint64_t* buffer, size_t n) {
    size_t curr = 0;
    // We traverse 'n' elements so that every element is visited exactly once per iteration
    for (size_t i = 0; i < n; i++) {
        curr = (size_t)buffer[curr];
    }
    // Volatile sink prevents the compiler from optimizing away the loop
    static volatile size_t sink;
    sink = curr;
}

// Helper to create a random cyclic path through the buffer
void init_pointer_chain(uint64_t* buffer, size_t n) {
    for (size_t i = 0; i < n; i++) buffer[i] = i;
    // Fisher-Yates shuffle
    for (size_t i = n - 1; i > 0; i--) {
        size_t j = rand() % (i + 1);
        uint64_t temp = buffer[i];
        buffer[i] = buffer[j];
        buffer[j] = temp;
    }
}

// --- BENCHMARK ENGINE ---
void run_benchmark(size_t buffer_kb, const char* mode) {
    size_t buffer_size_bytes = buffer_kb * 1024;
    size_t n = buffer_size_bytes / sizeof(uint64_t);
    struct timespec start, end;
    
    uint64_t *buffer = malloc(buffer_size_bytes);
    if (!buffer) return;

    srand((unsigned int)time(NULL));

    // SPECIAL SETUP: Chase needs a pre-linked chain
    if (strcmp(mode, "chase") == 0) {
        init_pointer_chain(buffer, n);
    }

    clock_gettime(CLOCK_MONOTONIC, &start);

    for (int i = 0; i < ITERATIONS; i++) {
        // Only re-fill the buffer for non-chasing tests
        if (strcmp(mode, "chase") != 0) {
            for (size_t j = 0; j < n; j++) {
                buffer[j] = ((uint64_t)rand() << 32) | (uint64_t)rand();
            }
        }

        if (strcmp(mode, "swap") == 0) do_swap_test(buffer, n);
        else if (strcmp(mode, "chunk") == 0) do_chunk_test(buffer, n);
        else if (strcmp(mode, "bucket") == 0) do_bucket_sort(buffer, n);
        else if (strcmp(mode, "chase") == 0) do_pointer_chase(buffer, n);
    }

    clock_gettime(CLOCK_MONOTONIC, &end);

    double total_time = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    double ops_per_sec = ((double)n * ITERATIONS) / total_time;

    printf("%7zu KB | %10s | %10.4f s | %10.7f s | %10.2e\n", 
           buffer_kb, mode, total_time, total_time / ITERATIONS, ops_per_sec);

    free(buffer);
}

void print_help() {
    printf("Options:\n");
    printf("  swap   - Random 64-bit element swaps\n");
    printf("  chunk  - Random 64-bit element replacement\n");
    printf("  bucket - Full bucket sort\n");
    printf("  chase  - Pointer chasing latency test (New)\n");
    printf("  all    - Runs all tests sequentially\n\n");
}

int main(int argc, char *argv[]) {
    if (argc < 2) { print_help(); return 1; }

    char* choice = argv[1];
    int run_swap = (strcmp(choice, "swap") == 0 || strcmp(choice, "all") == 0);
    int run_chunk = (strcmp(choice, "chunk") == 0 || strcmp(choice, "all") == 0);
    int run_bucket = (strcmp(choice, "bucket") == 0 || strcmp(choice, "all") == 0);
    int run_chase = (strcmp(choice, "chase") == 0 || strcmp(choice, "all") == 0);

    if (!run_swap && !run_chunk && !run_bucket && !run_chase) {
        print_help(); return 1;
    }

    printf("Buffer Size |    Test    | Total Time |   Avg/Iter   | Ops/sec\n");
    printf("------------|------------|------------|------------|-----------\n");

    size_t sizes[] = {64, 512, 4096, 8192};
    for(int i = 0; i < 4; i++) {
        if (run_swap)   run_benchmark(sizes[i], "swap");
        if (run_chunk)  run_benchmark(sizes[i], "chunk");
        if (run_bucket) run_benchmark(sizes[i], "bucket");
        if (run_chase)  run_benchmark(sizes[i], "chase");
        if (strcmp(choice, "all") == 0) printf("------------|------------|------------|------------|-----------\n");
    }

    return 0;
}
