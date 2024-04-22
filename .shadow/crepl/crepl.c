#include <stdio.h>
#include <string.h>

#include <dlfcn.h>
#include <fcntl.h>
#include <assert.h>
#include <stddef.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#define SRC_FILE_NAME_INIT "/tmp/crepl_src_XXXXXX.c"
#define DLIB_FILE_NAME_INIT "/tmp/crepl_dlib_XXXXXX.so"
#define get_list_entry(list_node, type, member) ((type *)((void *)(list_node) - offsetof(type, member)))
#define for_list_node(list_head, list_node) \
	for (list_node_t *list_node = (list_head).next; \
         list_node != &(list_head); \
		 list_node = list_node->next)

typedef struct list_node_t list_node_t;
struct list_node_t {
	list_node_t *next, *prev;
} c_code_list;
int c_expr_id;

static inline void list_node_init(list_node_t *list_node) {
	list_node->next = list_node;
	list_node->prev = list_node;
}

static inline void list_insert(list_node_t *list_node, list_node_t *prev, list_node_t *next) {
	prev->next = list_node;
	next->prev = list_node;
	list_node->next = next;
	list_node->prev = prev;
}

static inline void list_insert_front(list_node_t *list_node, list_node_t *list_head) {
	list_insert(list_node, list_head, list_head->next);
}

static inline void list_remove(list_node_t *list_node) {
	list_node->prev->next = list_node->next;
	list_node->next->prev = list_node->prev;
}

static inline void init() {
	list_node_init(&c_code_list);
	c_expr_id = 0;
}

typedef struct c_code_t c_code_t;
struct c_code_t {
	char *dlib_file_name;
	void *dlib_handle;
	list_node_t list_node;
};

void compile_c_code(char *const src_file_name,
                    char *const dlib_file_name) {
    char *const argv[] = {
        "/bin/gcc",
		"-fPIC",
		"-shared",
		"-O1",
		"-std=gnu11",
		#ifdef __x86_64__
			"-m64",
		#else
			"-m32",
		#endif
		"-Wno-implicit-function-declaration",
		"-Wno-unused-result",
		"-Wno-unused-value",
		"-Wno-unused-variable",
		src_file_name,
		"-o",
		dlib_file_name,
        NULL,
    };

    pid_t pid = fork();
    if (!pid) {
	    execv(argv[0], argv);
    } else {
		int status;
		pid = wait(&status);
		printf("[crepl]: exit_status = %d\n", WEXITSTATUS(status));
		if (WIFEXITED(status) && !WEXITSTATUS(status)) {
			printf("[crepl]: Compile successfully.\n");
		} else printf("[crepl]: Fail to Compile.\n");
	}
}

c_code_t *def_c_func(char *const code) {
	char src_file_name[] = SRC_FILE_NAME_INIT;
	char dlib_file_name[] = DLIB_FILE_NAME_INIT;

	int src_file_fd = mkstemps(src_file_name, strlen(".c"));
	dprintf(src_file_fd, "%s", code);
	close(src_file_fd);

	int dlib_file_fd = mkstemps(dlib_file_name, strlen(".so"));
	close(dlib_file_fd);

	compile_c_code(src_file_name, dlib_file_name);
	remove(src_file_name);

	c_code_t *c_code = malloc(sizeof(c_code_t));

	c_code->dlib_handle    = dlopen(dlib_file_name, RTLD_LAZY | RTLD_GLOBAL);
	assert(c_code->dlib_handle);
	dlerror();
	c_code->dlib_file_name = malloc(strlen(dlib_file_name) + 1);
	strcpy(c_code->dlib_file_name, dlib_file_name);

	list_insert_front(&c_code->list_node, &c_code_list);

	return c_code;
}

int eval_c_expr(char *const expr) {

	char *wrapper_func_code = malloc(strlen(expr) + 1 + 100);
	char *wrapper_func_name = malloc(100);
	sprintf(wrapper_func_name, "__expr_wrapper_%d", c_expr_id);
	sprintf(wrapper_func_code, "int %s() { return %s; }", wrapper_func_name, expr);
	// printf("%s\n", wrapper_func_code);
	// printf("%s\n", wrapper_func_name);

	c_code_t *c_code = def_c_func(wrapper_func_code);
	int (*expr_func)(void) = dlsym(c_code->dlib_handle, wrapper_func_name);
	int ret = expr_func();

	free(wrapper_func_code);
	free(wrapper_func_name);

	++c_expr_id;

	return ret;
}

__attribute__((destructor))
void cleanup() {
	for (list_node_t *ln = c_code_list.next; ln != &c_code_list; ln = c_code_list.next) {
		c_code_t *c_code = get_list_entry(ln, c_code_t, list_node);

		dlclose(c_code->dlib_handle);
		remove(c_code->dlib_file_name);
		free(c_code->dlib_file_name);
		list_remove(ln);
		free(c_code);
	}
}

int main(int argc, char *argv[]) {
    init();

    static char line[4096];

    while (1) {
        printf("crepl> ");
        fflush(stdout);

        if (!fgets(line, sizeof(line), stdin)) {
            break;
        }
        assert(line[strlen(line)] == '\0');

        if (strlen(line) >= 3 && line[0] == 'i' &&
                                 line[1] == 'n' &&
                                 line[2] == 't') {
            def_c_func(line);
        } else {
            int value = eval_c_expr(line);
            printf("(%s) == %d.\n", line, value);
            fflush(stdout);
        }

        // To be implemented.
        printf("Got %zu chars.\n", strlen(line));
    }
}
