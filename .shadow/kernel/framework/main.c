// DO NOT MODIFY: Will be reverted by the Online Judge.

#include <kernel.h>
#include <klib.h>

void memset_test() {
    int x[20];
    for (int i = 0; i < 20; ++i) {
        printf("%d ", x[i]);
    }
    printf("\n");
    memset(x, 1, sizeof(x));
    for (int i = 0; i < 20; ++i) {
        printf("%d ", x[i]);
    }
    printf("\n");
}

int main() {
    memset_test();
    //os->init();
    //mpe_init(os->run);
    return 1;
}
