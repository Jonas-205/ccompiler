static __attribute__((always_inline)) inline int jcl_color_attribute(FILE *out, JclColorAttribute attr) {
    return fprintf(out, "\033[%dm", attr);
}

int foo(char arg[]){

}
