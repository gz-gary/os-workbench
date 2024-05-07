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

int main(int argc, char *argv[]) {
    char **exec_argv = malloc((argc + 2) * sizeof(char *));
    exec_argv[0] = "strace";
    exec_argv[1] = "-T";
    memcpy(exec_argv + 2, argv + 1, (argc - 1) * sizeof(char *));
    exec_argv[argc + 1] = NULL;

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

        printf("parse: %s \n", line_buf);
    }
    // int status;
    // pid = wait(&status);
    // assert(!WEXITSTATUS(status)); // strace exits normally

    free(exec_argv);
    return 0;
}
