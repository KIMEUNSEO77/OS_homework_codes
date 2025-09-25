#include <stdlib.h>
#include <stdio.h>

int main(void) {
    int *a = malloc(100 * sizeof(int)); // 정수 100개 (400바이트)
    if (!a) return 1;

    int *mid = a + 50;   // 배열 중간을 가리키는 포인터
    free(mid);           // ❌ 잘못된 free

    return 0;
}
