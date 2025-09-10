#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    int use_append = (argc > 1 && strcmp(argv[1], "--append") == 0);

    int flags = O_CREAT | O_WRONLY | O_TRUNC;
    if (use_append) flags |= O_APPEND;

    int fd = open("test.log", flags, 0644);
    if (fd < 0) { perror("open"); return 1; }

    // 부모가 미리 한 줄 써둠
    dprintf(fd, "[parent %d] file opened%s\n",
            getpid(), use_append ? " (O_APPEND)" : "");

    pid_t pid = fork();
    if (pid < 0) { perror("fork"); return 1; }

    const int N = 50;
    char buf[128];

    if (pid == 0) {
        // 자식
        for (int i = 0; i < N; i++) {
            int len = snprintf(buf, sizeof(buf),
                               "[child  %d] i=%02d\n", getpid(), i);
            // 의도적으로 약간의 지연을 줘서 경쟁을 유발
            // (순서를 섞이게 하려는 용도)
            // usleep(1000); // 필요하면 해제
            if (write(fd, buf, len) != len) { perror("write(child)"); }
        }
        // 자식 종료
        _exit(0);
    } else {
        // 부모
        for (int i = 0; i < N; i++) {
            int len = snprintf(buf, sizeof(buf),
                               "[parent %d] i=%02d\n", getpid(), i);
            if (write(fd, buf, len) != len) { perror("write(parent)"); }
        }
        waitpid(pid, NULL, 0);
        fsync(fd);
        close(fd);

        puts("Done. Check test.log");
    }
    return 0;
}
