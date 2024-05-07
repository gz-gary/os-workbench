#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/syscall.h>

int main(int argc, char *argv[]) {
    /*for (int i = 0; i < argc; i++) {
        assert(argv[i]);
        printf("argv[%d] = %s\n", i, argv[i]);
    }
    assert(!argv[argc]);*/
    int fid = syscall(SYS_open, "log.txt", O_CREAT | O_WRONLY | O_TRUNC);
    assert(fid);
    dprintf(fid, "--- strace log below ---\n");

    int pid = fork();
    if (pid == 0) {
        char *exec_argv[] = { "strace", "ls", NULL, };
        char *exec_envp[] = { "PATH=/bin", NULL, };
        execve("strace",          exec_argv, exec_envp);
        execve("/bin/strace",     exec_argv, exec_envp);
        execve("/usr/bin/strace", exec_argv, exec_envp);
        perror(argv[0]);
        exit(EXIT_FAILURE);
    }

    close(fid);
    return 0;
}
