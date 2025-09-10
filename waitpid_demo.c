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
        printf("[child %d] I am the child, exiting with status 7\n", getpid());
        _exit(7);
    } else {
        // ---- 부모 ----
        int status;
        pid_t cpid = waitpid(pid, &status, 0);  // 특정 pid를 기다림
        if (cpid == -1) {
            perror("waitpid");
            return 1;
        }

        printf("[parent %d] waitpid() returned child PID=%d\n", getpid(), cpid);

        if (WIFEXITED(status)) {
            printf("[parent %d] child exited normally with status=%d\n",
                   getpid(), WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            printf("[parent %d] child killed by signal %d\n",
                   getpid(), WTERMSIG(status));
        }
    }
    return 0;
}
