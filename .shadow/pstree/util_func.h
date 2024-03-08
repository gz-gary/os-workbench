#include <string.h>

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
