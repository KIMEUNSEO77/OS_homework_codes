#define _GNU_SOURCE
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <sched.h>
#include <sys/time.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>

static inline uint64_t usec_now(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t)tv.tv_sec * 1000000ull + (uint64_t)tv.tv_usec;
}

static void pin_to_cpu0_or_exit(const char* who) {
    cpu_set_t set;
    CPU_ZERO(&set);
    CPU_SET(0, &set);
    if (sched_setaffinity(0, sizeof(set), &set) != 0) {
        perror("sched_setaffinity");
        fprintf(stderr, "[%s] failed to pin to CPU0\n", who);
        exit(1);
    }
}

int main(int argc, char** argv) {
    long iters = 100000; // 기본 왕복 10만 번
    if (argc >= 2) {
        iters = strtol(argv[1], NULL, 10);
        if (iters <= 0) {
            fprintf(stderr, "invalid iters\n");
            return 1;
        }
    }

    // 파이프 두 개: p2c(부모->자식), c2p(자식->부모)
    int p2c[2], c2p[2];
    if (pipe(p2c) < 0 || pipe(c2p) < 0) {
        perror("pipe");
        return 1;
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        // 자식
        pin_to_cpu0_or_exit("child");
        // 사용하지 않는 파이프 끝 닫기
        close(p2c[1]); // 자식은 p2c에서 읽기만
        close(c2p[0]); // 자식은 c2p에 쓰기만

        char buf;
        // 워밍업 약간 (스케줄 안정화)
        for (int w = 0; w < 1000; w++) {
            if (read(p2c[0], &buf, 1) != 1) exit(2);
            if (write(c2p[1], &buf, 1) != 1) exit(3);
        }

        for (;;) {
            ssize_t r = read(p2c[0], &buf, 1);
            if (r == 0) break;            // 부모 종료
            if (r < 0) { perror("child read"); exit(4); }
            if (write(c2p[1], &buf, 1) != 1) { perror("child write"); exit(5); }
        }
        close(p2c[0]);
        close(c2p[1]);
        return 0;
    } else {
        // 부모
        pin_to_cpu0_or_exit("parent");
        close(p2c[0]); // 부모는 p2c에 쓰기만
        close(c2p[1]); // 부모는 c2p에서 읽기만

        char byte = 0x5A, echo = 0;
        // 워밍업
        for (int w = 0; w < 1000; w++) {
            if (write(p2c[1], &byte, 1) != 1) { perror("warmup write"); return 1; }
            if (read(c2p[0], &echo, 1) != 1) { perror("warmup read"); return 1; }
        }

        uint64_t t0 = usec_now();
        for (long i = 0; i < iters; i++) {
            if (write(p2c[1], &byte, 1) != 1) { perror("write"); return 1; }
            if (read(c2p[0], &echo, 1) != 1) { perror("read"); return 1; }
        }
        uint64_t t1 = usec_now();

        // 종료 신호: p2c의 write 끝을 닫으면, 자식 read()가 0을 받고 종료
        close(p2c[1]);
        close(c2p[0]);

        int status = 0;
        waitpid(pid, &status, 0);

        double total_us = (double)(t1 - t0);
        double per_roundtrip_us = total_us / (double)iters;
        double per_roundtrip_ns = per_roundtrip_us * 1000.0;
        double per_ctxswitch_ns  = per_roundtrip_ns / 2.0; // 근사값

        printf("iters                 : %ld (round-trips)\n", iters);
        printf("total time            : %.3f us\n", total_us);
        printf("avg per round-trip    : %.3f ns\n", per_roundtrip_ns);
        printf("~avg per ctx switch   : %.3f ns (approx round-trip/2)\n", per_ctxswitch_ns);
        printf("(both processes pinned to CPU 0)\n");
        return 0;
    }
}
