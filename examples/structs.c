struct Foo {
    int x;
};

typedef struct {
    int y;
    int barrrr;
} Bar;

typedef struct Foo Foo;

typedef struct
{
  int __count;
  union
  {
    unsigned int __wch;
    char __wchb[4];
  } __value;
} __mbstate_t;

int foo() {
    return sizeof(__mbstate_t);
}
