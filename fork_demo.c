#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>

int main(void) {
    int x = 100;
    printf("[PARENT %d] 초기 x=%d, &x=%p\n", getpid(), x, (void*)&x);
    fflush(stdout); // 출력 버퍼 비우기(순서 섞임 방지)

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return 1;
    }

    if (pid == 0) { // 자식
        printf("  [CHILD  %d] fork 직후 x=%d, &x=%p (parent=%d)\n",
               getpid(), x, (void*)&x, getppid());
        x = 200;
        printf("  [CHILD  %d] x=200으로 변경 후 x=%d\n", getpid(), x);
        _exit(0);
    } else {        // 부모
        // 자식이 먼저 출력하도록 기다렸다가
        waitpid(pid, NULL, 0);

        printf("[PARENT %d] 자식 종료 후 여전히 x=%d, &x=%p\n",
               getpid(), x, (void*)&x);
        x = 300;
        printf("[PARENT %d] x=300으로 변경 후 x=%d\n", getpid(), x);
    }
    return 0;
}
