#include <stdio.h>

int main(void) {
    int *p = NULL;   // 정수를 가리키는 포인터, NULL로 초기화
    printf("포인터 값: %p\n", (void*)p);

    // NULL 포인터 역참조 시도
    printf("포인터가 가리키는 값: %d\n", *p);

    return 0;
}
