#include <assert.h>
#include <cstdio>
#include <set>
#include <cstdint>
#include <cstring>
#include <algorithm>
using namespace std;

using range = pair<uintptr_t, uintptr_t>;

set<range> ranges;

bool cross(const range &r1, const range &r2) {
    auto maxl = max(r1.first, r2.first);
    auto minr = min(r1.second, r2.second);
    return maxl <= minr;
}

int main() {
    char buf[100];
    void *l, *r;
    while (scanf("%s", buf) != EOF) {
        if (!strcmp(buf, "kalloc")) {
            scanf("%p%p", &l, &r);
            range new_range{(uintptr_t)l, (uintptr_t)r - 1};
            for (auto old_range : ranges) {
                assert(!cross(new_range, old_range));
            }
            ranges.insert(new_range);
        } else {
            scanf("%p", &l);
            auto iter = ranges.lower_bound({(uintptr_t)l, 0});
            assert(iter != ranges.end());
            ranges.erase(iter);
        }
    }
    printf("Congratulations! checker passed.\n");
    return 0;
}