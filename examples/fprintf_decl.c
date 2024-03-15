typedef struct FILE FILE;

extern int fprintf (FILE *__restrict __stream,
      const char *__restrict __format, ...) __attribute__ ((__nonnull__ (1)));
extern int remove (const char *__filename) __attribute__ ((__nothrow__ , __leaf__));
extern FILE *tmpfile (void)
    __attribute__ ((__malloc__)) __attribute__ ((__malloc__ (fclose, 1))) ;