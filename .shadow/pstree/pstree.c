#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <dirent.h>
#include <sys/procfs.h>
#include "util_ds.h"
#include "util_func.h"
#include "parse_args.h"

ListNode *processes_list_tail;
Process *root_of_process_tree;

void fetch_one_process(const char *pid_str) {
    char proc_filename[128] = "/proc/";
    strcat(proc_filename, pid_str);
    strcat(proc_filename, "/stat");

    FILE *fp = fopen(proc_filename, "r");
    if (!fp) goto release_fp;

    Process *proc = malloc(sizeof(Process));
    fscanf(fp, "%d %s %*c %d", &proc->pid, proc->name, &proc->ppid);
    remove_parentheses(proc->name);
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
    for (ListNode *now = processes_list_tail; now; now = now->prev) {
        for (ListNode *parent = processes_list_tail; parent; parent = parent->prev)
            if (parent->item->pid == now->item->ppid) {
                parent->item->son_list_tail = insert_item(parent->item->son_list_tail, now->item);
                break;
            }
        if (!now->item->ppid)
            root_of_process_tree = now->item;
    }

    // using bubble sort for LinkList here
    if (flag_n) {
        for (ListNode *now = processes_list_tail; now; now = now->prev) {
            for (ListNode *p1 = now->item->son_list_tail; p1; p1 = p1->prev) {
                ListNode *p2 = now->item->son_list_tail;
                while (((p2->prev) != NULL) && ((p2->item)->pid) > (((p2->prev)->item)->pid)) {
                    Process *t = p2->item;
                    p2->item = (p2->prev)->item;
                    (p2->prev)->item = t;
                    p2 = p2->prev;
                }
            }
        }
    }

}

void traverse_process_tree(Process *now, int depth) {
    for (int i = 0; i < 2 * depth; ++i) printf(" ");
    int is_leaf_node = (now->son_list_tail == NULL);
    if (is_leaf_node) return;
    printf("%s", now->name);
    // if (is_leaf_node) printf("{%s}", now->name);
    // else printf("%s", now->name);
    if (flag_p) printf("(%d)", now->pid);
    printf("\n");

    for (ListNode *son = now->son_list_tail; son; son = son->prev) {
        traverse_process_tree(son->item, depth + 1);
    }
}

void show_all_processes() {
    for (ListNode *now = processes_list_tail; now; now = now->prev) {
        printf("%d %s\n", now->item->pid, now->item->name);
    }
}

int main(int argc, char *argv[]) {
    /*for (int i = 0; i < argc; i++) {
        assert(argv[i]);
        printf("argv[%d] = %s\n", i, argv[i]);
    }
    assert(!argv[argc]);*/
    parse_args(argc, argv);

    if (flag_V) {
        fprintf(stderr, "pstree (a toy version)");
        return 0;
    }

    fetch_all_processes();
    buildup_process_tree();
    traverse_process_tree(root_of_process_tree, 0);
    // show_all_processes();
  
    return 0;
}
