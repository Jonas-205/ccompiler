static signed int c = 3;

typedef signed int sooooo;
static sooooo *bar();

extern signed char foo(int a, int b) {
    short int unsigned e = 5;
    return a + b * c + *bar() + e;
}

int main(void) {
    return foo(2, 3);
}

static sooooo *bar() {
    return &c;
}
