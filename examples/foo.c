static int c = 3;

static int bar();

extern int foo(int a, int b) {
    int e = 5;
    return a + b * c + bar() + e;
}

int main(void) {
    return foo(2, 3);
}

static int bar() {
    return 4;
}
