#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

#define ITERATIONS 1000

void run_benchmark(size_t buffer_kb) {
    size_t buffer_size_bytes = buffer_kb * 1024;
    size_t element_count = buffer_size_bytes / sizeof(uint64_t);
    struct timespec start, end;
    
    uint64_t *buffer = malloc(buffer_size_bytes);
    if (!buffer) {
        printf("%7zu KB | Allocation Failed\n", buffer_kb);
        return;
    }

    srand((unsigned int)time(NULL));

    clock_gettime(CLOCK_MONOTONIC, &start);

    for (int i = 0; i < ITERATIONS; i++) {
        // Fill (element_count ops)
        for (size_t j = 0; j < element_count; j++) {
            buffer[j] = (uint64_t)j;
        }

        // Swap (3 ops)
        size_t idxA = rand() % element_count;
        size_t idxB = rand() % element_count;
        uint64_t temp = buffer[idxA];
        buffer[idxA] = buffer[idxB];
        buffer[idxB] = temp;
    }

    clock_gettime(CLOCK_MONOTONIC, &end);

    double total_time = (end.tv_sec - start.tv_sec) + 
                        (end.tv_nsec - start.tv_nsec) / 1e9;
    
    double avg_time = total_time / ITERATIONS;
    double ops_per_iteration = (double)element_count + 3;
    double total_ops = ops_per_iteration * ITERATIONS;
    double ops_per_sec = total_ops / total_time;

    printf("%7zu KB | %12.6f s | %12.9f s | %10.3e\n", 
           buffer_kb, total_time, avg_time, ops_per_sec);

    free(buffer);
}

int main() {
    printf("Buffer Size |  Total Time   |   Avg/Iter     | Throughput (ops/s)\n");
    printf("------------|----------------|----------------|-------------------\n");

    run_benchmark(64);    // 2^13 elements (L1)
    run_benchmark(512);   // 2^16 elements (L2)
    run_benchmark(4096);  // 2^19 elements (L3/RAM)
    run_benchmark(8192);  // 2^20 elements (Deep L3/RAM)

    return 0;
}
