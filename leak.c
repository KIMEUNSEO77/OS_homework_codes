#include <stdio.h>
#include <stdlib.h>

void leak_once(void) {
    int *p = malloc(100 * sizeof(int));   // 동적 할당 (약 400바이트)
    if (!p) { perror("malloc"); exit(1); }

    for (int i = 0; i < 100; i++) p[i] = i;
    printf("last=%d\n", p[99]);

    // free(p);  // ❌ 해제 생략 → 메모리 누수 발생
}

int main(void) {
    leak_once();
    return 0;   // 프로그램 종료 전에 p를 해제하지 않음
}
