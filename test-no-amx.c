#define _GNU_SOURCE
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <linux/perf_event.h>
#include <asm/unistd.h>

#define MAX_ROWS 16
#define MAX_COLS 64


// Wrapper for perf_event_open
static long perf_event_open(struct perf_event_attr *hw_event, pid_t pid,int cpu, int group_fd, unsigned long flags)
{
    return syscall(__NR_perf_event_open, hw_event, pid, cpu,
                   group_fd, flags);
}

int main() { 
    // --- Setup PMC ---
struct perf_event_attr pe, pea;
memset(&pe, 0, sizeof(pe));
memset(&pea, 0, sizeof(pea));

pe.type = PERF_TYPE_HARDWARE;
pe.size = sizeof(pe);
pe.config = PERF_COUNT_HW_CPU_CYCLES;
pe.disabled = 1;
pe.exclude_kernel = 1;
pe.exclude_hv = 1;

pea.type = PERF_TYPE_RAW;
pea.size = sizeof(pea);
pea.config = 0x6B2; // EXE.AMX_BUSY (check your CPU)
pea.disabled = 1;
pea.exclude_kernel = 1;
pea.exclude_hv = 1;

int fd_cycles = perf_event_open(&pe, 0, -1, -1, 0);
int fd_amx   = perf_event_open(&pea, 0, -1, -1, 0);

    int rowsA = MAX_ROWS;
    int colsA = MAX_COLS;
    int rowsB = MAX_COLS;  // 64
    int colsB = MAX_ROWS;  // 16

    int8_t A[MAX_ROWS][MAX_COLS];
    int8_t B[MAX_COLS][MAX_ROWS];
    int32_t C[MAX_ROWS][MAX_ROWS];

    uint64_t count,ct;

    // Start counter
    ioctl(fd_cycles, PERF_EVENT_IOC_RESET, 0);
    ioctl(fd_cycles, PERF_EVENT_IOC_ENABLE, 0);

   for (int i = 0; i < rowsA; i++)
        for (int j = 0; j < colsA; j++)
            A[i][j] = 2;

    for (int i = 0; i < rowsB; i++)
        for (int j = 0; j < colsB; j++)
            B[i][j] = 2;

    // Initialize result with 0
    for (int i = 0; i < rowsA; i++)
        for (int j = 0; j < colsB; j++)
            C[i][j] = 0;

    // Standard matrix multiplication
    for (int i = 0; i < rowsA; i++) {
        for (int j = 0; j < colsB; j++) {
            int32_t sum = 0;
            for (int k = 0; k < colsA; k++) {
                sum += (int32_t)A[i][k] * (int32_t)B[k][j];
            }
            C[i][j] = sum;
        }
    }


    // Stop counter
    ioctl(fd_cycles, PERF_EVENT_IOC_DISABLE, 0);
    read(fd_cycles, &count, sizeof(uint64_t));
    read(fd_amx, &ct, sizeof(uint64_t));
    

    
    printf("Total Cycles: %lld\n",count);
    printf("AMX cycles: %lld\n",ct);
    close(fd_cycles);
    close(fd_amx);
    return 0;
}

