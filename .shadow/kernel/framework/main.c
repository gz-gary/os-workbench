// DO NOT MODIFY: Will be reverted by the Online Judge.

#include <kernel.h>
#include <klib.h>

void memset_test() {
    int x[20];
    for (int i = 0; i < 20; ++i) {
        printf("%d ", x[i]);
    }
    printf("\n");
    char s[20];
    s[19] = '\0';
    memset(s, 'a', sizeof(s));
    printf("%s\n", s);
}

int main() {
    memset_test();
    //os->init();
    //mpe_init(os->run);
    return 1;
}
