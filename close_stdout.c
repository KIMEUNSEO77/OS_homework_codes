#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(void) {
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        // ---- 자식 ----
        printf("[child %d] before closing stdout\n", getpid());
        fflush(stdout);

        close(STDOUT_FILENO);   // 표준 출력 닫기

        // 이제 printf()를 호출하면?
        printf("[child %d] after closing stdout\n", getpid());
        fflush(stdout); // 버퍼 비우기 시도

        _exit(0);
    } else {
        // ---- 부모 ----
        wait(NULL);
        printf("[parent %d] child has exited\n", getpid());
    }
    return 0;
}
