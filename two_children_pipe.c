#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(void) {
    int pfd[2];                // pfd[0]=read end, pfd[1]=write end
    if (pipe(pfd) == -1) { perror("pipe"); return 1; }

    pid_t w = fork();          // writer child
    if (w < 0) { perror("fork writer"); return 1; }

    if (w == 0) {
        // --- Child A: writer (stdout -> pipe write end) ---
        // 파이프의 읽기 끝은 필요 없으므로 닫기
        close(pfd[0]);

        // 표준출력을 파이프 쓰기 끝으로 바꿈
        if (dup2(pfd[1], STDOUT_FILENO) == -1) { perror("dup2 writer"); _exit(1); }
        close(pfd[1]); // dup2로 복제했으니 원본 fd는 닫기

        // 이제부터 printf는 파이프를 통해 흘러감 (자식 B의 stdin으로 들어감)
        for (int i = 1; i <= 5; i++) {
            printf("msg %d from child A\n", i);
            fflush(stdout);
            usleep(100000); // 보이기 좋게 살짝 지연
        }
        _exit(0);
    }

    pid_t r = fork();          // reader child
    if (r < 0) { perror("fork reader"); return 1; }

    if (r == 0) {
        // --- Child B: reader (stdin <- pipe read end) ---
        // 파이프의 쓰기 끝은 필요 없으므로 닫기
        close(pfd[1]);

        // 표준입력을 파이프 읽기 끝으로 바꿈
        if (dup2(pfd[0], STDIN_FILENO) == -1) { perror("dup2 reader"); _exit(1); }
        close(pfd[0]);

        // 간단한 "cat"처럼 stdin에서 읽어 부모 터미널로 그대로 내보내기
        // (여기서 stdout은 터미널을 향함)
        char buf[256];
        ssize_t n;
        while ((n = read(STDIN_FILENO, buf, sizeof(buf))) > 0) {
            if (write(STDOUT_FILENO, buf, n) != n) { perror("write"); _exit(1); }
        }
        if (n < 0) { perror("read"); _exit(1); }
        _exit(0);
    }

    // --- Parent ---
    // 부모는 파이프의 양쪽을 닫아야 자식 B가 EOF를 감지함
    close(pfd[0]);
    close(pfd[1]);

    // 두 자식 종료 대기
    int st;
    waitpid(w, &st, 0);
    waitpid(r, &st, 0);

    return 0;
}
