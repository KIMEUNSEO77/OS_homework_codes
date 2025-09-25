#include <stdio.h>

int main(void) {
    int data[100];          // 인덱스 범위: 0 ~ 99
    data[100] = 0;          // ❌ 경계 밖 쓰기 (정확한 마지막 인덱스는 99)
    printf("done\n");
    return 0;
}
