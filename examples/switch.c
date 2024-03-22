int printf(const char *, ...);

int main(int argc, char **argv) {
    switch (argc) {
        case 1:
            printf("No arguments\n");
            break;
        case 2:
            printf("One argument\n");
            break;
        default:
            printf("More than one argument\n");
    }
}
