#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <dirent.h>
#include <sys/procfs.h>
#include "util_ds.h"
#include "util_func.h"

ListNode *processes_list_tail;

void fetch_one_process(const char *pid_str) {
    char proc_filename[128] = "/proc/";
    strcat(proc_filename, pid_str);
    strcat(proc_filename, "/stat");

    FILE *fp = fopen(proc_filename, "r");
    if (!fp) goto release_fp;

    Process *proc = malloc(sizeof(Process));
    fscanf(fp, "%d (%s) %*c %d", proc->pid, proc->name, proc->ppid);
    processes_list_tail = insert_item(processes_list_tail, proc);

release_fp:
    if (fp) fclose(fp);
}

void fetch_all_processes() {
    DIR *dir = opendir("/proc");
    if (!dir) goto release_dir;

    struct dirent *entry;

    char proc_filename[128];

    while ((entry = readdir(dir)) != NULL) {
        // if entry represents a process
        if (is_pure_digits(entry->d_name)) {
            fetch_one_process(entry->d_name);

        }
    }

release_dir:
    if (dir) closedir(dir);
}

void buildup_process_tree() {
}

void show_all_processes() {
    for (ListNode *now = processes_list_tail; now; now = now->prev) {
        printf("%s\n", now->item->name);
    }
}

int main(int argc, char *argv[]) {
    for (int i = 0; i < argc; i++) {
        assert(argv[i]);
        printf("argv[%d] = %s\n", i, argv[i]);
    }
    assert(!argv[argc]);

    fetch_all_processes();
    show_all_processes();
  
    return 0;
}
