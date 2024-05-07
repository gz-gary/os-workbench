#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/syscall.h>

void parse(const char *info) {
    const char *ptr_l, *ptr_r;

    long syscall_id;
    sscanf(info, "[%ld]", &syscall_id);

    char syscall_name[64];

    ptr_r = info;
    while (*ptr_r != '(') ++ptr_r;

    ptr_l = info;
    while (*ptr_l != ']') ++ptr_l;
    ptr_l += 2;

    strncpy(syscall_name, ptr_l, ptr_r - ptr_l);
    printf("%d %s id=%ld\n", (int)(ptr_r - ptr_l), syscall_name, syscall_id);
}

int main(int argc, char *argv[]) {
    char **exec_argv = malloc((argc + 3 + 1) * sizeof(char *));
    exec_argv[0] = "strace";
    exec_argv[1] = "-T";
    exec_argv[2] = "-n";
    exec_argv[3] = "--strings-in-hex=all";
    memcpy(exec_argv + 4, argv + 1, (argc - 1) * sizeof(char *));
    exec_argv[argc + 3] = NULL;

    char *exec_envp[] = { "PATH=/bin", NULL, };
    
    int pipefd[2];
    assert(syscall(SYS_pipe, pipefd) >= 0);

    int pid = fork();
    if (pid == 0) {
        syscall(SYS_close, 2);
        syscall(SYS_dup, pipefd[1]);
        syscall(SYS_close, pipefd[0]);
        syscall(SYS_close, pipefd[1]);

        execve("strace",          exec_argv, exec_envp);
        execve("/bin/strace",     exec_argv, exec_envp);
        execve("/usr/bin/strace", exec_argv, exec_envp);
        perror(exec_argv[0]);
        exit(EXIT_FAILURE);
    }
    syscall(SYS_close, 0);
    syscall(SYS_dup, pipefd[0]);
    syscall(SYS_close, pipefd[0]);
    syscall(SYS_close, pipefd[1]);

    while (1) {
        static char line_buf[4096];

        if (!fgets(line_buf, sizeof(line_buf), stdin)) {
            break;
        }

        parse(line_buf);
    }
    // int status;
    // pid = wait(&status);
    // assert(!WEXITSTATUS(status)); // strace exits normally

    free(exec_argv);
    return 0;
}
