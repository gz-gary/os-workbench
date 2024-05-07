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
    /*for (int i = 0; i < argc; i++) {
        assert(argv[i]);
        printf("argv[%d] = %s\n", i, argv[i]);
    }
    assert(!argv[argc]);*/
    char **exec_argv = malloc((argc + 1) * sizeof(char *));
    memcpy(exec_argv, argv, argc * sizeof(char *));
    exec_argv[0] = "strace";
    exec_argv[argc] = NULL;
    for (int i = 0; i < argc; ++i) {
        assert(exec_argv[i]);
        printf("exec_argv[%d] = %s\n", i, exec_argv[i]);
    }
    
    /*int fid = open("log.txt", O_CREAT | O_WRONLY | O_TRUNC);
    assert(fid);
    dprintf(fid, "--- strace log below ---\n\n");

    int pid = fork();
    if (pid == 0) {
        syscall(SYS_close, 2);
        syscall(SYS_dup, fid);
        close(fid);

        char *exec_argv[] = { "strace", argv[1], NULL, };
        char *exec_envp[] = { "PATH=/bin", NULL, };
        execve("strace",          exec_argv, exec_envp);
        execve("/bin/strace",     exec_argv, exec_envp);
        execve("/usr/bin/strace", exec_argv, exec_envp);
        perror(argv[0]);
        exit(EXIT_FAILURE);
    }
    int status;
    pid = wait(&status);
    assert(!WEXITSTATUS(status)); // strace exits normally

    free(exec_argv);
    close(fid);*/
    return 0;
}
