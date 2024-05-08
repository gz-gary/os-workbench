#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/syscall.h>

#define swap(a, b) {     \
    typeof(a) t = (a);   \
    a = (b);             \
    b = t; }             \

#define SYSCALL_NAME_LEN 64
#define NR_SYSCALLS 512

typedef struct {
    char syscall_name[SYSCALL_NAME_LEN];
    float time;
    int rank;
} syscall_stat_t;
syscall_stat_t syscall_stats[NR_SYSCALLS];

int rank_to_syscall_id[NR_SYSCALLS];

float total_time;

int nr_syscalls;
int last_print_flag;
struct timeval last_print_time;

void parse(const char *info) {
    /* --- parse information from strace output --- */

    const char *ptr_l, *ptr_r;
    long syscall_id;
    char syscall_name[SYSCALL_NAME_LEN];
    int name_len;
    float time;

    for (ptr_l = info; *ptr_l && *ptr_l != '['; ++ptr_l);
    assert(*ptr_l == '[');
    ++ptr_l;
    sscanf(ptr_l, "%ld", &syscall_id);

    while (*ptr_l && *ptr_l != ']') ++ptr_l;
    assert(*ptr_l == ']');
    ptr_l += 2;

    if (*ptr_l == '+') return;

    for (ptr_r = ptr_l, name_len = 0; *ptr_r && *ptr_r != '('; ++ptr_r, ++name_len);
    assert(*ptr_r == '(');
    strncpy(syscall_name, ptr_l, name_len);
    syscall_name[name_len] = '\0';

    for (ptr_l = ptr_r; *ptr_l && *ptr_l != '<'; ++ptr_l);
    assert(*ptr_l == '<');
    ++ptr_l;
    sscanf(ptr_l, "%f", &time);

    /* --- update statistics ---*/

    syscall_stat_t *stat = &syscall_stats[syscall_id];
    if (stat->syscall_name[0] == '\0') {
        strcpy(stat->syscall_name, syscall_name);
        ++nr_syscalls;
    }
    stat->time += time;
    total_time += time;

    /* --- update ranking using bubble sort --- */

    int rank = stat->rank;
    while (rank > 0) {
        syscall_stat_t *pre_stat = &syscall_stats[rank_to_syscall_id[rank - 1]];
        if (stat->time > pre_stat->time) {
            swap(stat->rank, pre_stat->rank);
            swap(rank_to_syscall_id[rank], rank_to_syscall_id[rank - 1]);
            --rank;
        } else break;
    }
}

void output_stat() {
    printf("---\n");
    for (int i = 0, syscall_id; i < 5 && i < nr_syscalls; ++i) {
        syscall_id = rank_to_syscall_id[i];
        printf("%s (%d%%)\n", syscall_stats[syscall_id].syscall_name, (int)(syscall_stats[syscall_id].time / total_time * 100.f));
    }
    printf("---\n");
    for (int i = 0; i < 80; ++i) putchar('\0');
    fflush(stdout);
}

void init() {
    memset(syscall_stats, 0, sizeof(syscall_stats));
    total_time = 0.f;
    last_print_flag = 0;
    nr_syscalls = 0;

    for (int i = 0; i < NR_SYSCALLS; ++i) {
        syscall_stats[i].rank = i;
        rank_to_syscall_id[i] = i;
    }
}

int main(int argc, char *argv[]) {
    assert(argc >= 2);

    init();

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
        struct timeval current_time;
        gettimeofday(&current_time, NULL);
        if (!last_print_flag || current_time.tv_usec - last_print_time.tv_usec >= 100) {
            last_print_flag = 1;
            last_print_time = current_time;
            output_stat();
        }
    }
    if (!last_print_flag)
        output_stat();
    // int status;
    // pid = wait(&status);
    // assert(!WEXITSTATUS(status)); // strace exits normally

    free(exec_argv);
    return 0;
}
