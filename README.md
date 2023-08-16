# Bigint implementation in C

I developed it to solve [Project Euler problem 16](https://projecteuler.net/problem=16). it doesn't support all mathematical operations yet.

### Usage
```c
#define BIGINT_IMPLEMENTATION
#include "bigint.h"

int main(void) {
    bigint number = bigint_new("2");
    bigint n = bigint_new("10");

    bigint_pow(&number, &n);
    bigint_print(&number) // 1024

    bigint_add(&number, &n);
    bigint_print(&number) // 1034

    bigint_free(&number);
    bigint_free(&n);

    return 0;
}
```

### Tests
Requirements: CUnit
```shell
./test.sh
```
