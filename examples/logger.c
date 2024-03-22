/* #define JC_DISABLE_LOGGING */
/* #define JC_DISABLE_LOGGING_COLORS */
/* #define JC_DISABLE_ASSERTS */
#define JC_LOG_IMPLEMENTATION
#include "jc_log.h"

int main(void) {
    if (!jcl_file("logger.log")) {
        die("Could not open log file");
    }


    trace("TRACE");
    info("INFO");
    warn("WARN");
    error("ERROR");

    printf("-----------------------------------------------------------\n");

    jcl_toggle(false);
    trace("Not Printed");
    info("Not Printed");
    warn("Not Printed");
    error("Not Printed");
    /* jc_die("Would be printed (and die)"); */
    jcl_toggle(true);


    printf("-----------------------------------------------------------\n");

    jcl_level(JCL_LEVEL_WARN);

    trace("TRACE: %d", 3);  // not printed
    info("%s", "INFO");  // not printed

    if (jcl_level_current() >= JCL_LEVEL_WARN) {
        warn("Two messages were not printed because log level is %s", jcl_level_str(jcl_level_current()));
    }

    printf("-----------------------------------------------------------\n");

    warn("%s: %d", "WARN", 50);
    error("ERROR");
    jcl_assert(1 == 1, "One is not one");
    jcl_assert(1 == 0, "One is zero");  // will terminate the program
    die("DIE");

    // This wouln't be called because of DIE, however DIE flushes the log file before exiting, so it's not necessary.
    if (!jcl_file_close()) {
        die("Could not close log file");
    }
}
