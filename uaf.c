#include <stdio.h>
#include <stdlib.h>

int main(void) {
    int *a = malloc(100 * sizeof(int));   // 100개 int 동적 할당
    if (!a) { perror("malloc"); return 1; }

    for (int i = 0; i < 100; i++) a[i] = i;

    free(a);           // 메모리 해제
    // a는 여전히 이전 주소를 들고 있음(댕글링 포인터)

    printf("%d\n", a[0]);  // ❌ 해제된 메모리 접근 (use-after-free)
    return 0;
}
