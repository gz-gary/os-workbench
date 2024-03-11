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

#define for_in_list(list_tail, node) for (ListNode *node = (list_tail); (node); node = ((node)->prev))
#define for_in_list_prev(list_tail, node, prev_node) for (ListNode *node = (list_tail), *prev_node = ((node)->prev); (node); node = (prev_node), prev_node = ((node) ? ((node)->prev) : NULL))
#define insert_in_list(list_tail, node) list_tail = insert_item((list_tail), (node))
#define swap(type, a, b) { type t = (a); a = (b); b = t; }

ListNode *processes_list_tail;
Process *root_of_process_tree;

void fetch_one_process(const char *pid_str) {
    char proc_filename[128];
    sprintf(proc_filename, "/proc/%s/stat", pid_str);

    FILE *fp = fopen(proc_filename, "r");
    if (!fp) goto release_fp;

    Process *proc = malloc(sizeof(Process));
    fscanf(fp, "%d %s %*c %d", &proc->pid, proc->name, &proc->ppid);
    proc->son_list_tail = NULL;
    remove_parentheses(proc->name);
    insert_in_list(processes_list_tail, proc);

release_fp:
    if (fp) fclose(fp);
}

void fetch_all_processes() {
    DIR *dir = opendir("/proc");
    if (!dir) goto release_dir;

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
        if (is_pure_digits(entry->d_name)) // if entry represents a process
            fetch_one_process(entry->d_name);

release_dir:
    if (dir) closedir(dir);
}

void buildup_process_tree() {
    for_in_list(processes_list_tail, proc) {
        for_in_list(processes_list_tail, parent)
            if (parent->item->pid == proc->item->ppid) {
                insert_in_list(parent->item->son_list_tail, proc->item);
                break;
            }
        if (!proc->item->ppid)
            root_of_process_tree = proc->item;
    }

    // using bubble sort for LinkList here
    if (flag_n)
        for_in_list(processes_list_tail, proc)
            for_in_list(proc->item->son_list_tail, _)
                for_in_list_prev(proc->item->son_list_tail, now, prev)
                    if (prev && now->item->pid > prev->item->pid)
                        swap(Process*, now->item, prev->item);

}

void traverse_process_tree(Process *now, int depth) {
    for (int i = 0; i < 2 * depth; ++i) printf(" ");
    printf("%s", now->name);
    if (flag_p) printf("(%d)", now->pid);
    printf("\n");

    for_in_list(now->son_list_tail, son)
        traverse_process_tree(son->item, depth + 1);
}

void show_all_processes() {
    int cnt = 0;
    for (ListNode *now = processes_list_tail; now; now = now->prev) {
        ++cnt;
        //printf("%d %s\n", now->item->pid, now->item->name);
    }
    printf("%d\n", cnt);
}

void cleanup() {
    int cnt = 0;
    for_in_list_prev(processes_list_tail, now, _) {

        Process *proc = now->item;
        //for (ListNode *son = proc->son_list_tail; son; son = son->prev) {
        //}
        for_in_list_prev(proc->son_list_tail, son, __) {
            ++cnt;
            //free(son);
        }

        //free(now->item);
        //free(now);
    }
    printf("%d\n", cnt);
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
    show_all_processes();
    cleanup();
  
    return 0;
}
