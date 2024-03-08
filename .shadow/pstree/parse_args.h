#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>

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
