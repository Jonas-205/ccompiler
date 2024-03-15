extern void fun(int a);

typedef void (*handler)(int sig);
typedef void (*handler2)(int sig, bool);
// fn that takes two args (int, void(*)(int)), returns fn that takes int and returns void
extern void (*(signal1)(int, void(*)(int)))(int);
extern void (*signal(int, void(*)(int)))(int);
typedef void (*(*handler3)(int, void(*)(int)))(int);

int main(void) {
    void (*fn_ptr)(int) = &fun;
}
