#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <dirent.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/procfs.h>

/* --- Data structures --- */

typedef struct ListNode ListNode;
typedef struct Process Process;

struct ListNode {
    ListNode *prev;
    Process *item;
};

ListNode * insert_item(ListNode *list_tail, Process *new_item) {
    ListNode *new_node = (ListNode*)malloc(sizeof(ListNode));
    new_node->item = new_item;
    new_node->prev = list_tail;
    return new_node;
}

struct Process {
    pid_t pid;
    pid_t ppid;

    char name[128];

    // Process *parent;
    ListNode *son_list_tail;
};

/* --- String functions --- */

int is_pure_digits(const char *str) {
    int len = strlen(str);
    for (int i = 0; i < len; ++i)
        if (str[i] < '0' || str[i] > '9') return 0;
    return 1;
}

void remove_parentheses(char *str) {
    int len = strlen(str);
    for (int i = 0; i < len - 2; ++i) {
        str[i] = str[i + 1];
    }
    str[len - 2] = '\0';
    str[len - 1] = '\0';
}

/* --- Arguments resolution --- */

int flag_p, flag_n, flag_V;

void parse_args(int argc, char *argv[]) {
    int opt;
    int option_index = 0;
    char short_options[] = "pnV";
    struct option long_options[] = {
        {"show-pids", no_argument, NULL, 'p'},
        {"numeric-sort", no_argument, NULL, 'n'},
        {"version", no_argument, NULL, 'V'},
        {NULL, 0, NULL, 0}
    };

    while ((opt = getopt_long(argc, argv, short_options, long_options, &option_index)) != -1) {
        switch (opt) {
            case 'p':
                flag_p = 1;
                break;
            case 'n':
                flag_n = 1;
                break;
            case 'V':
                flag_V = 1;
                break;
        }
    }

}

#define for_in_list(list_tail, node) for (ListNode *node = (list_tail); (node); node = ((node)->prev))
#define for_in_list_prev(list_tail, node, prev_node) for (ListNode *node = (list_tail), *prev_node = ((list_tail) ? ((list_tail)->prev) : NULL); (node); node = (prev_node), prev_node = ((prev_node) ? ((prev_node)->prev) : NULL))
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

void cleanup() {
    for_in_list_prev(processes_list_tail, now, _) {
        for_in_list_prev(now->item->son_list_tail, son, __)
            free(son);
        free(now->item);
        free(now);
    }
}

int main(int argc, char *argv[]) {
    parse_args(argc, argv);

    if (flag_V) {
        fprintf(stderr, "pstree (a toy version)\n");
        return 0;
    }

    fetch_all_processes();
    buildup_process_tree();
    traverse_process_tree(root_of_process_tree, 0);
    cleanup();
  
    return 0;
}
