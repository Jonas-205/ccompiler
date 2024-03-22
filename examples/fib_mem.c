int printf(const char* format, ...);

typedef struct {
    int is_computed;
    int value;
} fib_mem_t;

static fib_mem_t fib_mem_table[2 * 64];

int fib_mem(int n) {
    if (n == 0) {
        return 0;
    } else if (n == 1) {
        return 1;
    }
    if (fib_mem_table[n].is_computed) {
        return fib_mem_table[n].value;
    }
    fib_mem_table[n].value = fib_mem(n - 1) + fib_mem(n - 2);
    fib_mem_table[n].is_computed = 1;
    return fib_mem_table[n].value;
}

int main(void) {
    int i;
    for (i = 0; i < 2 * 5; i++) {
        printf("%d\n", fib_mem(i));
    }
    return 0;
}
