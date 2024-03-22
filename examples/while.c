int printf(const char *, ...);

int main(void) {
    int i = 10;
    while (i > 0) {
        printf("%d\n", i);
        i--;
    }
    do {
        printf("%d\n", i);
        i++;
    } while (i < 10);
}
