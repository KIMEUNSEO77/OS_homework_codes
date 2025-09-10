#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

int main(void) {
    int p[2];
    if (pipe(p) == -1) { perror("pipe"); return 1; }

    pid_t pid = fork();
    if (pid < 0) { perror("fork"); return 1; }

    if (pid == 0) {
        // --- child ---
        // 부모가 읽을 필요 없는 read-end 닫기
        close(p[0]);

        // 자식 출력 (unbuffered)
        const char *msg = "hello\n";
        if (write(STDOUT_FILENO, msg, 6) != 6) _exit(1);

        // 부모에게 "이제 출력해도 됨" 신호 보내기
        if (write(p[1], "X", 1) != 1) _exit(1);
        close(p[1]);
        _exit(0);
    } else {
        // --- parent ---
        // 부모가 쓸 필요 없는 write-end 닫기
        close(p[1]);

        // 자식이 신호 보낼 때까지 블록 (wait() 없이 순서 보장)
        char token;
        if (read(p[0], &token, 1) != 1) {
            // 자식이 비정상 종료해도 여기서 풀릴 수 있음
        }
        close(p[0]);

        const char *msg = "goodbye\n";
        if (write(STDOUT_FILENO, msg, 8) != 8) return 1;

        // 여기서 굳이 wait() 안 해도 순서에는 영향 없음
        return 0;
    }
}
