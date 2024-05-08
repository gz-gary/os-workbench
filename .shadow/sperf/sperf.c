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
    char syscall_name[64];
    int name_len;
    float time;

    for (ptr_l = info; *ptr_l != '['; ++ptr_l);
    ++ptr_l;
    sscanf(ptr_l, "%ld", &syscall_id);

    while (*ptr_l != ']') ++ptr_l;
    ptr_l += 2;

    if (*ptr_l == '+') return;

    for (ptr_r = ptr_l, name_len = 0; *ptr_r != '('; ++ptr_r, ++name_len);
    strncpy(syscall_name, ptr_l, name_len);
    syscall_name[name_len] = '\0';

    for (ptr_l = ptr_r; *ptr_l != '<'; ++ptr_l);
    sscanf(ptr_l, "%f", &time);

    printf("%c\n", *ptr_l);
    printf("%s id=%ld cost=%.8f\n", syscall_name, syscall_id, time);
    printf("\n");
}

int main(int argc, char *argv[]) {
    assert(argc >= 2);

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
