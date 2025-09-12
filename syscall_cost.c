#define _GNU_SOURCE
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <errno.h>

static inline uint64_t usec_now(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t)tv.tv_sec * 1000000ull + (uint64_t)tv.tv_usec;
}

static inline void busy_barrier(void) {
    // 컴파일러가 빈 루프를 지워버리는 걸 방지
    asm volatile("" ::: "memory");
}

typedef enum { SC_GETPID, SC_READ0 } sc_kind;

static double measure_empty_loop_overhead(size_t iters) {
    uint64_t t0 = usec_now();
    for (size_t i = 0; i < iters; i++) {
        busy_barrier();
    }
    uint64_t t1 = usec_now();
    return (double)(t1 - t0); // usec
}

static double measure_getpid_cost(size_t iters) {
    uint64_t t0 = usec_now();
    for (size_t i = 0; i < iters; i++) {
        (void)getpid();
    }
    uint64_t t1 = usec_now();
    return (double)(t1 - t0); // usec
}

static double measure_read0_cost(size_t iters) {
    char buf;
    uint64_t t0 = usec_now();
    for (size_t i = 0; i < iters; i++) {
        // 길이 0 read: 커널을 들어갔다 바로 나오는 매우 가벼운 시스템콜
        // 표준입력(0)을 대상으로 하되, 길이가 0이라 블록되지 않음
        ssize_t r = read(0, &buf, 0);
        (void)r;
    }
    uint64_t t1 = usec_now();
    return (double)(t1 - t0); // usec
}

int main(int argc, char** argv) {
    size_t iters = 10ULL * 1000 * 1000; // 기본 1천만 회
    sc_kind kind = SC_GETPID;

    // 사용법: ./syscall_cost [iters] [getpid|read0]
    if (argc >= 2) {
        iters = strtoull(argv[1], NULL, 10);
        if (iters == 0) {
            fprintf(stderr, "invalid iters\n");
            return 1;
        }
    }
    if (argc >= 3) {
        if (strcmp(argv[2], "read0") == 0) kind = SC_READ0;
        else if (strcmp(argv[2], "getpid") == 0) kind = SC_GETPID;
        else {
            fprintf(stderr, "unknown kind: %s\n", argv[2]);
            return 1;
        }
    }

    // 워밍업(캐시/페이지 폴트 잡음 감소)
    for (int w = 0; w < 1000000; w++) busy_barrier();

    double empty_us = measure_empty_loop_overhead(iters);
    double total_us;

    if (kind == SC_GETPID) total_us = measure_getpid_cost(iters);
    else                   total_us = measure_read0_cost(iters);

    double syscall_us = total_us - empty_us;
    if (syscall_us < 0) syscall_us = 0; // 숫자 잡음 보정

    double per_call_us = syscall_us / (double)iters;
    double per_call_ns = per_call_us * 1000.0;

    printf("iters           : %zu\n", iters);
    printf("empty loop (us) : %.3f\n", empty_us);
    printf("total (us)      : %.3f\n", total_us);
    printf("syscall (us)    : %.3f (empty-subtracted)\n", syscall_us);
    printf("avg per call    : %.3f ns\n", per_call_ns);
    printf("kind            : %s\n", kind == SC_GETPID ? "getpid()" : "read(0,buf,0)\n");
    return 0;
}
