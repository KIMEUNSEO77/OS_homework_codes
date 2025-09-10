#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

/*
 * 사용법 예:
 *   ./exec_variants --execl
 *   ./exec_variants --execle
 *   ./exec_variants --execlp
 *   ./exec_variants --execv
 *   ./exec_variants --execvp
 *   ./exec_variants --execve
 */

static void die(const char* msg) {
    perror(msg);
    _exit(127);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr,
            "Usage: %s [--execl|--execle|--execlp|--execv|--execvp|--execve]\n",
            argv[0]);
        return 2;
    }

    const char* which = argv[1];

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        // --- child ---
        // /bin/ls에 넘겨줄 인자들(예: -l, -a)
        char *const vec[] = { "ls", "-l", "-a", NULL };

        if (strcmp(which, "--execl") == 0) {
            // 경로를 절대경로로 직접 지정, 인자들을 가변인자(list)로 나열
            execl("/bin/ls", "ls", "-l", "-a", (char*)NULL);
            die("execl");

        } else if (strcmp(which, "--execle") == 0) {
            // 환경변수 배열을 직접 제공 (e = environment)
            char *const envp[] = {
                (char*)"MYTAG=execle",
                (char*)"LC_ALL=C",   // (출력 안정화를 위해 예시)
                NULL
            };
            execle("/bin/ls", "ls", "-l", "-a", (char*)NULL, envp);
            die("execle");

        } else if (strcmp(which, "--execlp") == 0) {
            // p = PATH 사용 (경로 검색) → "/bin/ls" 대신 "ls"만 줘도 됨
            execlp("ls", "ls", "-l", "-a", (char*)NULL);
            die("execlp");

        } else if (strcmp(which, "--execv") == 0) {
            // v = vector (argv 배열 사용), PATH는 사용하지 않음
            execv("/bin/ls", vec);
            die("execv");

        } else if (strcmp(which, "--execvp") == 0) {
            // v + p: argv 배열 사용 + PATH 검색
            execvp("ls", vec);
            die("execvp");

        } else if (strcmp(which, "--execve") == 0) {
            // 커널 시스템콜 형태. 경로 + argv[] + envp[] 모두 제공
            char *const envp[] = {
                (char*)"MYTAG=execve",
                (char*)"LC_ALL=C",
                NULL
            };
            execve("/bin/ls", vec, envp);
            die("execve");

        } else {
            fprintf(stderr, "unknown option: %s\n", which);
            _exit(2);
        }
    }

    // --- parent ---
    int status = 0;
    waitpid(pid, &status, 0);
    if (WIFEXITED(status)) {
        printf("[parent] child exited with %d\n", WEXITSTATUS(status));
    } else if (WIFSIGNALED(status)) {
        printf("[parent] child killed by signal %d\n", WTERMSIG(status));
    }
    return 0;
}
