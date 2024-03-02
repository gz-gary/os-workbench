#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <dirent.h>
#include <sys/procfs.h>

int is_pure_digits(const char *str) {
    int len = strlen(str);
    for (int i = 0; i < len; ++i)
        if (str[i] < '0' || str[i] > '9') return 0;
    return 1;
}

typedef struct Process Process;

int process_cnt;

struct Process {
    pid_t pid;
    int id;
    Process *parent;
};

void fetch_all_processes() {
    DIR *dir = opendir("/proc");
    if (dir == NULL) return;

    FILE *fp = fopen("/proc/", "r");
    struct dirent *entry;

    while ((entry = readdir(dir)) != NULL) {
        printf("%s\n", entry->d_name);
    }
}

int main(int argc, char *argv[]) {
    for (int i = 0; i < argc; i++) {
        assert(argv[i]);
        printf("argv[%d] = %s\n", i, argv[i]);
    }
    assert(!argv[argc]);

    fetch_all_processes();
  
    return 0;
}
